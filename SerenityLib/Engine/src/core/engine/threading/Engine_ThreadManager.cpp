#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

epriv::ThreadManager* threadManager;

epriv::ThreadManager::ThreadManager(){ 
    m_ThreadPool = NEW ThreadPool();
    threadManager = this;
}
epriv::ThreadManager::~ThreadManager(){ 
    cleanup();
}
void epriv::ThreadManager::cleanup() {
    SAFE_DELETE(m_ThreadPool);
}
void epriv::ThreadManager::_update(const double& dt){ 
    m_ThreadPool->update();
}
const size_t epriv::ThreadManager::cores() const{  
    return threadManager->m_ThreadPool->numThreads();
}
void epriv::threading::finalizeJob(std::function<void()>& task){
    threadManager->m_ThreadPool->addJob(std::move(task));  
}
void epriv::threading::finalizeJob(std::function<void()>& task, std::function<void()>& then_task){
    threadManager->m_ThreadPool->addJob(std::move(task), std::move(then_task));
}
void epriv::threading::waitForAll(){ 
    threadManager->m_ThreadPool->wait_for_all();
}