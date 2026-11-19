#include "Integration/RenderGraph/RenderGraph.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <stdexcept>
#include <functional>
#include <map>
#include <queue>

namespace RenderGraph
{
    namespace
    {
        constexpr std::uint32_t NONE = 0xFFFFFFFFu;

        struct Use
        {
            std::uint32_t resource;
            Usage usage;
            bool history;
        };

        bool AllowedOn(Queue queue, Usage usage)
        {
            switch (queue)
            {
            case Queue::Graphics: return true;
            case Queue::Compute:
                return usage == Usage::SampledRead || usage == Usage::StorageWrite || usage == Usage::StorageRead
                    || usage == Usage::UniformRead || usage == Usage::IndirectRead
                    || usage == Usage::TransferSrc || usage == Usage::TransferDst;
            case Queue::Transfer:
                return usage == Usage::TransferSrc || usage == Usage::TransferDst;
            case Queue::Count:
                break;
            }
            return false;
        }
    }

    Compiled Compile(const GraphData& graph)
    {
        Compiled out;
        const std::size_t passCount = graph.passes.size();
        const std::size_t resourceCount = graph.resources.size();

        out.finalUsage.resize(resourceCount);
        out.lastUse.resize(resourceCount);
        out.lifetimes.resize(resourceCount);
        out.usageMasks.resize(resourceCount, 0u);
        out.queueMasks.resize(resourceCount, 0u);

        for (std::size_t i = 0; i < resourceCount; ++i)
            out.finalUsage[i] = graph.resources[i].initialState.usage;

        // 0. validate, and merge duplicate accesses within a pass
        std::vector<std::vector<Use>> uses(passCount);
        for (PassId passId = 0; passId < passCount; ++passId)
        {
            const PassInfo& pass = graph.passes[passId];
            for (const auto& setupError : pass.setupErrors)
                out.errors.push_back("pass '" + pass.name + "': " + setupError);
            for (const Access& access : pass.accesses)
            {
                if (!access.resource.IsValid() || access.resource.index >= resourceCount)
                {
                    out.errors.push_back("pass '" + pass.name + "': invalid resource id");
                    continue;
                }
                const ResourceInfo& accessed = graph.resources[access.resource.index];
                if (!UsageFits(accessed.kind, access.usage))
                    out.errors.push_back(
                        "pass '" + pass.name + "' uses " + ToString(accessed.kind) + " '" + accessed.name + "' as " +
                        ToString(access.usage) + ", which is not a usage of a " + ToString(accessed.kind));
                if (!AllowedOn(pass.queue, access.usage))
                    out.errors.push_back(
                        "pass '" + pass.name + "' on the " + ToString(pass.queue) + " queue cannot use '" +
                        graph.resources[access.resource.index].name + "' as " + ToString(access.usage));

                if (access.history && graph.resources[access.resource.index].IsTransient())
                {
                    out.errors.push_back("pass '"
                        + pass.name + "' reads the previous frame of transient '"
                        + graph.resources[access.resource.index].name
                        + "'. Transients do not survive between frames, import a history resource");
                }

                const auto existingUse = std::ranges::find_if(uses[passId], [&](const Use& use)
                {
                    return use.resource == access.resource.index;
                });
                if (existingUse == uses[passId].end())
                    uses[passId].push_back({access.resource.index, access.usage, access.history});
                else if (existingUse->usage != access.usage || existingUse->history != access.history)
                    out.errors.push_back(
                        "pass '" + pass.name + "' uses '" + graph.resources[access.resource.index].name +
                        "' as both " + ToString(existingUse->usage) + (existingUse->history ? " (previous)" : "") + " and " +
                        ToString(access.usage) + (access.history ? " (previous)" : "") +
                        "; that is a feedback loop, use a separate history resource");
            }
        }
        for (std::uint32_t resourceIndex = 0; resourceIndex < resourceCount; ++resourceIndex)
        {
            const ResourceInfo& info = graph.resources[resourceIndex];
            if (info.transientBuffer && info.transientBuffer->size == 0)
                out.errors.push_back("transient buffer '" + info.name + "': size 0");
            if (info.IsBuffer() && info.options.requiredFinal == Usage::Present)
                out.errors.push_back("buffer '" + info.name + "' cannot be presented");
            if (!info.transient)
                continue;

            const TransientDesc& desc = *info.transient;
            const std::string prefix = "transient '" + info.name + "': ";
            if (desc.format == 0)
                out.errors.push_back(prefix + "no format");
            if (!(desc.width > 0.0f) || !(desc.height > 0.0f))
                out.errors.push_back(prefix + "invalid width or height");
            if (desc.mipLevels == 0 || desc.layers == 0 || desc.samples == 0)
                out.errors.push_back(prefix + "mip levels, array layers or samples cannot be 0.");

            if (desc.sizeClass != SizeClass::InputRelative)
                continue;
            if (!desc.relativeTo.IsValid() || desc.relativeTo.index >= resourceCount)
            {
                out.errors.push_back(prefix + "InputRelative needs a valid relativeTo resource");
                continue;
            }

            std::uint32_t chainIndex = desc.relativeTo.index;
            for (std::size_t chainLength = 0; chainLength <= resourceCount; ++chainLength)
            {
                if (chainIndex == resourceIndex)
                {
                    out.errors.push_back(prefix + "its size depends on itself (InputRelative cycle)");
                    break;
                }
                const ResourceInfo& chainInfo = graph.resources[chainIndex];
                if (chainInfo.IsBuffer())
                {
                    out.errors.push_back(prefix + "InputRelative to buffer '" + chainInfo.name + "', which has no size in pixels");
                    break;
                }

                if (!chainInfo.transient || chainInfo.transient->sizeClass != SizeClass::InputRelative)
                    break; // chain ends in a resource with its own size
                const ResourceId next = chainInfo.transient->relativeTo;
                if (!next.IsValid() || next.index >= resourceCount)
                    break; // chain ends in error reported for that resource
                chainIndex = next.index;
            }
        }

        if (!out.errors.empty())
            return out;

        // 1. per-resource writers (ascending pass index) and readers
        std::vector<std::vector<PassId>> writers(resourceCount), readers(resourceCount), historyReaders(resourceCount);
        for (PassId pass = 0; pass < passCount; ++pass)
            for (const Use& use : uses[pass])
            {
                if (IsWrite(use.usage)) writers[use.resource].push_back(pass);
                else if (use.history) historyReaders[use.resource].push_back(pass);
                else readers[use.resource].push_back(pass);
            }
        // 1b transient has no contents until a pass of this frame writes it
        for (std::uint32_t resourceIndex = 0; resourceIndex < resourceCount; ++resourceIndex)
        {
            if (graph.resources[resourceIndex].IsTransient() && writers[resourceIndex].empty() && !readers[resourceIndex].empty())
                out.errors.push_back("transient '" + graph.resources[resourceIndex].name + "' is read by pass '" +
                    graph.passes[readers[resourceIndex].front()].name + "' but no pass writes it");
        }
        
        if (!out.errors.empty())
            return out;

        // 2. edges. data = value flows along it (propagates liveness); !data = ordering only (WAR, history)
        std::map<std::pair<PassId, PassId>, bool> edgeMap;
        const auto addEdge = [&](PassId from, PassId to, bool data)
        {
            if (from == to) return;
            bool& edgeCarriesData = edgeMap[{from, to}];
            edgeCarriesData = edgeCarriesData || data;
        };
        for (std::size_t resourceIndex = 0; resourceIndex < resourceCount; ++resourceIndex)
        {
            const auto& resourceWriters = writers[resourceIndex];
            for (std::size_t i = 1; i < resourceWriters.size(); ++i)
                addEdge(resourceWriters[i - 1], resourceWriters[i], true); // WAW
            for (const PassId pass : readers[resourceIndex])
            {
                if (resourceWriters.empty()) continue; // reads whatever the resource held on entry
                const std::size_t nextWriter = std::ranges::lower_bound(resourceWriters, pass) - resourceWriters.begin(); // first writer declared after pass
                if (nextWriter > 0)
                {
                    addEdge(resourceWriters[nextWriter - 1], pass, true); // RAW on the version declared before pass
                    if (nextWriter < resourceWriters.size()) addEdge(pass, resourceWriters[nextWriter], false); // WAR: finish before it is overwritten
                }
                else
                    addEdge(resourceWriters.back(), pass, true); // no earlier writer: wants this frame's result
            }
            for (const PassId pass : historyReaders[resourceIndex])
                if (!resourceWriters.empty()) addEdge(pass, resourceWriters.front(), false);
            // read last frame's contents before they are replaced
        }

        // 3. liveness: roots are side-effect passes and passes writing a resource with a required final state
        std::vector<std::vector<PassId>> dataPredecessors(passCount);
        for (const auto& [key, data] : edgeMap)
            if (data) dataPredecessors[key.second].push_back(key.first);

        std::vector<char> live(passCount, 0);
        std::vector<PassId> stack;
        const auto markLive = [&](PassId pass)
        {
            if (!live[pass])
            {
                live[pass] = 1;
                stack.push_back(pass);
            }
        };
        for (PassId pass = 0; pass < passCount; ++pass)
        {
            bool root = graph.passes[pass].sideEffect;
            for (const Use& use : uses[pass])
                if (IsWrite(use.usage) && graph.resources[use.resource].options.requiredFinal != Usage::None) root = true;
            if (root) markLive(pass);
        }
        while (!stack.empty())
        {
            const PassId pass = stack.back();
            stack.pop_back();
            for (const PassId predecessor : dataPredecessors[pass]) markLive(predecessor);
            for (const Use& use : uses[pass])
                if (use.history) // needs last frame's contents to be kept up to date
                    for (const PassId writer : writers[use.resource]) markLive(writer);
        }
        std::size_t liveCount = 0;
        for (PassId pass = 0; pass < passCount; ++pass)
        {
            if (live[pass]) ++liveCount;
            else out.culled.push_back(pass);
        }

        // 4. topological sort of live passes: finds cycles, each pass's level, and an order to walk backwards in 4c
        std::vector<std::vector<PassId>> successors(passCount), livePredecessors(passCount);
        std::vector<std::uint32_t> inDegree(passCount, 0), level(passCount, 0);
        for (const auto& [key, data] : edgeMap)
            if (live[key.first] && live[key.second])
            {
                successors[key.first].push_back(key.second);
                livePredecessors[key.second].push_back(key.first);
                ++inDegree[key.second];
            }
        std::priority_queue<PassId, std::vector<PassId>, std::greater<>> ready;
        for (PassId pass = 0; pass < passCount; ++pass)
            if (live[pass] && inDegree[pass] == 0) ready.push(pass);

        std::vector<PassId> topologicalOrder;
        while (!ready.empty())
        {
            const PassId pass = ready.top();
            ready.pop();
            topologicalOrder.push_back(pass);
            for (const PassId successor : successors[pass])
            {
                level[successor] = std::max(level[successor], level[pass] + 1);
                if (--inDegree[successor] == 0) ready.push(successor);
            }
        }
        if (topologicalOrder.size() != liveCount)
        {
            std::string message = "dependency cycle between passes:";
            for (PassId pass = 0; pass < passCount; ++pass)
                if (live[pass] && inDegree[pass] > 0) message += " '" + graph.passes[pass].name + "'";
            message += ". Break it with ReadPrevious() on one of the reads (needs a history copy).";
            out.errors.push_back(std::move(message));
            return out;
        }

        // 4b. an import touched from more than one queue (including the one it was left on) must be concurrent.
        //     A transient is created by the backend, which reads queueMasks to decide how to share it.
        {
            for (const PassId pass : topologicalOrder)
                for (const Use& use : uses[pass])
                    out.queueMasks[use.resource] |= 1u << ToIndex(graph.passes[pass].queue);
            for (std::size_t resourceIndex = 0; resourceIndex < resourceCount; ++resourceIndex)
            {
                const auto& info = graph.resources[resourceIndex];
                if (info.IsTransient())
                    continue;
                std::uint32_t queuesUsed = out.queueMasks[resourceIndex];
                if (queuesUsed != 0 && info.initialState.usage != Usage::None)
                    queuesUsed |= 1u << ToIndex(info.initialState.queue);
                if (std::popcount(queuesUsed) > 1 && !info.options.concurrent)
                    out.errors.push_back("resource '" + info.name +
                        "' is used on several queues but was not imported as concurrent");
            }
            if (!out.errors.empty())
                return out;
        }

        // 4c. schedule: the order the passes are recorded in and where a submission (one command buffer, one
        //     vkQueueSubmit2) ends. Any order that follows the edges is correct, step 5 plans its barriers and waits;
        //     this one is chosen so that the queues can work side by side:
        //      - a submission is a run of passes on one queue that wait for the same work on other queues. A pass
        //        that has to wait for more starts a new submission, so the passes before it are not held back;
        //      - a queue keeps going while it has such passes, which keeps the number of submissions down;
        //      - passes another queue is waiting for go first and end their submission; the other queue's ready
        //        passes are recorded next, so they are submitted early and run while this queue carries on with
        //        passes nobody else needs.
        //     Ties are broken by declaration order.
        std::vector<PassId> order;
        std::vector<std::uint32_t> submissionOfPass(passCount, NONE), submissionOfStep;
        {
            // a pass feeds another queue if a pass there, or a later pass on its own queue that feeds one, needs it
            std::vector<char> feedsOtherQueueDirectly(passCount, 0), feedsOtherQueue(passCount, 0);
            for (auto passIt = topologicalOrder.rbegin(); passIt != topologicalOrder.rend(); ++passIt)
            {
                const PassId pass = *passIt;
                for (const PassId successor : successors[pass])
                {
                    if (graph.passes[successor].queue != graph.passes[pass].queue)
                        feedsOtherQueueDirectly[pass] = 1;
                    else if (feedsOtherQueue[successor])
                        feedsOtherQueue[pass] = 1;
                }
                if (feedsOtherQueueDirectly[pass])
                    feedsOtherQueue[pass] = 1;
            }

            std::vector<std::uint32_t> unscheduledPredecessors(passCount, 0);
            std::vector<PassId> readyPasses; // ascending, so the first match is the first declared
            for (const PassId pass : topologicalOrder)
            {
                unscheduledPredecessors[pass] = static_cast<std::uint32_t>(livePredecessors[pass].size());
                if (livePredecessors[pass].empty())
                    readyPasses.push_back(pass);
            }
            std::ranges::sort(readyPasses);

            // submission that touched each resource last so far, as step 5 will see it
            std::vector<std::uint32_t> lastToucher(resourceCount, NONE);
            // per queue, the latest signal the open submission waits for (a later signal covers the earlier ones)
            std::array<std::uint32_t, QUEUE_COUNT> waitedSignals{};
            // per queue, the latest signal of every other queue some submission on it has waited for; work on a queue
            // runs in submission order, so a later submission there has waited for those too
            std::array<std::array<std::uint32_t, QUEUE_COUNT>, QUEUE_COUNT> queueWaitedSignals{};
            bool openSubmissionFeedsOtherQueue = false;

            // per queue, the latest signal `pass` has to wait for if it is recorded next
            const auto requiredSignals = [&](PassId pass)
            {
                std::array<std::uint32_t, QUEUE_COUNT> required{};
                const Queue queue = graph.passes[pass].queue;
                const auto requireSubmission = [&](std::uint32_t submissionIndex)
                {
                    const Submission& submission = out.submissions[submissionIndex];
                    if (submission.queue != queue)
                        required[ToIndex(submission.queue)] =
                            std::max(required[ToIndex(submission.queue)], submission.signalIndex);
                };
                for (const PassId predecessor : livePredecessors[pass])
                    requireSubmission(submissionOfPass[predecessor]);
                for (const Use& use : uses[pass])
                    if (lastToucher[use.resource] != NONE)
                        requireSubmission(lastToucher[use.resource]);
                return required;
            };
            // true if `pass` fits the open submission: same queue, and nothing more to wait for
            const auto fitsOpenSubmission = [&](PassId pass)
            {
                if (graph.passes[pass].queue != out.submissions.back().queue)
                    return false;
                const std::array<std::uint32_t, QUEUE_COUNT> required = requiredSignals(pass);
                for (std::size_t queueIndex = 0; queueIndex < QUEUE_COUNT; ++queueIndex)
                    if (required[queueIndex] > waitedSignals[queueIndex])
                        return false;
                return true;
            };

            // true if recording `pass` makes its queue wait for something it has not waited for yet
            const auto needsNewWait = [&](PassId pass)
            {
                const std::array<std::uint32_t, QUEUE_COUNT> required = requiredSignals(pass);
                const auto& alreadyWaited = queueWaitedSignals[ToIndex(graph.passes[pass].queue)];
                for (std::size_t queueIndex = 0; queueIndex < QUEUE_COUNT; ++queueIndex)
                    if (required[queueIndex] > alreadyWaited[queueIndex])
                        return true;
                return false;
            };

            while (!readyPasses.empty())
            {
                // 1. a pass that fits the open submission: first one another queue is waiting for, else any
                auto chosenIt = readyPasses.end();
                bool startSubmission = out.submissions.empty();
                if (!startSubmission)
                {
                    auto fittingIt = readyPasses.end();
                    for (auto passIt = readyPasses.begin(); passIt != readyPasses.end(); ++passIt)
                    {
                        if (!fitsOpenSubmission(*passIt))
                            continue;
                        if (feedsOtherQueue[*passIt])
                        {
                            chosenIt = passIt;
                            break;
                        }
                        if (fittingIt == readyPasses.end())
                            fittingIt = passIt;
                    }
                    if (chosenIt == readyPasses.end() && fittingIt != readyPasses.end())
                    {
                        if (openSubmissionFeedsOtherQueue)
                        {
                            // another queue waits for the open submission: close it and record that queue's ready
                            // work first, so it is submitted (and can start) before this queue's other passes
                            const Queue openQueue = out.submissions.back().queue;
                            const auto otherQueueIt = std::ranges::find_if(readyPasses, [&](PassId readyPass)
                            {
                                return graph.passes[readyPass].queue != openQueue;
                            });
                            chosenIt = otherQueueIt != readyPasses.end() ? otherQueueIt : fittingIt;
                            startSubmission = true;
                        }
                        else
                            chosenIt = fittingIt;
                    }
                }
                // 2. otherwise a new submission: the first declared ready pass that needs no new wait, so the waiting
                //    passes do not hold it back; failing that, the first declared ready pass
                if (chosenIt == readyPasses.end())
                {
                    chosenIt = std::ranges::find_if(readyPasses, [&](PassId readyPass) { return !needsNewWait(readyPass); });
                    if (chosenIt == readyPasses.end())
                        chosenIt = readyPasses.begin();
                    startSubmission = true;
                }

                const PassId pass = *chosenIt;
                readyPasses.erase(chosenIt);
                const Queue queue = graph.passes[pass].queue;
                const std::array<std::uint32_t, QUEUE_COUNT> required = requiredSignals(pass);

                if (startSubmission)
                {
                    Submission submission;
                    submission.queue = queue;
                    submission.firstStep = static_cast<std::uint32_t>(order.size());
                    submission.signalIndex = ++out.signalsPerQueue[ToIndex(queue)];
                    out.submissions.push_back(std::move(submission));
                    waitedSignals.fill(0);
                    openSubmissionFeedsOtherQueue = false;
                }
                const std::uint32_t submissionIndex = static_cast<std::uint32_t>(out.submissions.size() - 1);
                ++out.submissions.back().stepCount;
                auto& queueWaited = queueWaitedSignals[ToIndex(queue)];
                for (std::size_t queueIndex = 0; queueIndex < QUEUE_COUNT; ++queueIndex)
                {
                    waitedSignals[queueIndex] = std::max(waitedSignals[queueIndex], required[queueIndex]);
                    queueWaited[queueIndex] = std::max(queueWaited[queueIndex], required[queueIndex]);
                }
                openSubmissionFeedsOtherQueue = openSubmissionFeedsOtherQueue || feedsOtherQueueDirectly[pass];

                order.push_back(pass);
                submissionOfStep.push_back(submissionIndex);
                submissionOfPass[pass] = submissionIndex;
                for (const Use& use : uses[pass])
                    lastToucher[use.resource] = submissionIndex;

                Step step;
                step.pass = pass;
                step.queue = queue;
                step.level = level[pass];
                out.steps.push_back(std::move(step));

                for (const PassId successor : successors[pass])
                    if (--unscheduledPredecessors[successor] == 0)
                        readyPasses.insert(std::ranges::lower_bound(readyPasses, successor), successor);
            }
        }

        std::vector<std::vector<std::uint32_t>> submissionWaits(out.submissions.size());
        std::vector<std::vector<CarryWait>> submissionCarryWaits(out.submissions.size());

        // dependencies that cross queues need a timeline wait (same-queue order is implicit)
        for (std::uint32_t stepIndex = 0; stepIndex < order.size(); ++stepIndex)
        {
            const std::uint32_t submissionIndex = submissionOfStep[stepIndex];
            for (const PassId predecessor : livePredecessors[order[stepIndex]])
            {
                const std::uint32_t predecessorSubmission = submissionOfPass[predecessor];
                if (predecessorSubmission != submissionIndex && out.submissions[predecessorSubmission].queue != out.submissions[submissionIndex].queue)
                    submissionWaits[submissionIndex].push_back(predecessorSubmission);
            }
        }

        // 5. barriers: walk in order, tracking each resource's usage and who touched it last
        std::vector<Usage> currentUsage = out.finalUsage;
        std::vector<std::uint32_t> firstSubmission(resourceCount, NONE), lastSubmission(resourceCount, NONE);
        for (std::uint32_t stepIndex = 0; stepIndex < order.size(); ++stepIndex)
        {
            const PassId pass = order[stepIndex];
            const std::uint32_t submissionIndex = submissionOfStep[stepIndex];
            const Queue queue = out.submissions[submissionIndex].queue;
            Step& step = out.steps[stepIndex];

            for (const Use& use : uses[pass])
            {
                Lifetime &lifetime = out.lifetimes[use.resource];
                if (!lifetime.IsUsed())
                    lifetime.firstStep = stepIndex;
                lifetime.lastStep = stepIndex;
                out.usageMasks[use.resource] |= UsageBit(use.usage);
                
                const auto& info = graph.resources[use.resource];
                const Usage previousUsage = currentUsage[use.resource];
                if (previousUsage == Usage::None && !IsWrite(use.usage) && !use.history)
                    out.warnings.push_back("pass '" + graph.passes[pass].name + "' reads '" + info.name +
                        "' before anything has written it (contents undefined)");

                // Whoever touched the resource last on another queue has to be finished first, whether or not this
                // use needs a barrier (a second reader on another queue must still wait for the first reader's
                // layout transition): a submission of this frame, or for an import the previous frame's (carry).
                Queue previousQueue = queue;
                const std::uint32_t last = lastSubmission[use.resource];
                if (last != NONE)
                {
                    previousQueue = out.submissions[last].queue;
                    if (previousQueue != queue)
                        submissionWaits[submissionIndex].push_back(last);
                }
                else if (info.initialState.usage != Usage::None)
                {
                    previousQueue = info.initialState.queue;
                    if (previousQueue != queue && info.initialState.value != 0)
                        submissionCarryWaits[submissionIndex].push_back({previousQueue, info.initialState.value});
                }

                if (previousUsage != use.usage || IsWrite(use.usage))
                {
                    step.barriers.push_back({{use.resource}, previousUsage, use.usage, previousQueue});
                    currentUsage[use.resource] = use.usage;
                }
                if (firstSubmission[use.resource] == NONE) firstSubmission[use.resource] = submissionIndex;
                lastSubmission[use.resource] = submissionIndex;
            }
        }

        // 6. finalize waits (keep only the latest wait per source queue), external sync, epilogue, last use
        for (std::uint32_t submissionIndex = 0; submissionIndex < out.submissions.size(); ++submissionIndex)
        {
            Submission& submission = out.submissions[submissionIndex];

            std::array<std::uint32_t, QUEUE_COUNT> latestWaitPerQueue;
            latestWaitPerQueue.fill(NONE);
            for (const std::uint32_t waitedSubmission : submissionWaits[submissionIndex])
            {
                const std::size_t queueIndex = ToIndex(out.submissions[waitedSubmission].queue);
                if (latestWaitPerQueue[queueIndex] == NONE || out.submissions[waitedSubmission].signalIndex > out.submissions[latestWaitPerQueue[queueIndex]].signalIndex)
                    latestWaitPerQueue[queueIndex] = waitedSubmission;
            }
            for (const std::uint32_t waitedSubmission : latestWaitPerQueue)
                if (waitedSubmission != NONE) submission.waits.push_back(waitedSubmission);

            std::array<std::uint64_t, QUEUE_COUNT> carryPerQueue{};
            for (const CarryWait& carryWait : submissionCarryWaits[submissionIndex])
                carryPerQueue[ToIndex(carryWait.queue)] = std::max(carryPerQueue[ToIndex(carryWait.queue)], carryWait.value);
            for (std::size_t queueIndex = 0; queueIndex < QUEUE_COUNT; ++queueIndex)
                if (carryPerQueue[queueIndex] != 0) submission.carryWaits.push_back({static_cast<Queue>(queueIndex), carryPerQueue[queueIndex]});
        }
        for (std::size_t resourceIndex = 0; resourceIndex < resourceCount; ++resourceIndex)
        {
            if (firstSubmission[resourceIndex] == NONE)
                continue;
            const auto& options = graph.resources[resourceIndex].options;
            const ResourceId resourceId{static_cast<std::uint32_t>(resourceIndex)};
            Submission& last = out.submissions[lastSubmission[resourceIndex]];

            if (options.waitExternal) out.submissions[firstSubmission[resourceIndex]].waitExternal.push_back(resourceId);
            if (options.signalExternal) last.signalExternal.push_back(resourceId);
            if (options.requiredFinal != Usage::None && currentUsage[resourceIndex] != options.requiredFinal)
            {
                last.epilogue.push_back({resourceId, currentUsage[resourceIndex], options.requiredFinal, last.queue});
                out.usageMasks[resourceIndex] |= UsageBit(options.requiredFinal);
                currentUsage[resourceIndex] = options.requiredFinal;
            }
            out.lastUse[resourceIndex] = {true, last.queue, lastSubmission[resourceIndex]};
        }
        out.finalUsage = std::move(currentUsage);
        out.ok = true;
        return out;
    }

    namespace
    {
        std::uint32_t ScaleDimension(std::uint32_t base, float factor)
        {
            const double scaled = std::ceil(static_cast<double>(base) * static_cast<double>(factor));
            return std::max(1u, static_cast<std::uint32_t>(scaled));
        }

        Extent2D ResolveExtentAtDepth(const GraphData& graph, ResourceId resource, Extent2D swapchainExtent,
                                      const std::function<Extent2D(ResourceId)>& importedExtent, std::size_t depth)
        {
            if (depth > graph.resources.size())
                throw std::logic_error("ResolveExtent: InputRelative references form a cycle");

            const ResourceInfo& info = graph.resources.at(resource.index);
            if (info.IsBuffer())
                return {};
            if (!info.transient)
                return importedExtent ? importedExtent(resource) : Extent2D{};

            const TransientDesc& desc = *info.transient;
            switch (desc.sizeClass)
            {
            case SizeClass::Absolute:
                return {
                    std::max(1u, static_cast<std::uint32_t>(std::lround(desc.width))),
                    std::max(1u, static_cast<std::uint32_t>(std::lround(desc.height)))
                };
            case SizeClass::SwapchainRelative:
                return {
                    ScaleDimension(swapchainExtent.width, desc.width),
                    ScaleDimension(swapchainExtent.height, desc.height)
                };
            case SizeClass::InputRelative:
                {
                    const Extent2D base =
                        ResolveExtentAtDepth(graph, desc.relativeTo, swapchainExtent, importedExtent, depth + 1);
                    return {ScaleDimension(base.width, desc.width), ScaleDimension(base.height, desc.height)};
                }
            }
            return {};
        }
    }


    Extent2D ResolveExtent(const GraphData& graph, 
        ResourceId id, 
        Extent2D swapChainExtent,
        const std::function<Extent2D(ResourceId)>& importedExtent)
    {
        return ResolveExtentAtDepth(graph, id, swapChainExtent, importedExtent, 0);
    }

    std::string Dump(const GraphData& graph, const Compiled& compiled)
    {
        std::string text;
        const auto describeBarrier = [&](const Barrier& barrier)
        {
            return graph.resources[barrier.resourceId.index].name + ": " + ToString(barrier.from) + " -> " +
                ToString(barrier.to);
        };
        for (const auto& error : compiled.errors) text += "ERROR: " + error + "\n";
        for (const auto& warning : compiled.warnings) text += "warning: " + warning + "\n";
        for (std::size_t submissionIndex = 0; submissionIndex < compiled.submissions.size(); ++submissionIndex)
        {
            const Submission& submission = compiled.submissions[submissionIndex];
            text += "submission " + std::to_string(submissionIndex) + " [" + ToString(submission.queue) + " signal #" +
                std::to_string(
                    submission.signalIndex) + "]";
            for (const auto wait : submission.waits)
            {
                text += " waits " + std::string(ToString(compiled.submissions[wait].queue)) + "#" + std::to_string(
                    compiled.submissions[wait].signalIndex);
            }
            for (const auto& carryWait : submission.carryWaits)
            {
                text += " waits previous-frame " + std::string(ToString(carryWait.queue)) + "@" +
                    std::to_string(carryWait.value);
            }
            for (const auto resourceId : submission.waitExternal)
            {
                text += " waits external(" + graph.resources[resourceId.index].name + ")";
            }
            for (const auto resourceId : submission.signalExternal)
            {
                text += " signals external(" + graph.resources[resourceId.index].name + ")";
            }
            text += "\n";
            for (std::uint32_t i = 0; i < submission.stepCount; ++i)
            {
                const Step& step = compiled.steps[submission.firstStep + i];
                text += "  [L" + std::to_string(step.level) + "] " + graph.passes[step.pass].name + "\n";
                for (const auto& barrier : step.barriers)
                {
                    text += "      barrier " + describeBarrier(barrier) + (barrier.fromQueue != step.queue
                                                                            ? std::string(" (from ") + ToString(
                                                                                barrier.fromQueue) +
                                                                            ")"
                                                                            : "") + "\n";
                }
            }
            for (const auto& barrier : submission.epilogue) { text += "  [end] barrier " + describeBarrier(barrier) + "\n"; }
        }
        for (const PassId pass : compiled.culled)
        {
            text += "  culled: " + graph.passes[pass].name + "\n";
        }

        for (std::size_t resourceIndex = 0; resourceIndex < graph.resources.size() && resourceIndex < compiled.lifetimes.size();
             ++resourceIndex)
        {
            const ResourceInfo& info = graph.resources[resourceIndex];
            if (!info.IsTransient())
                continue;
            const Lifetime& lifetime = compiled.lifetimes[resourceIndex];
            text += std::string("  transient ") + (info.IsBuffer() ? "buffer " : "") + info.name + ": " +
                 (lifetime.IsUsed()
                      ? "steps " + std::to_string(lifetime.firstStep) + ".." + std::to_string(lifetime.lastStep)
                      : std::string("unused (every user was culled)")) +
                 "\n";
        }
        
        return text;
    }
}
