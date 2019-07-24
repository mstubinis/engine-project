#pragma once
#ifndef ENGINE_RENDERER_OPENGL_UNIFORM_BUFFER_OBJECT_H
#define ENGINE_RENDERER_OPENGL_UNIFORM_BUFFER_OBJECT_H

#include <core/engine/events/Engine_EventObject.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

//Core since version 3.1 (GLSL 140)
class  Shader;
class  ShaderP;
class UniformBufferObject final : public EventObserver {
    friend class Shader;
    private:
        const char*  m_NameInShader;
        uint         m_SizeOfStruct;
        int          m_GlobalBindingPointNumber;
        GLuint       m_UBOObject;

        void _load_CPU();
        void _unload_CPU();
        void _load_GPU();
        void _unload_GPU();
    public:
        static UniformBufferObject* UBO_CAMERA;

        static GLint MAX_UBO_BINDINGS;
        static uint  CUSTOM_UBO_AUTOMATIC_COUNT;

        UniformBufferObject(const char* nameInShader, const uint& sizeofStruct, const int& globalBindingPointIndex = -1);
        ~UniformBufferObject();

        void onEvent(const Event& e);
        const GLuint& address() const;

        void attachToShader(const ShaderP& shaderProgram);
        void updateData(void* data);
};

#endif