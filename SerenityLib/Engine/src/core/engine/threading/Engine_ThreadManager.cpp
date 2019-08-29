#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/threading/ThreadPool.h>

using namespace Engine;
using namespace std;

epriv::ThreadManager* threadManager;

epriv::ThreadManager::ThreadManager(const char* name, uint w, uint h){ 
    m_ThreadPool = new ThreadPool();
    threadManager = this;
}
epriv::ThreadManager::~ThreadManager(){ 
    SAFE_DELETE(m_ThreadPool);
}
void epriv::ThreadManager::_update(const double& dt){ 
    m_ThreadPool->update();
}
const uint epriv::ThreadManager::cores() const{  
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