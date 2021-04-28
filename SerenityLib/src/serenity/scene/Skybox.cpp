
#include <serenity/scene/Skybox.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/resources/texture/TextureLoaderCubemap.h>
#include <serenity/events/Event.h>

GLuint m_Buffer   = 0;
GLuint m_VAO      = 0;

constexpr std::array<glm::vec3, 36> VERTICES = {
    glm::vec3(-1, 1, 1),
    glm::vec3(1, 1, 1),
    glm::vec3(1, -1, 1),
    glm::vec3(-1, 1, 1),
    glm::vec3(1, -1, 1),
    glm::vec3(-1, -1, 1),
    glm::vec3(-1, 1, -1),
    glm::vec3(-1, -1, -1),
    glm::vec3(1, -1, -1),
    glm::vec3(-1, 1, -1),
    glm::vec3(1, -1, -1),
    glm::vec3(1, 1, -1),
    glm::vec3(-1, 1, 1),
    glm::vec3(-1, -1, 1),
    glm::vec3(-1, -1, -1),
    glm::vec3(-1, 1, 1),
    glm::vec3(-1, -1, -1),
    glm::vec3(-1, 1, -1),
    glm::vec3(-1, -1, 1),
    glm::vec3(1, -1, 1),
    glm::vec3(1, -1, -1),
    glm::vec3(-1, -1, 1),
    glm::vec3(1, -1, -1),
    glm::vec3(-1, -1, -1),
    glm::vec3(1, -1, 1),
    glm::vec3(1, 1, 1),
    glm::vec3(1, 1, -1),
    glm::vec3(1, -1, 1),
    glm::vec3(1, 1, -1),
    glm::vec3(1, -1, -1),
    glm::vec3(1, 1, 1),
    glm::vec3(-1, 1, 1),
    glm::vec3(-1, 1, -1),
    glm::vec3(1, 1, 1),
    glm::vec3(-1, 1, -1),
    glm::vec3(1, 1, -1),
};

/*
when saving a skybox in gimp, the layer ordering should be:
    - left
    - right
    - top
    - bottom
    - back    - should be front file
    - front   - should be back file

(switch the front and back files)

*/

namespace Engine::priv {
    class SkyboxImplInterface final {
        friend class Skybox;
        static void bindDataToGPU() noexcept {
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffer));
            GLCall(glEnableVertexAttribArray(0));
            GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
        }
        static void buildVAO() noexcept {
            Engine::Renderer::deleteVAO(m_VAO);
            if (Engine::priv::OpenGLState::constants.supportsVAO()) {
                Engine::Renderer::genAndBindVAO(m_VAO);
                Engine::priv::SkyboxImplInterface::bindDataToGPU();
                Engine::Renderer::bindVAO(0);
            }
        }
        static void initMesh() noexcept {
            if (m_Buffer != 0U) {
                return;
            }
            GLCall(glGenBuffers(1, &m_Buffer));
            GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_Buffer));
            GLCall(glBufferData(GL_ARRAY_BUFFER, VERTICES.size() * sizeof(glm::vec3), &VERTICES[0], GL_STATIC_DRAW));
            Engine::priv::SkyboxImplInterface::buildVAO();
        }
    };
};

Skybox::Skybox(const std::array<std::string_view, 6>& files) {
    Engine::priv::SkyboxImplInterface::initMesh();

    //instead of using files[0] generate a proper name using the directory?

    m_TextureCubemap = Engine::Resources::addResource<TextureCubemap>(files, std::string(files[0]) + "Cubemap", false, ImageInternalFormat::SRGB8_ALPHA8);
    Engine::priv::TextureLoaderCubemap::GeneratePBRData(*m_TextureCubemap.get<TextureCubemap>(), 32, m_TextureCubemap.get<TextureCubemap>()->width() / 4);

    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::Skybox(std::string_view filename) {
    Engine::priv::SkyboxImplInterface::initMesh();

    m_TextureCubemap = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get<TextureCubemap>(filename).m_Handle;
    if (!m_TextureCubemap) {
        m_TextureCubemap = Engine::Resources::addResource<TextureCubemap>(filename, false, ImageInternalFormat::SRGB8_ALPHA8);
        Engine::priv::TextureLoaderCubemap::GeneratePBRData(*m_TextureCubemap.get<TextureCubemap>(), 32, m_TextureCubemap.get<TextureCubemap>()->width() / 4);
    }
    registerEvent(EventType::WindowFullscreenChanged);
}
Skybox::~Skybox() {
    unregisterEvent(EventType::WindowFullscreenChanged);
}

void Skybox::bindMesh() {
    Engine::priv::SkyboxImplInterface::initMesh();
    if(m_VAO){
        Engine::Renderer::bindVAO(m_VAO);
        GLCall(glDrawArrays( GL_TRIANGLES, 0, (GLsizei)VERTICES.size() ));
    }else{
        Engine::priv::SkyboxImplInterface::bindDataToGPU();
        GLCall(glDrawArrays( GL_TRIANGLES, 0, (GLsizei)VERTICES.size() ));
        GLCall(glDisableVertexAttribArray(0));
    }
}
void Skybox::onEvent(const Event& e) {
    if (e.type == EventType::WindowFullscreenChanged) {
        Engine::priv::SkyboxImplInterface::buildVAO();
    }
}