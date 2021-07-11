#pragma once
#ifndef ENGINE_THREADING_WORKER_THREAD_CONTAINER_H
#define ENGINE_THREADING_WORKER_THREAD_CONTAINER_H

#include <unordered_map>
#include <vector>
#include <thread>
#include <serenity/system/Macros.h>
#include <serenity/types/Types.h>

namespace Engine::priv {
    class WorkerThreadContainer final {
        using ThreadVector = std::vector<std::jthread>;
        private:
            ThreadVector                                         m_WorkerThreads;
            std::unordered_map<std::jthread::id, std::jthread*>  m_WorkerThreadsHashed;
        public:
            WorkerThreadContainer() = default;
            ~WorkerThreadContainer();

            void clear() noexcept;
            void reserve(size_t newReserveSize) noexcept;

            [[nodiscard]] inline size_t size() const noexcept { return m_WorkerThreads.size(); }

            template<class FUNC>
            Engine::view_ptr<std::jthread> add_thread(FUNC&& func) noexcept {
                if (m_WorkerThreads.size() >= m_WorkerThreads.capacity()) {
                    ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): m_WorkerThreads reached its capacity!")
                    return nullptr;
                }
                auto& worker = m_WorkerThreads.emplace_back(std::forward<FUNC>(func));
                m_WorkerThreadsHashed.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(worker.get_id()),
                    std::forward_as_tuple(&worker)
                );
                return &worker;
            }

            [[nodiscard]] inline std::jthread* operator[](std::thread::id threadID) noexcept { return m_WorkerThreadsHashed.at(threadID); }

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(ThreadVector, m_WorkerThreads)
    };
};

#endif