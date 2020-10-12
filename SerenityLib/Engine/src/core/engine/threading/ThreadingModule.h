#pragma once
#ifndef ENGINE_THREADING_MODULE_H
#define ENGINE_THREADING_MODULE_H

namespace Engine::priv {
    class  ThreadPool;
};

#include <core/engine/threading/ThreadingHelpers.h>
#include <core/engine/threading/ThreadPool.h>

namespace Engine::priv {
    class ThreadingModule final {
        public:
            static Engine::view_ptr<Engine::priv::ThreadingModule> THREADING_MODULE;
        public:
            ThreadPool m_ThreadPool;

            ThreadingModule();
            ~ThreadingModule() { cleanup(); }

            inline void cleanup() { m_ThreadPool.shutdown(); }
            inline void update(const float dt) { m_ThreadPool.update(); }
    };
    namespace threading {
        bool isOpenGLThread() noexcept;
        bool isMainThread() noexcept;

        void submitTaskForMainThread(std::function<void()>&& task) noexcept;

        inline void waitForAll(size_t section = 0) noexcept {
            ThreadingModule::THREADING_MODULE->m_ThreadPool.wait_for_all(section);
        }

        template<class TASK> inline void finalizeJob(TASK&& task, size_t section) {
            ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::move(task), section);
        }
        template<class TASK, class THEN> inline void finalizeJob(TASK&& task, THEN&& then, size_t section) {
            ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::move(task), std::move(then), section);
        }
        template<class JOB> inline void addJob(JOB&& job, size_t section = 0U) {
            finalizeJob(std::move(job), section);
        }
        template<class JOB, class THEN> inline void addJobWithPostCallback(JOB&& job, THEN&& then, size_t section = 0U){
            finalizeJob(std::move(job), std::move(then), section);
        }
        template<class JOB, class T> void addJobSplitVectored(JOB&& job, std::vector<T>& collection, bool waitForAll, size_t section = 0U) {
            if (Engine::hardware_concurrency() > 1) {
                auto pairs = Engine::splitVectorPairs(collection);
                for (size_t k = 0U; k < pairs.size(); ++k) {
                    Engine::priv::threading::addJob([&pairs, k, &job, &collection]() {
                        for (size_t j = pairs[k].first; j <= pairs[k].second; ++j) {
                            job(collection[j], j, k);
                        }  
                    }, section);
                }
                if (waitForAll) {
                    Engine::priv::threading::waitForAll(section);
                }
            }else{
                for (size_t j = 0; j < collection.size(); ++j) {
                    job(collection[j], j, 0U);
                }
            }
        }
    };
};

#endif