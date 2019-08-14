#include <ecs/ComponentBody.h>
#include <ecs/ComponentModel.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/ModelInstance.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/scene/Camera.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region PhysicsData

ComponentBody::PhysicsData::PhysicsData(){ 
    //constructor
    mass      = 0;
    group     = CollisionFilter::DefaultFilter;
    mask      = CollisionFilter::AllFilter;
    rigidBody = nullptr;
    collision = nullptr;
}
ComponentBody::PhysicsData::PhysicsData(const ComponentBody::PhysicsData& p_Other){
    //copy constructor
    mass        = p_Other.mass;
    motionState = p_Other.motionState;
    rigidBody   = p_Other.rigidBody;
    group       = p_Other.group;
    mask        = p_Other.mask;

    if (p_Other.collision) collision = new Collision(*p_Other.collision);
    else                   collision = nullptr;

    if (p_Other.rigidBody) rigidBody = new btRigidBody(*p_Other.rigidBody);
    else                   rigidBody = nullptr;
}
ComponentBody::PhysicsData& ComponentBody::PhysicsData::operator=(const ComponentBody::PhysicsData& p_Other) {
    //copy assignment
    if (&p_Other == this)
        return *this;
    ComponentBody::PhysicsData tmp(p_Other); // re-use copy-constructor
    *this = std::move(tmp);                  // re-use move-assignment
    return *this;
}
ComponentBody::PhysicsData::PhysicsData(ComponentBody::PhysicsData&& p_Other) noexcept {
    //move constructor
    using std::swap;
    swap(mass, p_Other.mass);
    swap(motionState, p_Other.motionState);
    swap(group, p_Other.group);
    swap(mask, p_Other.mask);

    if (p_Other.collision) swap(collision, p_Other.collision);
    else                   collision = nullptr;
    if (p_Other.rigidBody) swap(rigidBody, p_Other.rigidBody);
    else                   rigidBody = nullptr;
}
ComponentBody::PhysicsData& ComponentBody::PhysicsData::operator=(ComponentBody::PhysicsData&& p_Other) noexcept {
    //move assignment
    using std::swap;
    swap(mass, p_Other.mass);
    swap(motionState, p_Other.motionState);
    swap(group, p_Other.group);
    swap(mask, p_Other.mask);
    if (p_Other.collision) swap(collision, p_Other.collision);
    else                   collision = nullptr;
    if (p_Other.rigidBody) swap(rigidBody, p_Other.rigidBody);
    else                   rigidBody = nullptr;
    return *this;
}
ComponentBody::PhysicsData::~PhysicsData() {
    //destructor
    //SAFE_DELETE(collision);
    SAFE_DELETE(rigidBody);
}

#pragma endregion

#pragma region NormalData

ComponentBody::NormalData::NormalData(){
    //constructor
    scale       = glm::vec3(1.0f, 1.0f, 1.0f);
    position    = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    modelMatrix = glm::mat4(1.0f);
}
ComponentBody::NormalData::NormalData(const ComponentBody::NormalData& p_Other) {
    //copy constructor
    position = p_Other.position;
    rotation = p_Other.rotation;
    scale = p_Other.scale;
    modelMatrix = p_Other.modelMatrix;
}
ComponentBody::NormalData& ComponentBody::NormalData::operator=(const ComponentBody::NormalData& p_Other) {
    //copy assignment
    if (&p_Other == this)
        return *this;
    ComponentBody::NormalData tmp(p_Other); // re-use copy-constructor
    *this = std::move(tmp);               // re-use move-assignment
    return *this;
}
ComponentBody::NormalData::NormalData(ComponentBody::NormalData&& other) noexcept {
    //move constructor
    using std::swap;
    swap(position, other.position);
    swap(rotation, other.rotation);
    swap(scale, other.scale);
    swap(modelMatrix, other.modelMatrix);
}
ComponentBody::NormalData& ComponentBody::NormalData::operator=(ComponentBody::NormalData&& p_Other) noexcept {
    //move assignment
    using std::swap;
    swap(position, p_Other.position);
    swap(rotation, p_Other.rotation);
    swap(scale, p_Other.scale);
    swap(modelMatrix, p_Other.modelMatrix);
    return *this;
}
ComponentBody::NormalData::~NormalData() {
    //destructor
}


#pragma endregion

#pragma region Component

ComponentBody::ComponentBody(const Entity& p_Entity) : ComponentBaseClass(p_Entity) {
    m_Physics                 = false;
    m_UserPointer             = nullptr;
    m_UserPointer1            = nullptr;
    m_UserPointer2            = nullptr;
    data.p                    = nullptr;
    data.n                    = new NormalData();
    setCollisionFunctor(ComponentBody_EmptyCollisionFunctor());
    auto& normalData          = *data.n;
    normalData.position       = glm::vec3(0.0f,0.0f,0.0f);
    normalData.scale          = glm::vec3(1.0f,1.0f,1.0f);
    normalData.rotation       = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    normalData.modelMatrix    = glm::mat4(1.0f);
    Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
}
ComponentBody::ComponentBody(const Entity& p_Entity, const CollisionType::Type p_CollisionType) : ComponentBaseClass(p_Entity) {
    m_Physics               = true;
    m_UserPointer           = nullptr;
    m_UserPointer1          = nullptr;
    m_UserPointer2          = nullptr;
    data.n                  = nullptr;
    data.p                  = new PhysicsData();
    setCollisionFunctor(ComponentBody_EmptyCollisionFunctor());
    auto& physicsData       = *data.p;
    m_Forward               = glm::vec3(0.0f, 0.0f, -1.0f);
	m_Right                 = glm::vec3(1.0f, 0.0f, 0.0f);
	m_Up                    = glm::vec3(0.0f, 1.0f, 0.0f);

    physicsData.motionState = btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
    float mass              = 1.0f;
    physicsData.mass        = mass;

    setCollision(p_CollisionType, mass);

    setMass(mass);
    Collision& collision     = *physicsData.collision;
    btCollisionShape* shape  = collision.getShape();
    const btVector3& inertia = collision.getInertia();

    btRigidBody::btRigidBodyConstructionInfo CI(mass, &physicsData.motionState, shape, inertia);
    physicsData.rigidBody = new btRigidBody(CI);
    auto& rigidBody = *physicsData.rigidBody;
    rigidBody.setSleepingThresholds(0.015f, 0.015f);
    rigidBody.setFriction(0.3f);
    rigidBody.setDamping(0.1f, 0.4f);//air friction 
    rigidBody.setMassProps(mass, inertia);
    rigidBody.updateInertiaTensor();
    setInternalPhysicsUserPointer(this);
}
ComponentBody::~ComponentBody() {
    //destructor
    if (m_Physics) {
        if(data.p)
            Physics::removeRigidBody(data.p->rigidBody);
        SAFE_DELETE(data.p);
    }else{
        SAFE_DELETE(data.n);
    }
}

ComponentBody::ComponentBody(const ComponentBody& p_Other) {
    //copy constructor
    //Might need more testing here...
	m_Physics          = p_Other.m_Physics;
    m_Forward          = p_Other.m_Forward;
    m_Right            = p_Other.m_Right;
    m_Up               = p_Other.m_Up;
    owner.data         = p_Other.owner.data;
    m_CollisionFunctor = p_Other.m_CollisionFunctor;
    m_UserPointer      = p_Other.m_UserPointer;
    m_UserPointer1     = p_Other.m_UserPointer1;
    m_UserPointer2     = p_Other.m_UserPointer2;
    if (p_Other.m_Physics) {
        if (p_Other.data.p) data.p = new PhysicsData(*p_Other.data.p);
        else                data.p = nullptr;
    }else{
        if (p_Other.data.n) data.n = new NormalData(*p_Other.data.n);
        else                data.n = nullptr;
    }
    setInternalPhysicsUserPointer(this);
}
ComponentBody& ComponentBody::operator=(const ComponentBody& p_Other) {
    //copy assignment
    //Might need more testing here...
    ComponentBody tmp(p_Other); // re-use copy-constructor
    *this = std::move(tmp);   // re-use move-assignment
    setInternalPhysicsUserPointer(this);
    return *this;
}
ComponentBody::ComponentBody(ComponentBody&& p_Other) noexcept {
    //move constructor
    using std::swap;
    swap(m_Physics,p_Other.m_Physics);
    swap(m_Forward, p_Other.m_Forward);
    swap(m_Right, p_Other.m_Right);
    swap(m_Up, p_Other.m_Up);
    swap(owner.data, p_Other.owner.data);
    swap(m_CollisionFunctor, p_Other.m_CollisionFunctor);
    swap(m_UserPointer, p_Other.m_UserPointer);
    swap(m_UserPointer1, p_Other.m_UserPointer1);
    swap(m_UserPointer2, p_Other.m_UserPointer2);
    if (p_Other.m_Physics) {
        swap(data.p, p_Other.data.p);
		p_Other.data.p = nullptr;
    }else{
        swap(data.n, p_Other.data.n);
		p_Other.data.n = nullptr;
    }
    setInternalPhysicsUserPointer(this);
}
ComponentBody& ComponentBody::operator=(ComponentBody&& p_Other) noexcept {
    //move assignment
    using std::swap;
    swap(m_Physics,p_Other.m_Physics);
    swap(m_Forward, p_Other.m_Forward);
    swap(m_Right, p_Other.m_Right);
    swap(m_Up, p_Other.m_Up);
    swap(owner.data, p_Other.owner.data);
    swap(m_CollisionFunctor, p_Other.m_CollisionFunctor);
    swap(m_UserPointer, p_Other.m_UserPointer);
    swap(m_UserPointer1, p_Other.m_UserPointer1);
    swap(m_UserPointer2, p_Other.m_UserPointer2);
    if (p_Other.m_Physics) {
        swap(data.p, p_Other.data.p);
    }else{
        swap(data.n, p_Other.data.n);
    }
    setInternalPhysicsUserPointer(this);
    return *this;
}
//kinda ugly
void ComponentBody::setInternalPhysicsUserPointer(void* userPtr) {
    if (m_Physics) {
        auto rigid = data.p->rigidBody;
        if (rigid) {
            rigid->setUserPointer(userPtr);
            auto shape = rigid->getCollisionShape();
            if (shape) {
                shape->setUserPointer(userPtr);
                if (data.p->collision && data.p->collision->getType() == CollisionType::Compound) {
                    btCompoundShape* compound = dynamic_cast<btCompoundShape*>(shape);
                    if (compound) {
                        const int numChildren = compound->getNumChildShapes();
                        compound->setUserPointer(userPtr); //do we need this?
                        if (numChildren > 0) {
                            for (int i = 0; i < numChildren; ++i) {
                                btCollisionShape* shape = compound->getChildShape(i);
                                shape->setUserPointer(userPtr);
                            }
                        }
                    }
                }
            }
        }
    }
}
void ComponentBody::setUserPointer(void* userPtr) {
    m_UserPointer = userPtr;
}
void ComponentBody::setUserPointer1(void* userPtr) {
    m_UserPointer1 = userPtr;
}
void ComponentBody::setUserPointer2(void* userPtr) {
    m_UserPointer2 = userPtr;
}
void* ComponentBody::getUserPointer() {
    return m_UserPointer;
}
void* ComponentBody::getUserPointer1() {
    return m_UserPointer1;
}
void* ComponentBody::getUserPointer2() {
    return m_UserPointer2;
}
void ComponentBody::collisionResponse(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) {
    if (!m_CollisionFunctor.empty()) { //hacky, but needed for some reason...
        m_CollisionFunctor(std::ref(owner), std::ref(ownerHit), std::ref(other), std::ref(otherHit), std::ref(normal));
    }
}
const ushort ComponentBody::getCollisionGroup() const {
    if (m_Physics) {
        return data.p->group;
    }
    return static_cast<ushort>(0);
}
const ushort ComponentBody::getCollisionMask() const {
    if (m_Physics) {
        return data.p->mask;
    }
    return static_cast<ushort>(0);
}
const ushort ComponentBody::getCollisionFlags() const {
    if (m_Physics) {
        return data.p->rigidBody->getCollisionFlags();
    }
    return static_cast<ushort>(0);
}

float ComponentBody::getDistance(const Entity& p_Other) {
    const glm::vec3& other_position = const_cast<Entity&>(p_Other).getComponent<ComponentBody>()->position();
    return glm::distance(position(), other_position);
}
unsigned long long ComponentBody::getDistanceLL(const Entity& p_Other) {
    const glm::vec3& other_position = const_cast<Entity&>(p_Other).getComponent<ComponentBody>()->position();
    return static_cast<unsigned long long>(glm::distance(position(), other_position));
}
void ComponentBody::alignTo(const glm::vec3& p_Direction, const float p_Speed) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        //recheck this
        btTransform tr;
        rigidBody.getMotionState()->getWorldTransform(tr);
        //Math::alignTo(Math::btToGLMQuat(tr.getRotation()),direction,speed);
        Math::recalculateForwardRightUp(rigidBody, m_Forward, m_Right, m_Up);
    }else{
        auto& normalData = *data.n;
        Math::alignTo(normalData.rotation, p_Direction, p_Speed);
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::setCollision(const CollisionType::Type p_CollisionType, const float p_Mass) {
    auto& physicsData = *data.p;
    if (!physicsData.collision) { //TODO: clean this up, its hacky and evil. its being used on the ComponentBody_EntityAddedToSceneFunction
        ComponentModel* modelComponent = owner.getComponent<ComponentModel>();
        if (modelComponent) {
            if (p_CollisionType == CollisionType::Compound) {
                physicsData.collision = new Collision(this, *modelComponent, p_Mass);
            }else{
                physicsData.collision = new Collision(p_CollisionType, modelComponent->getModel().mesh(), p_Mass);
            }
        }else{
            physicsData.collision = new Collision(p_CollisionType, nullptr, p_Mass);
        }
    }
    Collision& collision = *physicsData.collision;
    auto& shape = *collision.getShape();
    if (physicsData.rigidBody) {
        auto& rigidBody = *physicsData.rigidBody;
        rigidBody.setCollisionShape(&shape);
        rigidBody.setMassProps(physicsData.mass, collision.getInertia());
        rigidBody.updateInertiaTensor();
    }
    setInternalPhysicsUserPointer(this);
}
//double check this...
void ComponentBody::setCollision(Collision* p_Collision) {
    auto& physicsData = *data.p;
    if (physicsData.collision) {
        SAFE_DELETE(physicsData.collision);
    }
    physicsData.collision = p_Collision;
    Collision& collision = *physicsData.collision;
    auto& shape = *collision.getShape();
    if (physicsData.rigidBody) {
        auto& rigidBody = *physicsData.rigidBody;
        rigidBody.setCollisionShape(&shape);
        rigidBody.setMassProps(physicsData.mass, collision.getInertia());
        rigidBody.updateInertiaTensor();
    }
    setInternalPhysicsUserPointer(this);
}

void ComponentBody::translate(const glm::vec3& p_Translation, const bool p_Local) {
	ComponentBody::translate(p_Translation.x, p_Translation.y, p_Translation.z, p_Local);
}
void ComponentBody::translate(const float p_Translation, const bool p_Local) {
	ComponentBody::translate(p_Translation, p_Translation, p_Translation, p_Local);
}
void ComponentBody::translate(const float p_X, const float p_Y, const float p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(p_X, p_Y, p_Z);
        Math::translate(rigidBody, v, p_Local);
        ComponentBody::setPosition(position() + Engine::Math::btVectorToGLM(v));
    }else{
        auto& normalData = *data.n;
        glm::vec3& position_ = normalData.position;
		position_.x += p_X;
		position_.y += p_Y;
		position_.z += p_Z;
        glm::vec3 offset(p_X, p_Y, p_Z);
        if (p_Local) {
            offset = normalData.rotation * offset;
        }
		ComponentBody::setPosition(position_ + offset);
    }
}
void ComponentBody::rotate(const glm::vec3& p_Rotation, const bool p_Local) {
	ComponentBody::rotate(p_Rotation.x, p_Rotation.y, p_Rotation.z, p_Local);
}
void ComponentBody::rotate(const double p_Pitch, const double p_Yaw, const double p_Roll, const bool p_Local) {
	ComponentBody::rotate(static_cast<float>(p_Pitch), static_cast<float>(p_Yaw), static_cast<float>(p_Roll), p_Local);
}
void ComponentBody::rotate(const float p_Pitch, const float p_Yaw, const float p_Roll, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        btQuaternion quat = rigidBody.getWorldTransform().getRotation().normalize();
        glm::quat glmquat(quat.w(), quat.x(), quat.y(), quat.z());
        Math::rotate(glmquat, p_Pitch, p_Yaw, p_Roll);
        quat = btQuaternion(glmquat.x, glmquat.y, glmquat.z, glmquat.w);
        rigidBody.getWorldTransform().setRotation(quat);
        Math::recalculateForwardRightUp(rigidBody, m_Forward, m_Right, m_Up);
    }else{
        auto& normalData = *data.n;
        Math::rotate(normalData.rotation, p_Pitch, p_Yaw, p_Roll);
        Math::recalculateForwardRightUp(normalData.rotation, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::scale(const glm::vec3& p_ScaleAmount) { 
	ComponentBody::scale(p_ScaleAmount.x, p_ScaleAmount.y, p_ScaleAmount.z);
}
void ComponentBody::scale(const float p_ScaleAmount) {
	ComponentBody::scale(p_ScaleAmount, p_ScaleAmount, p_ScaleAmount);
}
void ComponentBody::scale(const float p_X, const float p_Y, const float p_Z) {
    if (m_Physics) {
        const auto& newScale = btVector3(p_X, p_Y, p_Z);
        const auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        auto collisionShape = collision_.getShape();
        if (collisionShape) {
            if (collision_.getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collisionShape);
                if (compoundShapeCast) {
                    const int numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                auto& _convexHullCast = *convexHullCast;
                                const auto& _scl = _convexHullCast.getLocalScaling() + newScale;
                                _convexHullCast.setLocalScaling(_scl);
                                continue;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                auto _triHullCast = *triHullCast;
                                const auto& _scl = _triHullCast.getLocalScaling() + newScale;
                                _triHullCast.setLocalScaling(_scl);
                            }
                        }
                    }
                }
            }else if (collision_.getType() == CollisionType::ConvexHull) {
                btUniformScalingShape* convex = static_cast<btUniformScalingShape*>(collisionShape);
                const auto& _scl = convex->getLocalScaling() + newScale;
                convex->setLocalScaling(_scl);
            }else if (collision_.getType() == CollisionType::TriangleShapeStatic) {
                btScaledBvhTriangleMeshShape* tri = static_cast<btScaledBvhTriangleMeshShape*>(collisionShape);
                const auto& _scl = tri->getLocalScaling() + newScale;
                tri->setLocalScaling(_scl);
            }else if (collision_.getType() == CollisionType::Sphere) {
                btMultiSphereShape* sph = static_cast<btMultiSphereShape*>(collisionShape);
                const auto& _scl = sph->getLocalScaling() + newScale;
                sph->setLocalScaling(_scl);
                sph->setImplicitShapeDimensions(_scl);
                sph->recalcLocalAabb();
            }
        }
    }else {
        auto& normalData = *data.n;
        glm::vec3& scale_ = normalData.scale;
		scale_.x += p_X;
		scale_.y += p_Y;
		scale_.z += p_Z;
    }
    auto* models = owner.getComponent<ComponentModel>();
    if (models) {
        epriv::ComponentModel_Functions::CalculateRadius(*models);
    }
}
void ComponentBody::setPosition(const glm::vec3& p_NewPosition) {
	ComponentBody::setPosition(p_NewPosition.x, p_NewPosition.y, p_NewPosition.z);
}
void ComponentBody::setPosition(const float p_NewPosition) {
	ComponentBody::setPosition(p_NewPosition, p_NewPosition, p_NewPosition);
}
void ComponentBody::setPosition(const float p_X, const float p_Y, const float p_Z) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        tr.setOrigin(btVector3(p_X, p_Y, p_Z));
        tr.setRotation(physicsData.rigidBody->getOrientation());
        Collision& collision = *physicsData.collision;
        if (collision.getType() == CollisionType::TriangleShapeStatic) {
            Physics::removeRigidBody(physicsData.rigidBody);
            SAFE_DELETE(physicsData.rigidBody);
        }
        physicsData.motionState.setWorldTransform(tr);
        if (collision.getType() == CollisionType::TriangleShapeStatic) {
            btRigidBody::btRigidBodyConstructionInfo ci(physicsData.mass, &physicsData.motionState, collision.getShape(), collision.getInertia());
            physicsData.rigidBody = new btRigidBody(ci);
            physicsData.rigidBody->setUserPointer(this);
            Physics::addRigidBody(physicsData.rigidBody, physicsData.group, physicsData.mask);
        }
        physicsData.rigidBody->setMotionState(&physicsData.motionState); //is this needed?
        physicsData.rigidBody->setWorldTransform(tr);
        physicsData.rigidBody->setCenterOfMassTransform(tr);
    }else{
        auto& normalData = *data.n;
        glm::vec3& position_ = normalData.position;
        glm::mat4& modelMatrix_ = normalData.modelMatrix;
		position_.x = p_X;
		position_.y = p_Y;
		position_.z = p_Z;
		modelMatrix_[3][0] = p_X;
		modelMatrix_[3][1] = p_Y;
		modelMatrix_[3][2] = p_Z;
    }
}
void ComponentBody::setGravity(const float& p_X, const float& p_Y, const float& p_Z) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        physicsData.rigidBody->setGravity(btVector3(p_X, p_Y, p_Z));
    }
}
void ComponentBody::setRotation(const glm::quat& p_NewRotation) {
	ComponentBody::setRotation(p_NewRotation.x, p_NewRotation.y, p_NewRotation.z, p_NewRotation.w);
}
void ComponentBody::setRotation(const float p_X, const float p_Y, const float p_Z, const float p_W) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        btQuaternion quat(p_X, p_Y, p_Z, p_W);
        quat = quat.normalize();
        auto& rigidBody = *physicsData.rigidBody;
        btTransform tr; tr.setOrigin(rigidBody.getWorldTransform().getOrigin());
        tr.setRotation(quat);
        rigidBody.setWorldTransform(tr);
        rigidBody.setCenterOfMassTransform(tr);
        physicsData.motionState.setWorldTransform(tr);
        Math::recalculateForwardRightUp(rigidBody, m_Forward, m_Right, m_Up);
        clearAngularForces();
    }else{
        auto& normalData = *data.n;
        glm::quat newRotation(p_W, p_X, p_Y, p_Z);
        newRotation = glm::normalize(newRotation);
        glm::quat& rotation_ = normalData.rotation;
		rotation_ = newRotation;
        Math::recalculateForwardRightUp(rotation_, m_Forward, m_Right, m_Up);
    }
}
void ComponentBody::setScale(const glm::vec3& p_NewScale) {
	ComponentBody::setScale(p_NewScale.x, p_NewScale.y, p_NewScale.z);
}
void ComponentBody::setScale(const float p_NewScale) {
	ComponentBody::setScale(p_NewScale, p_NewScale, p_NewScale);
}
void ComponentBody::setScale(const float p_X, const float p_Y, const float p_Z) {
    if (m_Physics) {
        const auto& newScale = btVector3(p_X, p_Y, p_Z);
        const auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        auto collisionShape = collision_.getShape();
        if (collisionShape) {
            if (collision_.getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collisionShape);
                if (compoundShapeCast) {
                    const int numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                convexHullCast->setLocalScaling(newScale);
                                continue;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                triHullCast->setLocalScaling(newScale);
                            }
                        }
                    }
                }
            }else if (collision_.getType() == CollisionType::ConvexHull) {
                btUniformScalingShape* convex = static_cast<btUniformScalingShape*>(collisionShape);
                convex->setLocalScaling(newScale);
            }else if (collision_.getType() == CollisionType::TriangleShapeStatic) {
                btScaledBvhTriangleMeshShape* tri = static_cast<btScaledBvhTriangleMeshShape*>(collisionShape);
                tri->setLocalScaling(newScale);
            }else if (collision_.getType() == CollisionType::Sphere) {
                btMultiSphereShape* sph = static_cast<btMultiSphereShape*>(collisionShape);
                sph->setLocalScaling(newScale);
                sph->setImplicitShapeDimensions(newScale);
                sph->recalcLocalAabb();
            }
        }
    }else{
        auto& normalData = *data.n;
        glm::vec3& scale = normalData.scale;
        scale.x = p_X;
		scale.y = p_Y;
		scale.z = p_Z;
    }
    auto* models = owner.getComponent<ComponentModel>();
    if (models) {
        epriv::ComponentModel_Functions::CalculateRadius(*models);
    }
}
const glm::vec3 ComponentBody::position() const { //theres prob a better way to do this
    if (m_Physics) {
        glm::mat4 modelMatrix_(1.0f);
        btTransform tr;
		data.p->rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(modelMatrix_));
        return glm::vec3(modelMatrix_[3][0], modelMatrix_[3][1], modelMatrix_[3][2]);
    }
    glm::mat4& modelMatrix_ = data.n->modelMatrix;
    return glm::vec3(modelMatrix_[3][0], modelMatrix_[3][1], modelMatrix_[3][2]);
}
glm::vec3 ComponentBody::getScreenCoordinates(const bool p_ClampToEdge) {
	return Math::getScreenCoordinates(position(), p_ClampToEdge);
}
ScreenBoxCoordinates ComponentBody::getScreenBoxCoordinates(const float p_MinOffset) {
    ScreenBoxCoordinates ret;
    const auto& worldPos    = position();
    float radius            = 0.0001f;
    ComponentModel* model   = owner.getComponent<ComponentModel>();
    const auto& center2DRes = Math::getScreenCoordinates(worldPos, false);
    glm::vec2 center2D      = glm::vec2(center2DRes.x, center2DRes.y);
    if (model) {
        radius = model->radius();
    }else{
        ret.topLeft         = center2D;
        ret.topRight        = center2D;
        ret.bottomLeft      = center2D;
        ret.bottomRight     = center2D;
        ret.inBounds        = center2DRes.z;
        return ret;
    }
    auto& cam                    = *Resources::getCurrentScene()->getActiveCamera();
    const glm::vec3& camvectest  = cam.up();   
    const auto& testRes          = Math::getScreenCoordinates(worldPos + (camvectest * radius), false); 
    const glm::vec2 test         = glm::vec2(testRes.x, testRes.y);
    const auto radius2D          = glm::max(p_MinOffset, glm::distance(test, center2D));
    const float& yPlus           = center2D.y + radius2D;
    const float& yNeg            = center2D.y - radius2D;
    const float& xPlus           = center2D.x + radius2D;
    const float& xNeg            = center2D.x - radius2D;
    ret.topLeft                  = glm::vec2(xNeg,  yPlus);
    ret.topRight                 = glm::vec2(xPlus, yPlus);
    ret.bottomLeft               = glm::vec2(xNeg,  yNeg);
    ret.bottomRight              = glm::vec2(xPlus, yNeg);
    ret.inBounds                 = center2DRes.z;
    return ret;
}
const glm::vec3 ComponentBody::getScale() const {
    if (m_Physics) {
        const auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        auto collisionShape = collision_.getShape();
        if (collisionShape) {
            if (collision_.getType() == CollisionType::Compound) {
                btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collisionShape);
                if (compoundShapeCast) {
                    const int& numChildren = compoundShapeCast->getNumChildShapes();
                    if (numChildren > 0) {
                        for (int i = 0; i < numChildren; ++i) {
                            btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                            btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                            if (convexHullCast) {
                                const auto ret = Math::btVectorToGLM(convexHullCast->getLocalScaling());
                                return ret;
                            }
                            btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                            if (triHullCast) {
                                const auto ret = Math::btVectorToGLM(triHullCast->getLocalScaling());
                                return ret;
                            }
                        }
                    }
                }
            }else if (collision_.getType() == CollisionType::ConvexHull) {
                btUniformScalingShape* convex = static_cast<btUniformScalingShape*>(collisionShape);
                if (convex) {
                    const auto ret = Math::btVectorToGLM(convex->getLocalScaling());
                    return ret;
                }
            }else if (collision_.getType() == CollisionType::TriangleShapeStatic) {
                btScaledBvhTriangleMeshShape* tri = static_cast<btScaledBvhTriangleMeshShape*>(collisionShape);
                if (tri) {
                    const auto ret = Math::btVectorToGLM(tri->getLocalScaling());
                    return ret;
                }
            }else if (collision_.getType() == CollisionType::Sphere) {
                btMultiSphereShape* sph = static_cast<btMultiSphereShape*>(collisionShape);
                if (sph) {
                    const auto ret = Math::btVectorToGLM(sph->getLocalScaling());
                    return ret;
                }
            }
        }
        return glm::vec3(1.0f);
    }
    return data.n->scale;
}
const glm::quat ComponentBody::rotation() const {
    if (m_Physics) {
		const auto& quat = data.p->rigidBody->getWorldTransform().getRotation();
        return Engine::Math::btToGLMQuat(quat);
    }
    return data.n->rotation;
}
const glm::vec3 ComponentBody::forward() const {
	return m_Forward; 
}
const glm::vec3 ComponentBody::right() const {
	return m_Right; 
}
const glm::vec3 ComponentBody::up() const {
	return m_Up; 
}
const glm::vec3 ComponentBody::getLinearVelocity() const  {
    if (m_Physics) {
        const btVector3& v = data.p->rigidBody->getLinearVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    return glm::vec3(0.0f);
}
const glm::vec3 ComponentBody::getAngularVelocity() const  {
    if (m_Physics) {
        const btVector3& v = data.p->rigidBody->getAngularVelocity();
        return Engine::Math::btVectorToGLM(v);
    }
    return glm::vec3(0.0f);
}
const float ComponentBody::mass() const {
	return data.p->mass; 
}
const glm::mat4 ComponentBody::modelMatrix() const { //theres prob a better way to do this
    if (m_Physics) {
        auto& physicsData = *data.p;
        glm::mat4 modelMatrix_(1.0f);
        btTransform tr; physicsData.rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(modelMatrix_));
        Collision& collision_ = *physicsData.collision;
        if (collision_.getShape()) {
			modelMatrix_ = glm::scale(modelMatrix_, getScale());
        }
        return modelMatrix_;
    }
    return data.n->modelMatrix;
}
const btRigidBody& ComponentBody::getBody() const {
	return *data.p->rigidBody;
}
void ComponentBody::setDamping(const float p_LinearFactor, const float p_AngularFactor) {
	data.p->rigidBody->setDamping(p_LinearFactor, p_AngularFactor);
}
void ComponentBody::setCollisionGroup(const short& group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != group) {
            Physics::removeRigidBody(phyData.rigidBody);
            phyData.group = group;
            Physics::addRigidBody(phyData.rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::setCollisionMask(const short& mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != mask) {
            Physics::removeRigidBody(phyData.rigidBody);
            phyData.mask = mask;
            Physics::addRigidBody(phyData.rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::setCollisionGroup(const CollisionFilter::Filter& group) {
    ComponentBody::setCollisionGroup(static_cast<short>(group));
}
void ComponentBody::setCollisionMask(const CollisionFilter::Filter& mask) {
    ComponentBody::setCollisionMask(static_cast<short>(mask));
}
void ComponentBody::addCollisionGroup(const short& group) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.group != (phyData.group | group)) {
            Physics::removeRigidBody(phyData.rigidBody);
            phyData.group = phyData.group | group;
            Physics::addRigidBody(phyData.rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::addCollisionMask(const short& mask) {
    if (m_Physics) {
        auto& phyData = *data.p;
        if (phyData.mask != (phyData.mask | mask)) {
            Physics::removeRigidBody(phyData.rigidBody);
            phyData.mask = phyData.mask | mask;
            Physics::addRigidBody(phyData.rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::addCollisionGroup(const CollisionFilter::Filter& group) {
    ComponentBody::addCollisionGroup(static_cast<short>(group));
}
void ComponentBody::addCollisionMask(const CollisionFilter::Filter& mask) {
    ComponentBody::addCollisionMask(static_cast<short>(mask));
}
void ComponentBody::setCollisionFlag(const short& flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != flag) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(flag);
            Physics::addRigidBody(&rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::setCollisionFlag(const CollisionFlag::Flag& flag) {
    ComponentBody::setCollisionFlag(static_cast<short>(flag));
}
void ComponentBody::addCollisionFlag(const short& flag) {
    if (m_Physics) {
        auto& phyData = *data.p;
        auto& rigidBody = *phyData.rigidBody;
        const auto& currFlags = rigidBody.getCollisionFlags();
        if (currFlags != (currFlags | flag)) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(currFlags | flag);
            Physics::addRigidBody(&rigidBody, phyData.group, phyData.mask);
        }
    }
}
void ComponentBody::addCollisionFlag(const CollisionFlag::Flag& flag) {
    ComponentBody::addCollisionFlag(static_cast<short>(flag));
}


void ComponentBody::setDynamic(const bool p_Dynamic) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        auto& rigidBody = *physicsData.rigidBody;
        if (p_Dynamic) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            Physics::addRigidBody(&rigidBody, physicsData.group, physicsData.mask);
            rigidBody.activate();
        }else{
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            ComponentBody::clearAllForces();
            Physics::addRigidBody(&rigidBody, physicsData.group, physicsData.mask);
            rigidBody.activate();
        }
    }
}
void ComponentBody::setLinearVelocity(const float p_X, const float p_Y, const float p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(p_X, p_Y, p_Z);
        Math::translate(rigidBody, v, p_Local);
        rigidBody.setLinearVelocity(v);
    }
}
void ComponentBody::setLinearVelocity(const double p_X, const double p_Y, const double p_Z, const bool p_Local) {
	ComponentBody::setLinearVelocity(static_cast<float>(p_X), static_cast<float>(p_Y), static_cast<float>(p_Z), p_Local);
}
void ComponentBody::setLinearVelocity(const glm::vec3& p_Velocity, const bool p_Local) {
	ComponentBody::setLinearVelocity(p_Velocity.x, p_Velocity.y, p_Velocity.z, p_Local);
}
void ComponentBody::setAngularVelocity(const float p_X, const float p_Y, const float p_Z, const bool p_Local) {
    if (m_Physics) {
		auto& rigidBody = *data.p->rigidBody;
		rigidBody.activate();
		btVector3 v(p_X, p_Y, p_Z);
		Math::translate(rigidBody, v, p_Local);
		rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setAngularVelocity(const double p_X, const double p_Y, const double p_Z, const bool p_Local) {
	ComponentBody::setAngularVelocity(static_cast<float>(p_X), static_cast<float>(p_Y), static_cast<float>(p_Z), p_Local);
}
void ComponentBody::setAngularVelocity(const glm::vec3& p_Velocity, const bool p_Local) {
	ComponentBody::setAngularVelocity(p_Velocity.x, p_Velocity.y, p_Velocity.z, p_Local);
}
void ComponentBody::applyForce(const float p_X, const float p_Y, const float p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(p_X, p_Y, p_Z);
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyCentralForce(v);
    }
}
void ComponentBody::applyForce(const double p_X, const double p_Y, const double p_Z, const bool p_Local) {
	ComponentBody::applyForce(static_cast<float>(p_X), static_cast<float>(p_Y), static_cast<float>(p_Z), p_Local);
}
void ComponentBody::applyForce(const glm::vec3& p_Force, const glm::vec3& p_Origin, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(p_Force.x, p_Force.y, p_Force.z);
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyForce(v, btVector3(p_Origin.x, p_Origin.y, p_Origin.z));
    }
}
void ComponentBody::applyImpulse(const float p_X, const float p_Y, const float p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(p_X, p_Y, p_Z);
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyCentralImpulse(v);
    }
}
void ComponentBody::applyImpulse(const double p_X, const double p_Y, const double p_Z, const bool p_Local) {
	ComponentBody::applyImpulse(static_cast<float>(p_X), static_cast<float>(p_Y), static_cast<float>(p_Z), p_Local);
}
void ComponentBody::applyImpulse(const glm::vec3& p_Impulse, const glm::vec3& p_Origin, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(p_Impulse.x, p_Impulse.y, p_Impulse.z);
        Math::translate(rigidBody, v, p_Local);
        rigidBody.applyImpulse(v, btVector3(p_Origin.x, p_Origin.y, p_Origin.z));
    }
}
void ComponentBody::applyTorque(const float p_X, const float p_Y, const float p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 t(p_X, p_Y, p_Z);
        if (p_Local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorque(t);
    }
}
void ComponentBody::applyTorque(const double p_X, const double p_Y, const double p_Z, const bool p_Local) {
	ComponentBody::applyTorque(static_cast<float>(p_X), static_cast<float>(p_Y), static_cast<float>(p_Z), p_Local);
}
void ComponentBody::applyTorque(const glm::vec3& p_Torque, const bool p_Local) {
	ComponentBody::applyTorque(p_Torque.x, p_Torque.y, p_Torque.z, p_Local);
}
void ComponentBody::applyTorqueImpulse(const float p_X, const float p_Y, const float p_Z, const bool p_Local) {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 t(p_X, p_Y, p_Z);
        if (p_Local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorqueImpulse(t);
    }
}
void ComponentBody::applyTorqueImpulse(const double p_X, const double p_Y, const double p_Z, const bool p_Local) {
	ComponentBody::applyTorqueImpulse(static_cast<float>(p_X), static_cast<float>(p_Y), static_cast<float>(p_Z), p_Local);
}
void ComponentBody::applyTorqueImpulse(const glm::vec3& p_TorqueImpulse, const bool p_Local) {
	ComponentBody::applyTorqueImpulse(p_TorqueImpulse.x, p_TorqueImpulse.y, p_TorqueImpulse.z, p_Local);
}
void ComponentBody::clearLinearForces() {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setLinearVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAngularForces() {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setAngularVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAllForces() {
    if (m_Physics) {
        auto& rigidBody = *data.p->rigidBody;
        btVector3 v(0, 0, 0);
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setLinearVelocity(v);
        rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setMass(const float p_Mass) {
    if (m_Physics) {
        auto& physicsData = *data.p;
        physicsData.mass = p_Mass;
        Collision& collision = *physicsData.collision;
        if (collision.getShape()) {
            collision.setMass(physicsData.mass);
            if (physicsData.rigidBody) {
                physicsData.rigidBody->setMassProps(physicsData.mass, collision.getInertia());
            }
        }
    }
}

#pragma endregion

#pragma region System

struct epriv::ComponentBody_UpdateFunction final {
    static void _defaultUpdate(const vector<uint>& p_Vector, vector<ComponentBody>& p_Components) {
        for (uint j = 0; j < p_Vector.size(); ++j) {
            ComponentBody& b = p_Components[p_Vector[j]];
            if (b.m_Physics) {
                auto& rigidBody = *b.data.p->rigidBody;
                Engine::Math::recalculateForwardRightUp(rigidBody, b.m_Forward, b.m_Right, b.m_Up);
            }else{
                auto& n = *b.data.n;
                //TODO: implement parent->child relations
                //n.modelMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale) * n.modelMatrix;
                n.modelMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale);
                //Engine::Math::recalculateForwardRightUp(n.rotation, b._forward, b._right, b._up); //double check if this is needed
            }
        }
    }
    void operator()(void* p_ComponentPool, const double& p_Dt, Scene& p_Scene) const {
        auto& pool = *static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool);
        auto& components = pool.pool();
        auto split = epriv::threading::splitVectorIndices(components);
        for (auto& vec : split) {
            epriv::threading::addJobRef(_defaultUpdate, vec, components);
        }
        epriv::threading::waitForAll();
    }
};
struct epriv::ComponentBody_ComponentAddedToEntityFunction final {void operator()(void* p_Component, Entity& p_Entity) const {
}};
struct epriv::ComponentBody_EntityAddedToSceneFunction final {void operator()(void* p_ComponentPool,Entity& p_Entity, Scene& p_Scene) const {
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool);
    auto* _component = pool.getComponent(p_Entity);
    if (_component) {
        auto& component = *_component;
        if (component.m_Physics) {
            auto& physicsData = *component.data.p;
            component.setCollision(static_cast<CollisionType::Type>(physicsData.collision->getType()), physicsData.mass);

            auto currentScene = Resources::getCurrentScene();
            if (currentScene && currentScene == &p_Scene) {
                auto& phyData = *component.data.p;
                Physics::addRigidBody(phyData.rigidBody, phyData.group, phyData.mask);
            }
        }
    }
}};
struct epriv::ComponentBody_SceneEnteredFunction final {void operator()(void* p_ComponentPool,Scene& p_Scene) const {
	auto& pool = (*static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool)).pool();
    for (auto& component : pool) { 
        if (component.m_Physics) {
            auto& phyData = *component.data.p;
            Physics::addRigidBody(phyData.rigidBody, phyData.group, phyData.mask);
        } 
    }
}};
struct epriv::ComponentBody_SceneLeftFunction final {void operator()(void* p_ComponentPool, Scene& p_Scene) const {
	auto& pool = (*static_cast<ECSComponentPool<Entity, ComponentBody>*>(p_ComponentPool)).pool();
    for (auto& component : pool) { 
        if (component.m_Physics) {
            auto& phyData = *component.data.p;
            Physics::removeRigidBody(phyData.rigidBody);
        } 
    }
}};
    
ComponentBody_System::ComponentBody_System() {
    setUpdateFunction(ComponentBody_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentBody_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentBody_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentBody_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentBody_SceneLeftFunction());
}

#pragma endregion