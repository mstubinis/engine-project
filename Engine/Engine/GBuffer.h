#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

#include "Texture.h"
#include <unordered_map>

class GBufferType{public: enum Type{
    Diffuse,
    Normal,
    Misc,
    Lighting,
    Bloom,
    GodRays,
    Free2,
    Depth,

    EnumTotal
};};

class TextureBuffer final: public Texture{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        TextureBuffer(std::string name,GLuint,GLuint,GLuint,GLuint,uint,uint,float divisor);
        ~TextureBuffer();

        const float divisor() const;
        const int attatchment() const;
};
class GBuffer final{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        GBuffer(uint w,uint h);
        ~GBuffer();

        void resize(uint w,uint h);

        void start(std::vector<uint>&,std::string = "RGBA",bool = true);
        void start(uint,std::string = "RGBA",bool = true);
        void start(uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,uint,uint,uint,std::string = "RGBA",bool = true);
        void stop();

        const std::unordered_map<uint,boost::weak_ptr<TextureBuffer>>& getBuffers() const;
        Texture* getTexture(uint);

        const GLuint& getMainFBO() const;
        const GLuint& getSmallFBO() const;
};
#endif
