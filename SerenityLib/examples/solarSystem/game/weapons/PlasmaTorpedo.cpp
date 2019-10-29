#include "Weapons.h"
#include "../map/Map.h"

#include <ecs/Components.h>
#include <core/engine/math/Engine_Math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/lights/Lights.h>

#include <core/engine/Engine.h>
#include <core/engine/materials/Material.h>

#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/renderer/Decal.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>

using namespace Engine;
using namespace std;

struct PlasmaTorpedoCollisionFunctor final { void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto torpedoShipVoid = owner.getUserPointer1();
    auto& torpedoProjectile = *static_cast<PlasmaTorpedoProjectile*>(owner.getUserPointer());

    auto otherPtrShip = other.getUserPointer1();
    if (otherPtrShip && torpedoShipVoid) {
        if (otherPtrShip != torpedoShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherPtrShip);
            if (otherShip && torpedoProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(torpedoShipVoid);
                if (sourceShip->IsPlayer()) {
                    auto& torpedo = *static_cast<PlasmaTorpedo*>(owner.getUserPointer2());
                    auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                    auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    auto local = otherHit - glm::vec3(other.position());

                    if (shields && other.getUserPointer() == shields) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            torpedoProjectile.clientToServerImpact(torpedo.m_Map.getClient(), *otherShip, local, normal, torpedo.impactRadius, torpedo.damage, torpedo.impactTime, true);
                            return;
                        }
                    }
                    if (hull && other.getUserPointer() == hull) {
                        torpedoProjectile.clientToServerImpact(torpedo.m_Map.getClient(), *otherShip, local, normal, torpedo.impactRadius, torpedo.damage, torpedo.impactTime, false);
                    }
                }
            }
        }
    }
}};

struct PlasmaTorpedoInstanceCoreBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    const auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrix();
    glm::mat4 model = parentModel * i.modelMatrix();

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    auto worldPos = glm::vec3(model[3][0], model[3][1], model[3][2]);
    modelMatrix = glm::translate(modelMatrix, worldPos);
    modelMatrix *= glm::mat4_cast(camOrien);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(body.getScale()) * i.getScale());

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PlasmaTorpedoInstanceCoreUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};
struct PlasmaTorpedoInstanceGlowBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    const auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrix();
    glm::mat4 model = parentModel * i.modelMatrix();

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PlasmaTorpedoInstanceGlowUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};
struct PlasmaTorpedoFlareInstanceBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrix();
    glm::mat4 model = parentModel * i.modelMatrix();
    glm::vec3 worldPos = glm::vec3(model[3][0], model[3][1], model[3][2]);

    glm::mat4 translation = glm::translate(worldPos);
    glm::mat4 rotationMatrix = glm::mat4_cast(camOrien);
    glm::mat4 scaleMatrix = glm::scale(glm::vec3(body.getScale()) * i.getScale());

    glm::mat4 modelMatrix = glm::mat4(1.0f) * translation * rotationMatrix * scaleMatrix;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PlasmaTorpedoFlareInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


PlasmaTorpedoProjectile::PlasmaTorpedoProjectile(PlasmaTorpedo& source, Map& map, const glm_vec3& position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos) : torpedo(source), SecondaryWeaponTorpedoProjectile(map, position, forward, index) {
    maxTime = 30.5f;
    rotationAngleSpeed = source.rotationAngleSpeed;

    EntityDataRequest request(entity);
    EntityDataRequest shipRequest(source.ship.entity());

    auto& model = *entity.addComponent<ComponentModel>(request, Mesh::Plane, (Material*)(ResourceManifest::TorpedoCoreMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles_2);
    auto& glow = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& body = *entity.addComponent<ComponentBody>(request, CollisionType::Sphere);

    auto& core = model.getModel(0);
    core.setCustomBindFunctor(PlasmaTorpedoInstanceCoreBindFunctor());
    core.setCustomUnbindFunctor(PlasmaTorpedoInstanceCoreUnbindFunctor());
    core.setColor(1.0f, 1.0f, 1.0f, 1.0f);

    btMultiSphereShape& sph = *static_cast<btMultiSphereShape*>(body.getCollision()->getBtShape());
    const auto& _scl = btVector3(0.05f, 0.05f, 0.05f);
    sph.setLocalScaling(_scl);
    sph.setMargin(0.135f);
    sph.setImplicitShapeDimensions(_scl);
    sph.recalcLocalAabb();

    const auto plasmaGreen = glm::vec4(0.162f, 0.96f, 0.5f, 1.0f);
    glow.setColor(plasmaGreen);
    glow.setScale(10.6f);
    glow.setCustomBindFunctor(PlasmaTorpedoInstanceGlowBindFunctor());
    glow.setCustomUnbindFunctor(PlasmaTorpedoInstanceGlowUnbindFunctor());

    for (uint i = 0; i < 1; ++i) {
        auto& flare = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::TorpedoFlareTriMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
        flare.setScale(12.2f);
        flare.setColor(plasmaGreen);

        float speed = 0.2f;
        float speedRand = Helper::GetRandomFloatFromTo(0.0f, 1.0f);
        if (speedRand < 0.5f)
            speed *= -1.0f;

        float randAngle = Helper::GetRandomFloatFromTo(0.0f, 360.0f);

        glm::vec3 spin = glm::vec3(0, 0, speed);
        PlasmaTorpedoFlare flareD = PlasmaTorpedoFlare(spin);
        flareD.start = glm::angleAxis(glm::radians(randAngle), glm::vec3(0, 0, 1));
        flare.setOrientation(flareD.start);

        flares.push_back(flareD);
    }
    auto finalPosition = glm_vec3(0.0f);

    auto& shipBody = *source.ship.getComponent<ComponentBody>(shipRequest);
    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    finalPosition = glm_vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
    body.setPosition(finalPosition);
    body.setLinearVelocity(shipLinVel, false);
    body.setAngularVelocity(shipAngVel, false);

    body.addCollisionFlag(CollisionFlag::NoContactResponse);
    body.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    body.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    body.setUserPointer(this);
    body.setUserPointer1(&source.ship);
    body.setUserPointer2(&source);
    body.setCollisionFunctor(PlasmaTorpedoCollisionFunctor());
    body.setInternalPhysicsUserPointer(&body);
    //body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    const_cast<btRigidBody&>(body.getBtBody()).setDamping(0.0f, 0.0f);

    auto data = source.calculatePredictedVector(body, chosen_target_pos);
    auto& offset = data.pedictedVector;
    hasLock = data.hasLock;
    target = data.target;
    glm_quat q;
    Math::alignTo(q, -offset);
    body.setRotation(q);
    offset *= glm::vec3(source.travelSpeed);

    body.applyImpulse(offset.x, offset.y, offset.z, false);

    //body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    const_cast<btRigidBody&>(body.getBtBody()).setDamping(0.0f, 0.0f);
    body.setRotation(q);

    light = new PointLight(finalPosition, &map);
    light->setColor(plasmaGreen);
    light->setAttenuation(LightRange::_20);
}
PlasmaTorpedoProjectile::~PlasmaTorpedoProjectile() {
}
void PlasmaTorpedoProjectile::update(const double& dt) {
    if (active) {
        auto& glowModel = *entity.getComponent<ComponentModel>();
        //homing logic
        if (hasLock && target) {

        }
        //flares
        for (uint i = 0; i < flares.size(); ++i) {
            auto& flare = glowModel.getModel(2 + i);
            flare.rotate(flares[i].spin);
        }
    }
    SecondaryWeaponTorpedoProjectile::update(dt);
}

PlasmaTorpedo::PlasmaTorpedo(Ship& ship, Map& map, const glm_vec3& position, const glm_vec3& forward, const float& arc, const uint& _maxCharges, const float& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume, const float& _rotAngleSpeed) :SecondaryWeaponTorpedo(map,WeaponType::PlasmaTorpedo, ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume, _rotAngleSpeed){

}
PlasmaTorpedo::~PlasmaTorpedo() {

}

void PlasmaTorpedo::update(const double& dt) {
    SecondaryWeaponTorpedo::update(dt);
}
