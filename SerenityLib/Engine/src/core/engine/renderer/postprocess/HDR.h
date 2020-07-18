#pragma once
#ifndef ENGINE_RENDERER_POSTPROCESS_HDR_H
#define ENGINE_RENDERER_POSTPROCESS_HDR_H

class  ShaderProgram;
class  Shader;
class  Viewport;

struct HDRAlgorithm { enum Algorithm : unsigned char {
    None,
    Reinhard, 
    Filmic, 
    Exposure, 
    Uncharted,
};};

namespace Engine::priv {
    class  GBuffer;
    class  Renderer;
    class  HDR final {
        private:
            Shader*                   m_Vertex_Shader     = nullptr;
            Shader*                   m_Fragment_Shader   = nullptr;
            ShaderProgram*            m_Shader_Program    = nullptr;

            std::string               m_GLSL_frag_code    = "";
        public:
            bool                      hdr_active = true;
            float                     exposure   = 3.0f;
            HDRAlgorithm::Algorithm   algorithm  = HDRAlgorithm::Uncharted;

            HDR() = default;
            ~HDR();

            const bool init_shaders();

            void pass(GBuffer&, const Viewport& viewport, const bool godRays, const bool lighting, const float godRaysFactor, const Engine::priv::Renderer& renderer);

            static HDR hdr;
    };
};
namespace Engine::Renderer::hdr {
    const float getExposure();
    void setExposure(const float e);
    void setAlgorithm(const HDRAlgorithm::Algorithm a);
    const HDRAlgorithm::Algorithm getAlgorithm();
};



#endif