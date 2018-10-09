#include "ecs/ComponentBody.h"
#include "ecs/ComponentModel.h"
#include "core/engine/Engine_Math.h"
#include "core/engine/Engine_ThreadManager.h"
#include "core/MeshInstance.h"
#include "core/Mesh.h"

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

#pragma region PhysicsData

ComponentBody::PhysicsData::PhysicsData(){ 
    mass = 0;
    rigidBody = nullptr;
    collision = nullptr;
}
void ComponentBody::PhysicsData::_copy(const ComponentBody::PhysicsData& other) {
    mass = other.mass;
    motionState = other.motionState;
    rigidBody = other.rigidBody;

    Collision* copyCollision = new Collision(*other.collision);
    SAFE_DELETE(collision);
    collision = copyCollision;

    btRigidBody* copyBody = new btRigidBody(*other.rigidBody);
    SAFE_DELETE(rigidBody);
    rigidBody = copyBody;
}
ComponentBody::PhysicsData::PhysicsData(const ComponentBody::PhysicsData& other) : rigidBody(other.rigidBody) {
    rigidBody = nullptr;
    collision = nullptr;
    _copy(other);
}
ComponentBody::PhysicsData& ComponentBody::PhysicsData::operator=(const ComponentBody::PhysicsData& other) {
    if (&other == this)
        return *this;
    _copy(other);
    return *this;
}
ComponentBody::PhysicsData::~PhysicsData() {
    SAFE_DELETE(rigidBody);
    SAFE_DELETE(collision);
}

#pragma endregion

#pragma region NormalData

ComponentBody::NormalData::NormalData(){
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    modelMatrix = glm::mat4(1.0f);
}
void ComponentBody::NormalData::_copy(const ComponentBody::NormalData& other) {
    scale = other.scale;
    position = other.position;
    rotation = other.rotation;
    modelMatrix = other.modelMatrix;
}
ComponentBody::NormalData::NormalData(const ComponentBody::NormalData& other) {
    _copy(other);
}
ComponentBody::NormalData& ComponentBody::NormalData::operator=(const ComponentBody::NormalData& other) {
    if (&other == this)
        return *this;
    _copy(other);
    return *this;
}
ComponentBody::NormalData::~NormalData() {

}

#pragma endregion

#pragma region Component

ComponentBody::ComponentBody(Entity& _e) : ComponentBaseClass(_e) {
    data.n = nullptr;
    data.p = nullptr;
    data.n = new NormalData();
    _physics = false;
    auto& normalData = *data.n;
    normalData.position = glm::vec3(0.0f,0.0f,0.0f);
    normalData.scale = glm::vec3(1.0f,1.0f,1.0f);
    normalData.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    normalData.modelMatrix = glm::mat4(1.0f);
    Math::recalculateForwardRightUp(normalData.rotation, _forward, _right, _up);
}
ComponentBody::ComponentBody(Entity& _e,CollisionType::Type _collisionType) : ComponentBaseClass(_e) {
    data.n = nullptr;
    data.p = nullptr;
    data.p = new PhysicsData();
    _physics = true;
    auto& physicsData = *data.p;
    _forward = glm::vec3(0.0f, 0.0f, -1.0f);  _right = glm::vec3(1.0f, 0.0f, 0.0f);  _up = glm::vec3(0.0f, 1.0f, 0.0f);

    physicsData.motionState = btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
    float _mass = 1.0f;
    physicsData.mass = _mass;

    setCollision(_collisionType, _mass);

    setMass(_mass);
    Collision& collision_ = *physicsData.collision;
    btCollisionShape* _shape = collision_.getShape();
    const btVector3& _inertia = collision_.getInertia();

    btRigidBody::btRigidBodyConstructionInfo CI(_mass, &physicsData.motionState, _shape, _inertia);
    physicsData.rigidBody = new btRigidBody(CI);
    auto& rigidBody = *physicsData.rigidBody;
    rigidBody.setSleepingThresholds(0.015f, 0.015f);
    rigidBody.setFriction(0.3f);
    rigidBody.setDamping(0.1f, 0.4f);//air friction 
    rigidBody.setMassProps(_mass, _inertia);
    rigidBody.updateInertiaTensor();
    rigidBody.setUserPointer(this);
}
void ComponentBody::_copy(const ComponentBody& other) {
    _physics = other._physics;
    owner = other.owner;
    if (_physics) {
        PhysicsData* _newData = new PhysicsData();
        _newData = other.data.p;
        SAFE_DELETE(data.p);
        data.p = _newData;
    }else {
        NormalData* _newData = new NormalData();
        _newData = other.data.n;
        SAFE_DELETE(data.n);
        data.n = _newData;
    }
    _forward = other._forward;
    _right = other._right;
    _up = other._up;
}
void ComponentBody::_move(ComponentBody&& other) {
    owner = other.owner;
    _physics = other._physics;
    if (_physics) {
        data.p = new PhysicsData();
        data.p->_copy(*other.data.p);
    }else{
        data.n = new NormalData();
        data.n->_copy(*other.data.n);
    }
    _forward = other._forward;
    _right = other._right;
    _up = other._up;
}
ComponentBody::~ComponentBody() {
    if (_physics) {
        Physics::removeRigidBody(data.p->rigidBody);
        SAFE_DELETE(data.p);
    }else{
        SAFE_DELETE(data.n);
    }
}
ComponentBody::ComponentBody(const ComponentBody& other) {
    data.n = nullptr;
    data.p = nullptr;
    _copy(other);
}
ComponentBody& ComponentBody::operator=(const ComponentBody& other) {
    if (&other == this)
        return *this;
    _copy(other);
    return *this;
}
ComponentBody::ComponentBody(ComponentBody&& other) noexcept {
    data.n = nullptr;
    data.p = nullptr;
    _move(std::move(other));
}
ComponentBody& ComponentBody::operator=(ComponentBody&& other) noexcept {
    _move(std::move(other));
    return *this;
}


void ComponentBody::alignTo(glm::vec3 direction, float speed) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        //recheck this
        btTransform tr;
        rigidBody.getMotionState()->getWorldTransform(tr);
        //Math::alignTo(Math::btToGLMQuat(tr.getRotation()),direction,speed);
        Math::recalculateForwardRightUp(rigidBody, _forward, _right, _up);
    }else{
        auto& normalData = *data.n;
        Math::alignTo(normalData.rotation, direction, speed);
        Math::recalculateForwardRightUp(normalData.rotation, _forward, _right, _up);
    }
}
void ComponentBody::setCollision(CollisionType::Type _type, float _mass) {
    auto& physicsData = *data.p;
    ComponentModel* modelComponent = owner.getComponent<ComponentModel>();
    if (modelComponent) {
        if (_type == CollisionType::Compound) {
            physicsData.collision = new Collision(*modelComponent, _mass);
        }else{
            physicsData.collision = new Collision(_type, modelComponent->getModel().mesh(), _mass);
        }
    }else{
        physicsData.collision = new Collision(_type, nullptr, _mass);
    }
    Collision& collision_ = *physicsData.collision;
    collision_.getShape()->setUserPointer(this);
    auto& rigidBody = *physicsData.rigidBody;
    Physics::removeRigidBody(&rigidBody);
    rigidBody.setCollisionShape(collision_.getShape());
    rigidBody.setMassProps(physicsData.mass, collision_.getInertia());
    rigidBody.updateInertiaTensor();
    Physics::addRigidBody(&rigidBody);
}
void ComponentBody::translate(glm::vec3 translation, bool local) { ComponentBody::translate(translation.x, translation.y, translation.z, local); }
void ComponentBody::translate(float x, float y, float z, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(rigidBody, v, local);
        setPosition(position() + Engine::Math::btVectorToGLM(v));
    }else{
        auto& normalData = *data.n;
        glm::vec3& _position = normalData.position;
        _position.x += x; _position.y += y; _position.z += z;
        glm::vec3 offset(x, y, z);
        if (local) {
            offset = normalData.rotation * offset;
        }
        setPosition(_position + offset);
    }
}
void ComponentBody::rotate(glm::vec3 rotation, bool local) { ComponentBody::rotate(rotation.x, rotation.y, rotation.z, local); }
void ComponentBody::rotate(float pitch, float yaw, float roll, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        btQuaternion quat = rigidBody.getWorldTransform().getRotation().normalize();
        glm::quat glmquat(quat.w(), quat.x(), quat.y(), quat.z());

        if (abs(pitch) >= 0.001f) glmquat = glmquat * (glm::angleAxis(-pitch, glm::vec3(1, 0, 0)));
        if (abs(yaw) >= 0.001f)   glmquat = glmquat * (glm::angleAxis(-yaw, glm::vec3(0, 1, 0)));
        if (abs(roll) >= 0.001f)  glmquat = glmquat * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));

        quat = btQuaternion(glmquat.x, glmquat.y, glmquat.z, glmquat.w);
        rigidBody.getWorldTransform().setRotation(quat);
    }else{
        auto& normalData = *data.n;
        glm::quat& _rotation = normalData.rotation;
        if (abs(pitch) >= 0.001f) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1, 0, 0)));
        if (abs(yaw) >= 0.001f)   _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0, 1, 0)));
        if (abs(roll) >= 0.001f)  _rotation = _rotation * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));
        Math::recalculateForwardRightUp(_rotation, _forward, _right, _up);
    }
}
void ComponentBody::scale(glm::vec3 amount) { ComponentBody::scale(amount.x, amount.y, amount.z); }
void ComponentBody::scale(float x, float y, float z) {
    if (_physics) {
        auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        if (collision_.getShape() && collision_.getType() == CollisionType::Compound) {
            btCompoundShape* cast = ((btCompoundShape*)(collision_.getShape()));
            for (int i = 0; i < cast->getNumChildShapes(); ++i) {
                btCollisionShape* shape = cast->getChildShape(i);
                btUniformScalingShape* convexHull = dynamic_cast<btUniformScalingShape*>(shape);
                if (convexHull) {
                    convexHull->setLocalScaling(convexHull->getLocalScaling() + btVector3(x, y, z));
                    continue;
                }
                btScaledBvhTriangleMeshShape* triHull = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                if (triHull) {
                    triHull->setLocalScaling(triHull->getLocalScaling() + btVector3(x, y, z));
                }
            }
        }
    }else{
        auto& normalData = *data.n;
        glm::vec3& _scale = normalData.scale;
        _scale.x += x; _scale.y += y; _scale.z += z;
    }
    auto* models = owner.getComponent<ComponentModel>();
    if (models) {
        //epriv::ComponentInternalFunctionality::CalculateRadius(*models);
    }
}
void ComponentBody::setPosition(glm::vec3 newPosition) { ComponentBody::setPosition(newPosition.x, newPosition.y, newPosition.z); }
void ComponentBody::setPosition(float x, float y, float z) {
    if (_physics) {
        auto& physicsData = *data.p;
        btTransform tr;
        tr.setOrigin(btVector3(x, y, z));

        tr.setRotation(physicsData.rigidBody->getOrientation());
        Collision& collision_ = *physicsData.collision;
        if (collision_.getType() == CollisionType::TriangleShapeStatic) {
            Physics::removeRigidBody(physicsData.rigidBody);
            SAFE_DELETE(physicsData.rigidBody);
        }
        physicsData.motionState.setWorldTransform(tr);
        if (collision_.getType() == CollisionType::TriangleShapeStatic) {
            btRigidBody::btRigidBodyConstructionInfo ci(physicsData.mass, &physicsData.motionState, collision_.getShape(), collision_.getInertia());
            physicsData.rigidBody = new btRigidBody(ci);
            physicsData.rigidBody->setUserPointer(this);
            Physics::addRigidBody(physicsData.rigidBody);
        }
        physicsData.rigidBody->setMotionState(&physicsData.motionState); //is this needed?
        physicsData.rigidBody->setWorldTransform(tr);
        physicsData.rigidBody->setCenterOfMassTransform(tr);
    }else{
        auto& normalData = *data.n;
        glm::vec3& _position = normalData.position;
        glm::mat4& _matrix = normalData.modelMatrix;
        _position.x = x; _position.y = y; _position.z = z;
        _matrix[3][0] = x;
        _matrix[3][1] = y;
        _matrix[3][2] = z;
    }
}
void ComponentBody::setRotation(glm::quat newRotation) { ComponentBody::setRotation(newRotation.x, newRotation.y, newRotation.z, newRotation.w); }
void ComponentBody::setRotation(float x, float y, float z, float w) {
    if (_physics) {
        auto& physicsData = *data.p;
        btQuaternion quat(x, y, z, w);
        quat = quat.normalize();
        auto& rigidBody = *physicsData.rigidBody;
        btTransform tr; tr.setOrigin(rigidBody.getWorldTransform().getOrigin());
        tr.setRotation(quat);

        rigidBody.setWorldTransform(tr);
        rigidBody.setCenterOfMassTransform(tr);
        physicsData.motionState.setWorldTransform(tr);

        Math::recalculateForwardRightUp(rigidBody, _forward, _right, _up);

        clearAngularForces();
    }else{
        auto& normalData = *data.n;
        glm::quat newRotation(w, x, y, z);
        newRotation = glm::normalize(newRotation);
        glm::quat& _rotation = normalData.rotation;
        _rotation = newRotation;
        Math::recalculateForwardRightUp(_rotation, _forward, _right, _up);
    }
}
void ComponentBody::setScale(glm::vec3 newScale) { ComponentBody::setScale(newScale.x, newScale.y, newScale.z); }
void ComponentBody::setScale(float x, float y, float z) {
    if (_physics) {
        auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        if (collision_.getShape() && collision_.getType() == CollisionType::Compound) {
            btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collision_.getShape());
            if (compoundShapeCast) {
                int numChildren = compoundShapeCast->getNumChildShapes();
                if (numChildren > 0) {
                    for (int i = 0; i < numChildren; ++i) {
                        btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                        btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                        if (convexHullCast) {
                            convexHullCast->setLocalScaling(btVector3(x, y, z));
                            continue;
                        }
                        btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                        if (triHullCast) {
                            triHullCast->setLocalScaling(btVector3(x, y, z));
                        }
                    }
                }
            }
        }
    }else{
        auto& normalData = *data.n;
        glm::vec3& _scale = normalData.scale;
        _scale.x = x; _scale.y = y; _scale.z = z;
    }
    auto* models = owner.getComponent<ComponentModel>();
    if (models) {
        //epriv::ComponentInternalFunctionality::CalculateRadius(*models);
    }
}
btRigidBody& ComponentBody::getBody(){ return *data.p->rigidBody; }
glm::vec3 ComponentBody::position() { //theres prob a better way to do this
    if (_physics) {
        glm::mat4 m(1.0f);
        btTransform tr;  data.p->rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        return glm::vec3(m[3][0], m[3][1], m[3][2]);
    }
    glm::mat4& _matrix = data.n->modelMatrix;
    return glm::vec3(_matrix[3][0], _matrix[3][1], _matrix[3][2]);
}
glm::vec3 ComponentBody::getScreenCoordinates() { return Math::getScreenCoordinates(position(), false); }
glm::vec3 ComponentBody::getScale() {
    if (_physics) {
        auto& physicsData = *data.p;
        Collision& collision_ = *physicsData.collision;
        if (collision_.getShape() && collision_.getType() == CollisionType::Compound) {
            btCompoundShape* compoundShapeCast = dynamic_cast<btCompoundShape*>(collision_.getShape());
            if (compoundShapeCast) {
                int numChildren = compoundShapeCast->getNumChildShapes();
                if (numChildren > 0) {
                    for (int i = 0; i < numChildren; ++i) {
                        btCollisionShape* shape = compoundShapeCast->getChildShape(i);
                        btUniformScalingShape* convexHullCast = dynamic_cast<btUniformScalingShape*>(shape);
                        if (convexHullCast) {
                            return Math::btVectorToGLM(const_cast<btVector3&>(convexHullCast->getLocalScaling()));
                        }
                        btScaledBvhTriangleMeshShape* triHullCast = dynamic_cast<btScaledBvhTriangleMeshShape*>(shape);
                        if (triHullCast) {
                            return Math::btVectorToGLM(const_cast<btVector3&>(triHullCast->getLocalScaling()));
                        }
                    }
                }
            }
        }
        return glm::vec3(1.0f);
    }
    return data.n->scale;
}
glm::quat ComponentBody::rotation() {
    if (_physics) {
        return Engine::Math::btToGLMQuat(data.p->rigidBody->getWorldTransform().getRotation());
    }
    return data.n->rotation;
}
glm::vec3 ComponentBody::forward() { return _forward; }
glm::vec3 ComponentBody::right() { return _right; }
glm::vec3 ComponentBody::up() { return _up; }
glm::vec3 ComponentBody::getLinearVelocity() { btVector3 v = data.p->rigidBody->getLinearVelocity(); return Engine::Math::btVectorToGLM(v); }
glm::vec3 ComponentBody::getAngularVelocity() { btVector3 v = data.p->rigidBody->getAngularVelocity(); return Engine::Math::btVectorToGLM(v); }
float ComponentBody::mass() { return data.p->mass; }
glm::mat4 ComponentBody::modelMatrix() { //theres prob a better way to do this
    if (_physics) {
        auto& physicsData = *data.p;
        glm::mat4 m(1.0f);
        btTransform tr; physicsData.rigidBody->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(glm::value_ptr(m));
        Collision& collision_ = *physicsData.collision;
        if (collision_.getShape()) {
            m = glm::scale(m, getScale());
        }
        return m;
    }
    return data.n->modelMatrix;
}
void ComponentBody::setDamping(float linear, float angular) { data.p->rigidBody->setDamping(linear, angular); }
void ComponentBody::setDynamic(bool dynamic) {
    if (_physics) {
        auto& physicsData = *data.p;
        auto& rigidBody = *physicsData.rigidBody;
        if (dynamic) {
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
            Physics::addRigidBody(&rigidBody);
            rigidBody.activate();
        }else{
            Physics::removeRigidBody(&rigidBody);
            rigidBody.setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
            ComponentBody::clearAllForces();
            Physics::addRigidBody(&rigidBody);
            rigidBody.activate();
        }
    }
}
void ComponentBody::setLinearVelocity(float x, float y, float z, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(rigidBody, v, local);
        rigidBody.setLinearVelocity(v);
    }
}
void ComponentBody::setLinearVelocity(glm::vec3 velocity, bool local) { ComponentBody::setLinearVelocity(velocity.x, velocity.y, velocity.z, local); }
void ComponentBody::setAngularVelocity(float x, float y, float z, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(rigidBody, v, local);
        rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setAngularVelocity(glm::vec3 velocity, bool local) { ComponentBody::setAngularVelocity(velocity.x, velocity.y, velocity.z, local); }
void ComponentBody::applyForce(float x, float y, float z, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(rigidBody, v, local);
        rigidBody.applyCentralForce(v);
    }
}
void ComponentBody::applyForce(glm::vec3 force, glm::vec3 origin, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(force.x, force.y, force.z);
        Math::translate(rigidBody, v, local);
        rigidBody.applyForce(v, btVector3(origin.x, origin.y, origin.z));
    }
}
void ComponentBody::applyImpulse(float x, float y, float z, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(x, y, z);
        Math::translate(rigidBody, v, local);
        rigidBody.applyCentralImpulse(v);
    }
}
void ComponentBody::applyImpulse(glm::vec3 impulse, glm::vec3 origin, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 v(impulse.x, impulse.y, impulse.z);
        Math::translate(rigidBody, v, local);
        rigidBody.applyImpulse(v, btVector3(origin.x, origin.y, origin.z));
    }
}
void ComponentBody::applyTorque(float x, float y, float z, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 t(x, y, z);
        if (local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorque(t);
    }
}
void ComponentBody::applyTorque(glm::vec3 torque, bool local) { ComponentBody::applyTorque(torque.x, torque.y, torque.z, local); }
void ComponentBody::applyTorqueImpulse(float x, float y, float z, bool local) {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.activate();
        btVector3 t(x, y, z);
        if (local) {
            t = rigidBody.getInvInertiaTensorWorld().inverse() * (rigidBody.getWorldTransform().getBasis() * t);
        }
        rigidBody.applyTorqueImpulse(t);
    }
}
void ComponentBody::applyTorqueImpulse(glm::vec3 torqueImpulse, bool local) { ComponentBody::applyTorqueImpulse(torqueImpulse.x, torqueImpulse.y, torqueImpulse.z, local); }
void ComponentBody::clearLinearForces() {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setLinearVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAngularForces() {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setAngularVelocity(btVector3(0, 0, 0));
    }
}
void ComponentBody::clearAllForces() {
    if (_physics) {
        auto& rigidBody = *data.p->rigidBody;
        btVector3 v(0, 0, 0);
        rigidBody.setActivationState(0);
        rigidBody.activate();
        rigidBody.setLinearVelocity(v);
        rigidBody.setAngularVelocity(v);
    }
}
void ComponentBody::setMass(float mass) {
    if (_physics) {
        auto& physicsData = *data.p;
        physicsData.mass = mass;
        Collision& collision_ = *physicsData.collision;
        if (collision_.getShape()) {
            collision_.setMass(physicsData.mass);
            if (physicsData.rigidBody) {
                physicsData.rigidBody->setMassProps(physicsData.mass, collision_.getInertia());
            }
        }
    }
}

#pragma endregion

#pragma region System

struct ComponentBodyUpdateFunction final {
    static void _defaultUpdate(vector<uint>& _vec, vector<ComponentBody>& _components) {
        for (uint j = 0; j < _vec.size(); ++j) {
            ComponentBody& b = _components[_vec[j]];
            if (b._physics) {
                auto& rigidBody = *b.data.p->rigidBody;
                Engine::Math::recalculateForwardRightUp(rigidBody, b._forward, b._right, b._up);
            }else{
                auto& n = *b.data.n;
                n.modelMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale) * n.modelMatrix;
            }
        }
    }
    void operator()(void* _componentPool, const float& dt) const {
        auto& pool = *(ECSComponentPool<Entity, ComponentBody>*)_componentPool;
        auto& components = pool.dense();
        auto split = epriv::threading::splitVectorIndices(components);
        for (auto& vec : split) {
            epriv::threading::addJobRef(_defaultUpdate, vec, components);
        }
        epriv::threading::waitForAll();
    }
};
struct ComponentBodyComponentAddedToEntityFunction final {void operator()(void* _component) const {
    ComponentBody& component = *(ComponentBody*)_component;
    if (component._physics) {
        auto* _collision = component.data.p->collision;
        component.setCollision((CollisionType::Type)_collision->getType(), component.data.p->mass);
    }
}};
struct ComponentBodyEntityAddedToSceneFunction final {void operator()(void* _componentPool,Entity& _entity) const {
    auto& scene = _entity.scene();
    auto& pool = *(ECSComponentPool<Entity, ComponentBody>*)_componentPool;
    auto& component = *pool.getComponent(_entity);
    if (component._physics) {
        auto& rigidBody = *component.data.p->rigidBody;
        if (&scene == Resources::getCurrentScene()) {
            Physics::addRigidBody(&rigidBody);
        }else{
            Physics::removeRigidBody(&rigidBody);
        }
    }
}};
struct ComponentBodySceneEnteredFunction final {void operator()(void* _componentPool,Scene& _Scene) const {

}};
struct ComponentBodySceneLeftFunction final {void operator()(void* _componentPool, Scene& _Scene) const {

}};
    
ComponentBodySystem::ComponentBodySystem() {
    setUpdateFunction(ComponentBodyUpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentBodyComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentBodyEntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentBodySceneEnteredFunction());
    setOnSceneLeftFunction(ComponentBodySceneLeftFunction());
}

#pragma endregion