#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>

#include <Core/ThreadPool.hpp>

namespace Droplet
{
    /// @brief Runs p_work(i) for every i in [0, p_count), spread across the global ThreadPool and the calling thread.
    /// Blocks until every index has been processed. The calling thread also takes indices, so the call
    /// completes even if the pool is not running or drops its tasks.
    /// @param p_work Callable taking a std::size_t index. Must be safe to call concurrently for different indices.
    template <typename Work>
    void ParallelFor(std::size_t p_count, Work p_work)
    {
        if (p_count == 0)
        {
            return;
        }

        struct State
        {
            std::atomic<std::size_t> next{0};
            std::atomic<std::size_t> done{0};
            std::mutex mutex;
            std::condition_variable condition;
        };
        const std::shared_ptr<State> state = std::make_shared<State>();

        // Pool tasks may run after this call returns. They only touch `state` once no indices are left,
        // and `p_work` is only called for indices claimed before the wait below ends.
        const auto drain = [state, p_count, &p_work]()
        {
            while (true)
            {
                const std::size_t index = state->next.fetch_add(1);
                if (index >= p_count)
                {
                    return;
                }

                p_work(index);

                if (state->done.fetch_add(1) + 1 == p_count)
                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    state->condition.notify_all();
                }
            }
        };

        const std::size_t helperCount = std::min<std::size_t>(p_count, std::max(1u, std::thread::hardware_concurrency()));
        for (std::size_t helper = 0; helper < helperCount; ++helper)
        {
            ThreadPool::GetInstance().PushTask(drain);
        }

        drain();

        std::unique_lock<std::mutex> lock(state->mutex);
        state->condition.wait(lock, [&state, p_count]()
            {
                return state->done.load() == p_count;
            });
    }
}
