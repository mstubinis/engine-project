#pragma once
#ifndef ENGINE_SHADERPROGRAM_H
#define ENGINE_SHADERPROGRAM_H

#include "BindableResource.h"
#include "Engine_EventObject.h"
#include <unordered_map>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

struct Handle;
class Shader;
class ShaderP;
class UniformBufferObject;
typedef unsigned int uint;

class ShaderRenderPass{public: enum Pass{
    None,
    Geometry,
    Forward,
};};
class ShaderType{public:enum Type{
    Vertex,
    Fragment,
    Geometry,
};};

//Core since version 3.1 (GLSL 140)
class UniformBufferObject final: public EventObserver{
    friend class ::Shader;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        static UniformBufferObject* UBO_CAMERA;

        static GLint MAX_UBO_BINDINGS;
        static uint CUSTOM_UBO_AUTOMATIC_COUNT;

        UniformBufferObject(const char* nameInShader,uint sizeofStruct,int globalBindingPointIndex = -1);
        ~UniformBufferObject();

        void onEvent(const Event& e);
        GLuint address();

        void attachToShader(ShaderP* shaderProgram);
        void updateData(void* data);
};

class Shader final: public EngineResource{
    friend class ::ShaderP;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Shader(std::string shaderFileOrData, ShaderType::Type shaderType, bool fromFile = true);
        virtual ~Shader();

        ShaderType::Type type();
        std::string data();
        bool fromFile();
};
class InternalShaderProgramPublicInterface final{
    public:
        static void LoadCPU(ShaderP*);
        static void LoadGPU(ShaderP*);
        static void UnloadCPU(ShaderP*);
        static void UnloadGPU(ShaderP*);
};
class ShaderP final: public BindableResource, public EventObserver{
    friend class ::UniformBufferObject;
    friend class ::Shader;
    friend class ::InternalShaderProgramPublicInterface;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        ShaderP(std::string name, Shader* vertexShader, Shader* fragmentShader);
        virtual ~ShaderP();

        void onEvent(const Event& e);

        void load();
        void unload();

        void bind();
        void unbind();

        GLuint program();
        const std::unordered_map<std::string,GLint>& uniforms() const;
};
#endif