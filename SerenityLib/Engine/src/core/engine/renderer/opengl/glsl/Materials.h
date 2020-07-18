#pragma once
#ifndef ENGINE_RENDERER_OPENGL_GLSL_MATERIALS_H
#define ENGINE_RENDERER_OPENGL_GLSL_MATERIALS_H

#include <core/engine/shaders/ShaderIncludes.h>

namespace Engine::priv::opengl::glsl {
    class Materials final {
        public:
            static void convert(std::string& code, const unsigned int versionNumber, const ShaderType::Type shaderType);
    };
};

#endif