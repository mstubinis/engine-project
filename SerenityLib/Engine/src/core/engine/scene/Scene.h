#pragma once
#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include <core/engine/renderer/RendererEnums.h>
#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/events/Engine_EventObject.h>
#include <vector>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

class  Camera;

class  SunLight;
class  DirectionalLight;
class  PointLight;
class  SpotLight;
class  RodLight;

class  Viewport;
class  Skybox;
class  ModelInstance;
struct Entity;

namespace Engine {
namespace epriv {
    class  RenderPipeline;
    struct InternalScenePublicInterface;
    struct EntityPOD;
    template<typename T> class ECS;
};
};
class Scene: public EngineResource, public EventObserver{
    friend class  Engine::epriv::RenderPipeline;
    friend struct Engine::epriv::InternalScenePublicInterface;
    public:
        virtual void update(const double& dt);
        virtual void onEvent(const Event& _event);

        template<typename T> void setOnUpdateFunctor(const T& functor) {
            m_OnUpdateFunctor = boost::bind<void>(functor, this, _1);
        }
    private:
        struct impl; std::unique_ptr<impl>   m_i;
        boost::function<void(const double&)> m_OnUpdateFunctor;
    public:
        Scene(const std::string& name);
        virtual ~Scene();

        const uint& id() const;

        //ecs
        Entity createEntity();
        Entity getEntity(const Engine::epriv::EntityPOD&);
        void removeEntity(const uint& entityID);
        void removeEntity(Entity& entity);

        
        Viewport& getMainViewport();
        Viewport& addViewport(const uint& x, const uint& y, const uint& width, const uint& height, const Camera& camera);

        Camera* getActiveCamera() const;
        void setActiveCamera(Camera&);


        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(const float& r, const float& g, const float& b, const float& a);
        void setBackgroundColor(const glm::vec4& backgroundColor);

        const glm::vec3& getGlobalIllumination() const;
        void setGlobalIllumination(const float global, const float diffuse, const float specular);
        void setGlobalIllumination(const glm::vec3& globalIllumination);

        
        Skybox* skybox() const;
        void setSkybox(Skybox*);

        void centerSceneToObject(const Entity& centerEntity);
};
namespace Engine {
    namespace epriv {
        struct InternalScenePublicInterface final {
            friend class Scene;
            friend class Engine::epriv::RenderPipeline;

            static std::vector<EntityPOD>&           GetEntities(Scene&);
            static std::vector<Viewport*>&           GetViewports(Scene&);
            static std::vector<Camera*>&             GetCameras(Scene&);
            static std::vector<SunLight*>&           GetSunLights(Scene&);
            static std::vector<DirectionalLight*>&   GetDirectionalLights(Scene&);
            static std::vector<PointLight*>&         GetPointLights(Scene&);
            static std::vector<SpotLight*>&          GetSpotLights(Scene&);
            static std::vector<RodLight*>&           GetRodLights(Scene&);

            static void           RenderGeometryOpaque(Scene&, Camera&, const double& dt);
            static void           RenderGeometryTransparent(Scene&, Camera&, const double& dt);
            static void           RenderForwardOpaque(Scene&, Camera&, const double& dt);
            static void           RenderForwardTransparent(Scene&, Camera&, const double& dt);
            static void           AddModelInstanceToPipeline(Scene&, ModelInstance&, const RenderStage::Stage& stage);
            static void           RemoveModelInstanceFromPipeline(Scene&, ModelInstance&, const RenderStage::Stage& stage);
            static ECS<Entity>&   GetECS(Scene&);
            static uint           NumScenes;
        };
    };
};

#endif