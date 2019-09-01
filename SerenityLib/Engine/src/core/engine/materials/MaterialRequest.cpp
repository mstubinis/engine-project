#include <core/engine/materials/MaterialRequest.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/textures/Texture.h>

#include <core/engine/Engine.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::epriv;

MaterialRequest::MaterialRequest() {
    async = false;
}
MaterialRequest::MaterialRequest(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular) : MaterialRequest() {
    auto d = new TextureRequest(diffuse,  true,  ImageInternalFormat::SRGB8_ALPHA8);
    auto n = new TextureRequest(normal,   false, ImageInternalFormat::RGBA8);
    auto g = new TextureRequest(glow,     false, ImageInternalFormat::R8);
    auto s = new TextureRequest(specular, false, ImageInternalFormat::R8);
    
    part.textureRequests.push_back(d);
    part.textureRequests.push_back(n);
    part.textureRequests.push_back(g);
    part.textureRequests.push_back(s);
    part.name = name;
}
MaterialRequest::~MaterialRequest() {

}
void MaterialRequest::request() {
    async = false;
    InternalMaterialRequestPublicInterface::Request(*this);
}
void MaterialRequest::requestAsync() {
    async = true;
    for (auto& textureRequest : part.textureRequests) {
        textureRequest->async = true;
    }
    InternalMaterialRequestPublicInterface::Request(*this);
}

void InternalMaterialRequestPublicInterface::Request(MaterialRequest& request) {
    request.part.material = new Material();
    request.part.material->setName(request.part.name);
    request.part.handle = Core::m_Engine->m_ResourceManager.m_Resources->add(request.part.material, ResourceType::Material);

    if (request.async) {
        for (auto& textureRequest : request.part.textureRequests) {
            textureRequest->selfClean = false;
            textureRequest->requestAsync();
        }
        const auto& reference = std::ref(request);
        const auto& job = std::bind(&InternalMaterialRequestPublicInterface::LoadCPU, reference);
        const auto& cbk = std::bind(&InternalMaterialRequestPublicInterface::LoadGPU, reference);
        threading::addJobWithPostCallback(job, cbk);
    }else{
        for (auto& textureRequest : request.part.textureRequests) {
            textureRequest->selfClean = false;
            textureRequest->request();
        }
        InternalMaterialRequestPublicInterface::LoadCPU(request);
        InternalMaterialRequestPublicInterface::LoadGPU(request);
    }
}
void InternalMaterialRequestPublicInterface::LoadCPU(MaterialRequest& request) {
    InternalMaterialPublicInterface::LoadCPU(*request.part.material);
}
void InternalMaterialRequestPublicInterface::LoadGPU(MaterialRequest& request) {
    const auto& size = request.part.textureRequests.size();
    if (size > 0) {
        request.part.material->addComponent(MaterialComponentType::Diffuse, request.part.textureRequests[0]->file);
    }
    if (size > 1) {
        request.part.material->addComponent(MaterialComponentType::Normal, request.part.textureRequests[1]->file);
    }
    if (size > 2) {
        request.part.material->addComponent(MaterialComponentType::Glow, request.part.textureRequests[2]->file);
    }
    if (size > 3) {
        request.part.material->addComponent(MaterialComponentType::Specular, request.part.textureRequests[3]->file);
    }
    InternalMaterialPublicInterface::LoadGPU(*request.part.material);
    for (auto& textureRequest : request.part.textureRequests) {
        if (textureRequest && !textureRequest->selfClean)
            delete(textureRequest);
    }
    if (request.async)
        delete(&request); //yes its ugly, but its needed. see Resources::loadMaterialAsync()
}