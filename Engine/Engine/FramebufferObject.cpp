#include "FramebufferObject.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Texture.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <unordered_map>

using namespace Engine;
using namespace std;

#pragma region FramebufferObjectAttatchmentBaseClass

class epriv::FramebufferObjectAttatchment::impl{
    public:
        GLuint m_InternalFormat;
        FramebufferObject* m_FBO;
        GLuint m_GL_Attatchment;
        void _init(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i){
            m_FBO = _fbo;
            m_GL_Attatchment = FramebufferAttatchment::at(a);
            m_InternalFormat = ImageInternalFormat::at(i);
        }
        void _init(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t){
            m_FBO = _fbo;
            m_GL_Attatchment = FramebufferAttatchment::at(a);
            m_InternalFormat = ImageInternalFormat::at(t->internalFormat());
        }
        void _destruct(){}
        void _resize(){}
};
epriv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):m_i(new impl){
    m_i->_init(_fbo,a,i);
}
epriv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t):m_i(new impl){
    m_i->_init(_fbo,a,t);
}
epriv::FramebufferObjectAttatchment::~FramebufferObjectAttatchment(){ m_i->_destruct(); }
uint epriv::FramebufferObjectAttatchment::width(){ return m_i->m_FBO->width(); }
uint epriv::FramebufferObjectAttatchment::height(){ return m_i->m_FBO->height(); }
epriv::FramebufferObject* epriv::FramebufferObjectAttatchment::fbo(){ return m_i->m_FBO; }
uint epriv::FramebufferObjectAttatchment::attatchment(){ return m_i->m_GL_Attatchment; }
void epriv::FramebufferObjectAttatchment::resize(FramebufferObject* _fbo,uint w, uint h){ m_i->_resize(); }
GLuint epriv::FramebufferObjectAttatchment::address(){ return GLuint(0); }
void epriv::FramebufferObjectAttatchment::bind(){}
void epriv::FramebufferObjectAttatchment::unbind(){}
GLuint epriv::FramebufferObjectAttatchment::internalFormat(){ return m_i->m_InternalFormat; }

#pragma endregion

class epriv::FramebufferTexture::impl{
    public:
        Texture* m_Texture;
        GLuint m_PixelFormat;
        GLuint m_PixelType;
        void _init(FramebufferTexture* super,Texture* t){
            m_Texture = t;
            m_PixelFormat = ImagePixelFormat::at(t->pixelFormat());
            m_PixelType = ImagePixelType::at(t->pixelType());
            glFramebufferTexture2D(GL_FRAMEBUFFER,super->attatchment(),GL_TEXTURE_2D,m_Texture->address(),0);
            Renderer::bindTexture(m_Texture->type(),0);
        }
        void _destruct(){
            SAFE_DELETE(m_Texture);
        }
        void _resize(FramebufferObject* _fbo,uint w,uint h){
            m_Texture->resize(_fbo,w,h);
            Renderer::bindTexture(m_Texture->type(),0);
        }
};
epriv::FramebufferTexture::FramebufferTexture(FramebufferObject* _fbo,FramebufferAttatchment::Attatchment a,Texture* t):FramebufferObjectAttatchment(_fbo,a,t),m_i(new impl){
    m_i->_init(this,t);
}
epriv::FramebufferTexture::~FramebufferTexture(){ m_i->_destruct(); }
void epriv::FramebufferTexture::resize(FramebufferObject* _fbo,uint w, uint h){ m_i->_resize(_fbo,w,h); }
GLuint epriv::FramebufferTexture::address(){ return m_i->m_Texture->address(); }
Texture* epriv::FramebufferTexture::texture(){ return m_i->m_Texture; }
void epriv::FramebufferTexture::bind(){}
void epriv::FramebufferTexture::unbind(){}

class epriv::RenderbufferObject::impl{
    public:
        GLuint m_RBO;
        void _init(){
            glGenRenderbuffers(1, &m_RBO);
        }
        void _destruct(){
            glDeleteRenderbuffers(1, &m_RBO);
        }
        void _resize(RenderbufferObject& super,uint w, uint h){
            Renderer::bindRBO(m_RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, super.attatchment(), w, h);
            Renderer::unbindRBO();
        }
};
epriv::RenderbufferObject::RenderbufferObject(FramebufferObject* f,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):FramebufferObjectAttatchment(f,a,i),m_i(new impl){
    m_i->_init();
}
epriv::RenderbufferObject::~RenderbufferObject(){ m_i->_destruct(); }
void epriv::RenderbufferObject::resize(FramebufferObject* _fbo,uint w, uint h){ m_i->_resize(*this,w,h); }
GLuint epriv::RenderbufferObject::address(){ return m_i->m_RBO; }

void epriv::RenderbufferObject::bind(){ Renderer::bindRBO(m_i->m_RBO); }
void epriv::RenderbufferObject::unbind(){ Renderer::unbindRBO(); }

namespace Engine{
    namespace epriv{
        struct FramebufferObjectDefaultBindFunctor{void operator()(BindableResource* r) const {
            FramebufferObject& fbo = *(FramebufferObject*)r;
            Renderer::setViewport(0,0,fbo.width(),fbo.height());
            Renderer::bindFBO(&fbo);
            for(auto attatchment:fbo.attatchments()){
                attatchment.second->bind();
            }
        }};
        struct FramebufferObjectDefaultUnbindFunctor{void operator()(BindableResource* r) const {
            FramebufferObject& fbo = *(FramebufferObject*)r;
            for(auto attatchment:fbo.attatchments()){
                attatchment.second->unbind();
            }
            Renderer::unbindFBO();
            glm::uvec2& winSize = Resources::getWindowSize();
            Renderer::setViewport(0,0,winSize.x,winSize.y);
        }};
    };
};

class epriv::FramebufferObject::impl{
    public:
        static FramebufferObjectDefaultBindFunctor DEFAULT_BIND_FUNCTOR;
        static FramebufferObjectDefaultUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        GLuint m_FBO;

        unordered_map<uint,FramebufferObjectAttatchment*> m_Attatchments;
        float m_Divisor;

        uint m_FramebufferWidth; uint m_FramebufferHeight;
        void _baseInit(FramebufferObject* super,uint width, uint height,float divisor){
            m_Divisor = divisor;
            m_FramebufferWidth = uint(float(width) * m_Divisor);
            m_FramebufferHeight = uint(float(height) * m_Divisor);
            super->setCustomBindFunctor(FramebufferObject::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR);
            glGenFramebuffers(1, &m_FBO);
        }
        void _init(FramebufferObject* super,uint width, uint height,float divisor){
            _baseInit(super,width,height,divisor);
        }
        void _init(FramebufferObject* super,uint width, uint height,ImageInternalFormat::Format depthInternalFormat,float divisor){
            _baseInit(super,width,height,divisor);
            RenderbufferObject* rbo;
            if(depthInternalFormat == ImageInternalFormat::Depth24Stencil8 || depthInternalFormat == ImageInternalFormat::Depth32FStencil8)
                rbo = new RenderbufferObject(super,FramebufferAttatchment::DepthAndStencil,depthInternalFormat);
            else if(depthInternalFormat == ImageInternalFormat::StencilIndex8)
                rbo = new RenderbufferObject(super,FramebufferAttatchment::Stencil,depthInternalFormat);
            else
                rbo = new RenderbufferObject(super,FramebufferAttatchment::Depth,depthInternalFormat);
            _attatchRenderbuffer(super,rbo);
        }
        void _destruct(){
            for(auto attatchment:m_Attatchments){
                delete attatchment.second;
            }
            glDeleteFramebuffers(1, &m_FBO);
        }
        void _resize(FramebufferObject* super,uint new_width,uint new_height){
            m_FramebufferWidth = uint(float(new_width) * m_Divisor);
            m_FramebufferHeight = uint(float(new_height) * m_Divisor);
            Renderer::setViewport(0,0,m_FramebufferWidth,m_FramebufferHeight);
            Renderer::bindFBO(m_FBO);
            for(auto attatchment:m_Attatchments){
                attatchment.second->resize(super,new_width,new_height);
            }
        }
        FramebufferTexture* _attatchTexture(FramebufferObject* super,Texture* _t,FramebufferAttatchment::Attatchment a){
            if(m_Attatchments.count(a)) return nullptr;
            Renderer::bindFBO(m_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER,FramebufferAttatchment::at(a),_t->type(),_t->address(),0);
            FramebufferTexture* t = new FramebufferTexture(super,a,_t);
            m_Attatchments.emplace(a,t);
            Renderer::unbindFBO();
            return t;
        }
        RenderbufferObject* _attatchRenderbuffer(FramebufferObject* super,RenderbufferObject* rbo){
            if(m_Attatchments.count(rbo->attatchment())){ return nullptr; }
            Renderer::bindFBO(m_FBO);
            Renderer::bindRBO(rbo);
            glRenderbufferStorage(GL_RENDERBUFFER,rbo->internalFormat(), super->width(), super->height());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,rbo->internalFormat(),GL_RENDERBUFFER,rbo->address());
            m_Attatchments.emplace(rbo->attatchment(),rbo);
            Renderer::unbindRBO();
            Renderer::unbindFBO();
            return rbo;
        }
        bool _check(FramebufferObject* super){
            super->bind();
            GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if(err != GL_FRAMEBUFFER_COMPLETE){
                cout << "Framebuffer completeness in FramebufferObject::impl _check() is incomplete!" << endl;
                cout << "Error is: " << err << std::endl;
                return false;
            }
            return true;
        }
};
epriv::FramebufferObjectDefaultBindFunctor epriv::FramebufferObject::impl::DEFAULT_BIND_FUNCTOR;
epriv::FramebufferObjectDefaultUnbindFunctor epriv::FramebufferObject::impl::DEFAULT_UNBIND_FUNCTOR;

epriv::FramebufferObject::FramebufferObject(string name,uint w,uint h,float divisor):BindableResource(name),m_i(new impl){
    m_i->_init(this,w,h,divisor);
}
epriv::FramebufferObject::FramebufferObject(string name,uint w,uint h,ImageInternalFormat::Format depthInternalFormat,float divisor):BindableResource(name),m_i(new impl){
    m_i->_init(this,w,h,depthInternalFormat,divisor);
}
epriv::FramebufferObject::~FramebufferObject(){ m_i->_destruct(); }
void epriv::FramebufferObject::resize(uint w,uint h){ m_i->_resize(this,w,h); }
epriv::FramebufferTexture* epriv::FramebufferObject::attatchTexture(Texture* t,FramebufferAttatchment::Attatchment a){ 
    return m_i->_attatchTexture(this,t,a); 
}
epriv::RenderbufferObject* epriv::FramebufferObject::attatchRenderBuffer(epriv::RenderbufferObject* t){ 
    return m_i->_attatchRenderbuffer(this,t); 
}
uint epriv::FramebufferObject::width(){ return m_i->m_FramebufferWidth; }
uint epriv::FramebufferObject::height(){ return m_i->m_FramebufferHeight; }
const GLuint& epriv::FramebufferObject::address() const { return m_i->m_FBO; }
unordered_map<uint,epriv::FramebufferObjectAttatchment*>& epriv::FramebufferObject::attatchments(){ return m_i->m_Attatchments; }
bool epriv::FramebufferObject::check(){ return m_i->_check(this); }
float epriv::FramebufferObject::divisor(){ return m_i->m_Divisor; }
