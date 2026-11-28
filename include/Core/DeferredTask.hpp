#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>

#include <Core/ThreadPool.hpp>

namespace Droplet
{
    /// @brief Runs p_work(payload) once on the global ThreadPool and lets the caller wait for it.
    /// If the pool has not started the work when Wait() is called, Wait() runs it on the calling thread,
    /// so Wait() never blocks on work that will not run.
    /// @tparam Payload Data owned by the task. Read it with Get() only after Wait() has returned.
    template <typename Payload>
    class DeferredTask
    {
    public:
        using Work = void (*)(Payload &);

        /// @brief Queues p_work for p_payload on the thread pool.
        /// @return The task. The pool keeps it alive until the work has run, so the caller may drop it.
        [[nodiscard]] static std::shared_ptr<DeferredTask> Launch(Payload p_payload, Work p_work)
        {
            std::shared_ptr<DeferredTask> task(new DeferredTask(std::move(p_payload), p_work));
            ThreadPool::GetInstance().PushTask([task]()
            {
                task->Execute();
            });
            return task;
        }

        /// @brief Blocks until the work has finished, running it on this thread if it has not started.
        void Wait()
        {
            if (Claim())
            {
                Run();
                return;
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this]()
            {
                return m_state == State::Done;
            });
        }

        /// @return The payload. Only read it after Wait() has returned.
        [[nodiscard]] Payload &Get()
        {
            return m_payload;
        }

    private:
        enum class State
        {
            Queued,
            Running,
            Done
        };

        DeferredTask(Payload p_payload, Work p_work)
            : m_payload(std::move(p_payload))
            , m_work(p_work)
        {
        }

        void Execute()
        {
            if (Claim())
            {
                Run();
            }
        }

        /// @return true if this caller now owns running the work.
        bool Claim()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_state != State::Queued)
            {
                return false;
            }
            m_state = State::Running;
            return true;
        }

        void Run()
        {
            m_work(m_payload);

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_state = State::Done;
            }
            m_condition.notify_all();
        }

        Payload m_payload;
        Work m_work;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        State m_state = State::Queued;
    };
}
