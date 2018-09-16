#pragma once
#ifndef ENGINE_OBJECT_COMPONENTS_H
#define ENGINE_OBJECT_COMPONENTS_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Engine_EventObject.h"
#include "Engine_ObjectPool.h"

#include <boost/type_index.hpp>
#include <boost/unordered_map.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

typedef unsigned short ushort;

struct Handle;
class Entity;
class Scene;
class Camera;
class Mesh;
class Material;
class MeshInstance;
class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;

class ComponentBaseClass;
class ComponentBody;
class ComponentModel;
class ComponentCamera;
class Components;

typedef boost::typeindex::type_index   boost_type_index;

namespace Engine{
    namespace epriv{
        class ComponentInternalFunctionality;
        struct MeshMaterialPair;
        class ComponentTypeRegistry;
        class ComponentManager;
        class ComponentCameraSystem;
        class ComponentModelSystem;
        class ComponentBodySystem;
    };
};
class ComponentType{public:enum Type{
    Body,
    Model,
    Camera, //Can contain: ComponentCamera, ComponentGameCamera
_TOTAL,};};

class ComponentBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    friend class ::Entity;
    protected:
        uint m_Owner;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBaseClass(uint owner);
        ComponentBaseClass();
        Entity* owner();
        virtual ~ComponentBaseClass();
};
namespace Engine{

    template<class Base,class Derived> void registerComponent(){
        const boost_type_index baseType    = boost_type_index(boost::typeindex::type_id<Base>());
        const boost_type_index derivedType = boost_type_index(boost::typeindex::type_id<Derived>());
        auto& map = epriv::ComponentTypeRegistry::m_Registry;
        if(!map.count(derivedType)){
            const uint& baseClassSlot = map.at(baseType);
            map.emplace(derivedType,baseClassSlot);
        }
    }

    namespace epriv{
        const uint MAX_NUM_ENTITIES = 131072;
        class ComponentManager final: private Engine::epriv::noncopyable{
            friend class ::Entity;
            friend class ::Scene;
            friend class ::Components;
            friend class ::Engine::epriv::ComponentTypeRegistry;
            friend class ::Engine::epriv::ComponentCameraSystem;
            friend class ::Engine::epriv::ComponentModelSystem;
            friend class ::Engine::epriv::ComponentBodySystem;
            private:
                class impl; std::unique_ptr<impl> m_i;
                ObjectPool<Entity>*                                                 m_EntityPool;
                static ObjectPool<ComponentBaseClass>*                              m_ComponentPool;
                static boost::unordered_map<uint, std::vector<ComponentBaseClass*>> m_ComponentVectors;
                static boost::unordered_map<uint, std::vector<ComponentBaseClass*>> m_ComponentVectorsScene;
            public:
                ComponentManager(const char* name, uint w, uint h);
                ~ComponentManager();

                static void onEntityAddedToScene(Scene*, Entity*);

                void _pause(bool=true);
                void _unpause();
                void _update(const float& dt);
                void _resize(uint width,uint height);

                void _sceneSwap(Scene* oldScene, Scene* newScene);
                void _deleteEntityImmediately(Entity*);
                void _addEntityToBeDestroyed(uint id);
                void _addEntityToBeDestroyed(Entity*);

                void _removeComponent(uint componentID);
                void _removeComponent(ComponentBaseClass* component);
        };
        class ComponentTypeRegistry final: private Engine::epriv::noncopyable{
            friend class ::Entity;
            friend class ::Engine::epriv::ComponentManager;
            private:
                uint m_NextIndex;
            public:
                static boost::unordered_map<boost_type_index,uint> m_Registry;

                static uint slot(const boost_type_index _index) { return m_Registry.at(_index); }

                ComponentTypeRegistry() { m_NextIndex = 0; }
                ~ComponentTypeRegistry() { m_Registry.clear(); m_NextIndex = 0; }

                template<class T> void emplace(){
                    m_Registry.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    ComponentManager::m_ComponentVectors     .emplace(m_NextIndex,std::vector<ComponentBaseClass*>());
                    ComponentManager::m_ComponentVectorsScene.emplace(m_NextIndex,std::vector<ComponentBaseClass*>());
                    ++m_NextIndex;
                }
                template<class T,class V> void emplace(){
                    m_Registry.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    m_Registry.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndex);
                    ComponentManager::m_ComponentVectors     .emplace(m_NextIndex,std::vector<ComponentBaseClass*>());
                    ComponentManager::m_ComponentVectorsScene.emplace(m_NextIndex,std::vector<ComponentBaseClass*>());
                    ++m_NextIndex;
                }
                template<class T,class V,class W> void emplace(){
                    m_Registry.emplace(boost_type_index(boost::typeindex::type_id<T>()),m_NextIndex);
                    m_Registry.emplace(boost_type_index(boost::typeindex::type_id<V>()),m_NextIndex);
                    m_Registry.emplace(boost_type_index(boost::typeindex::type_id<W>()),m_NextIndex);
                    ComponentManager::m_ComponentVectors     .emplace(m_NextIndex,std::vector<ComponentBaseClass*>());
                    ComponentManager::m_ComponentVectorsScene.emplace(m_NextIndex,std::vector<ComponentBaseClass*>());
                    ++m_NextIndex;
                }
        };
        class ComponentSystemBaseClass : private Engine::epriv::noncopyable {
            friend class ::Engine::epriv::ComponentManager;
            private:
            public:
                ComponentSystemBaseClass(){}
                virtual ~ComponentSystemBaseClass(){}
                virtual void update(const float& dt){}
                virtual void onEntityAddedToScene(Scene*, ComponentBaseClass*, Entity*) {}
        };
        class ComponentModelSystem final: public ComponentSystemBaseClass {
            friend class ::Engine::epriv::ComponentManager;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                ComponentModelSystem();
                ~ComponentModelSystem();
                void update(const float& dt);
                void onEntityAddedToScene(Scene*, ComponentBaseClass*, Entity*);
        };
        class ComponentCameraSystem final : public ComponentSystemBaseClass {
            friend class ::Engine::epriv::ComponentManager;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                ComponentCameraSystem();
                ~ComponentCameraSystem();
                void update(const float& dt);
                void onEntityAddedToScene(Scene*, ComponentBaseClass*, Entity*);
        };
        class ComponentBodySystem final : public ComponentSystemBaseClass {
            friend class ::Engine::epriv::ComponentManager;
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                ComponentBodySystem();
                ~ComponentBodySystem();
                void update(const float& dt);
                void onEntityAddedToScene(Scene*, ComponentBaseClass*, Entity*);
        };
    };
};

class ComponentModel: public ComponentBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    friend class ::Engine::epriv::ComponentModelSystem;
    friend class ::Engine::epriv::ComponentInternalFunctionality;
    friend class ::ComponentBody;
    private:
        std::vector<MeshInstance*> models;
        float _radius;
        bool _passedRenderCheck, _visible;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentModel(Handle& meshHandle,Handle& materialHandle,Entity*);
        ComponentModel(Mesh*,Handle& materialHandle,Entity*);
        ComponentModel(Handle& meshHandle,Material*,Entity*);
        ComponentModel(Mesh*,Material*,Entity*);
        ~ComponentModel();

        uint getNumModels();
        float radius();
        bool passedRenderCheck();
        bool visible();
        void show();
        void hide();

        MeshInstance* getModel(uint index = 0);

        uint addModel(Handle& meshHandle, Handle& materialHandle);
        uint addModel(Mesh*,Material*);

        void setModel(Handle& meshHandle,Handle& materialHandle,uint index);
        void setModel(Mesh*,Material*,uint index);

        void setModelMesh(Mesh*,uint index);
        void setModelMesh(Handle& meshHandle, uint index);
        
        void setModelMaterial(Material*,uint index);
        void setModelMaterial(Handle& materialHandle,uint index);

        bool rayIntersectSphere(ComponentCamera* camera);

        template<class T> void setCustomBindFunctor  (T& functor,uint index = 0){ models.at(index)->setCustomBindFunctor(functor); }
        template<class T> void setCustomUnbindFunctor(T& functor,uint index = 0){ models.at(index)->setCustomUnbindFunctor(functor); }
};

class ComponentBody: public ComponentBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    friend class ::Engine::epriv::ComponentBodySystem;
    friend class ::ComponentModel;
    private:
        struct PhysicsData{
            Collision* collision;
            btRigidBody* rigidBody;
            btDefaultMotionState* motionState;
            float mass;
            PhysicsData() {
                collision = 0; rigidBody = 0; motionState = 0;
            }
        };
        struct NormalData{
            glm::vec3* scale;
            glm::vec3* position;
            glm::quat* rotation;
            glm::mat4* modelMatrix;
            NormalData() {
                scale = 0; position = 0; rotation = 0; modelMatrix = 0;
            }
        };
        struct{
            NormalData n;
            PhysicsData p;
        } data;
        bool _physics;
        glm::vec3 _forward, _right, _up;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBody();
        ComponentBody(CollisionType::Type,glm::vec3 scale = glm::vec3(1.0f));
        ~ComponentBody();

        void alignTo(glm::vec3 direction,float speed);

        void translate(glm::vec3 translation,bool local = true);   void translate(float x,float y,float z,bool local = true);
        void rotate(glm::vec3 rotation,bool local = true);         void rotate(float pitch,float yaw,float roll,bool local = true);
        void scale(glm::vec3 amount);                              void scale(float x,float y,float z);

        void setPosition(glm::vec3 newPosition);                   void setPosition(float x,float y,float z);
        void setRotation(glm::quat newRotation);                   void setRotation(float x,float y,float z,float w);
        void setScale(glm::vec3 newScale);                         void setScale(float x,float y,float z);

        float mass();
        glm::vec3 getScreenCoordinates();
        glm::quat rotation();
        glm::vec3 getScale();
        glm::vec3 position();
        glm::vec3 forward();
        glm::vec3 right();
        glm::vec3 up();
        glm::vec3 getLinearVelocity();
        glm::vec3 getAngularVelocity();
        glm::mat4 modelMatrix();
        const btRigidBody* getBody() const;

        void setCollision(CollisionType::Type,float mass,glm::vec3 _scale = glm::vec3(1.0f));
        void setDamping(float linear,float angular);

        void setDynamic(bool dynamic);
        void setMass(float mass);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(float x,float y,float z,bool local = true);   void setLinearVelocity(glm::vec3 velocity,bool local = true);
        void setAngularVelocity(float x,float y,float z,bool local = true);  void setAngularVelocity(glm::vec3 velocity,bool local = true);
        void applyForce(float x,float y,float z,bool local=true);            void applyForce(glm::vec3 force,glm::vec3 origin = glm::vec3(0),bool local=true);
        void applyImpulse(float x,float y,float z,bool local=true);          void applyImpulse(glm::vec3 impulse,glm::vec3 origin = glm::vec3(0),bool local=true);
        void applyTorque(float x,float y,float z,bool local=true);           void applyTorque(glm::vec3 torque,bool local=true);
        void applyTorqueImpulse(float x,float y,float z,bool local=true);    void applyTorqueImpulse(glm::vec3 torqueImpulse,bool local=true);
};

class ComponentCamera: public ComponentBaseClass{
    friend class ::Engine::epriv::ComponentManager;
    friend class ::Engine::epriv::ComponentCameraSystem;
    friend class ::Engine::epriv::ComponentInternalFunctionality;
    friend class ::ComponentModel;
    friend class ::Camera;
    private:
        enum Type{ Perspective, Orthographic, };
        Type _type;
        glm::vec3 _eye,_up;
        glm::mat4 _viewMatrix, _viewMatrixNoTranslation, _projectionMatrix;
        glm::vec4 _planes[6];
        float _nearPlane,  _farPlane,  _bottom,  _top;
        union{ float _angle;        float _left;  };
        union{ float _aspectRatio;  float _right; };
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentCamera();
        ComponentCamera(float angle,float aspectRatio,float nearPlane,float farPlane);
        ComponentCamera(float left,float right,float bottom,float top,float nearPlane,float farPlane);
        ~ComponentCamera();

        virtual void update(const float& dt);
        void resize(uint width,uint height);
        void lookAt(glm::vec3 eye,glm::vec3 forward,glm::vec3 up);

        glm::mat4 getViewProjectionInverse();
        glm::mat4 getProjection();
        glm::mat4 getView();
        glm::mat4 getViewInverse();
        glm::mat4 getProjectionInverse();
        glm::mat4 getViewProjection();
        glm::vec3 getViewVector();
        glm::vec3 getViewVectorNoTranslation();

        float getAngle();    void setAngle(float);
        float getAspect();   void setAspect(float);
        float getNear();     void setNear(float);
        float getFar();      void setFar(float);

        uint pointIntersectTest(glm::vec3& objectPosition);
        uint sphereIntersectTest(glm::vec3& objectPosition,float objectRadius);
};

class Entity: public EventObserver{
    friend class ::Scene;
    friend class ::Engine::epriv::ComponentManager;
    private:
        Scene* m_Scene;
        uint m_ParentID, m_ID;
        std::vector<uint> m_Components;
    public:
        Entity();
        virtual ~Entity();

        const uint id() const;
        Scene* scene();
        virtual void update(const float& dt){}

        void destroy(bool immediate = false); //completely eradicate from memory. by default it its eradicated at the end of the update frame before rendering logic, but can be overrided to be deleted immediately after the call
        Entity* parent();
        void addChild(Entity* child);

        template<class T> T* getComponent(){
            const boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
            const uint& slot = Engine::epriv::ComponentTypeRegistry::slot(typeIndex);
            const uint& componentID = m_Components.at(slot);
            if(componentID == 0){
                return nullptr;
            }
            return Engine::epriv::ComponentManager::m_ComponentPool->getAsFast<T>(componentID);
        }
        template<class T> void addComponent(T* component){
            const boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
            const uint& slot = Engine::epriv::ComponentTypeRegistry::slot(typeIndex);
            uint& componentID = m_Components.at(slot);
            if(componentID != 0) return;
            uint generatedID = Engine::epriv::ComponentManager::m_ComponentPool->add(component);
            Engine::epriv::ComponentManager::m_ComponentVectors.at(slot).push_back(component);
            component->m_Owner = m_ID;
            componentID = generatedID;
            if (m_Scene) {
                Engine::epriv::ComponentManager::m_ComponentVectorsScene.at(slot).push_back(component);
                Engine::epriv::ComponentManager::onEntityAddedToScene(m_Scene, this);
            }
        }
        template<class T> void removeComponent(T* component){
            const boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
            const uint& slot = Engine::epriv::ComponentTypeRegistry::slot(typeIndex);
            uint& componentID = m_Components.at(slot);
            if(componentID == 0) return;
            component->m_Owner = 0;
            removeFromVector(Engine::epriv::ComponentManager::m_ComponentVectors.at(slot),component);
            removeFromVector(Engine::epriv::ComponentManager::m_ComponentVectorsScene.at(slot),component);
            Engine::epriv::ComponentManager::m_ComponentPool->remove(componentID);
            componentID = 0;
        }
};

class Components final{
    friend class ::Engine::epriv::ComponentManager;
    public:

        static ComponentBaseClass* GetComponent(uint id);
        static Entity* GetEntity(uint id);


        template <typename T> static uint getSlot() {
            const boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id<T>());
            return Engine::epriv::ComponentTypeRegistry::m_Registry.at(typeIndex);
        }
        template <typename T> static uint getSlot(T* component) {
            const boost_type_index typeIndex = boost_type_index(boost::typeindex::type_id_runtime(*component));
            return Engine::epriv::ComponentTypeRegistry::m_Registry.at(typeIndex);
        }
};

#endif
