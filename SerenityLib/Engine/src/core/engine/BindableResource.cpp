#include <core/engine/BindableResource.h>

using namespace std;

namespace Engine{
    namespace priv{
        struct BindableResourceEmptyFunctor{template<class T> void operator()(T* r) const {}};
    };
};

BindableResource::BindableResource(const ResourceType::Type& type, const string& name) : EngineResource(type, name){
    Engine::priv::BindableResourceEmptyFunctor empty;
    setCustomBindFunctor(empty);
    setCustomUnbindFunctor(empty);
}
BindableResource::~BindableResource(){
}
void BindableResource::bind() const { 
    m_CustomBindFunctor(); 
}
void BindableResource::unbind() const { 
    m_CustomUnbindFunctor(); 
}
