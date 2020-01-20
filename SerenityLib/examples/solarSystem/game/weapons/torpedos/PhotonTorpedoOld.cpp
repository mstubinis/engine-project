#include "../Weapons.h"
#include "../../map/Map.h"

#include <ecs/Components.h>
#include <core/engine/math/Engine_Math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/physics/Collision.h>

#include <core/engine/system/Engine.h>
#include <core/engine/materials/Material.h>

#include <core/engine/shaders/ShaderProgram.h>

#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/renderer/Decal.h>
#include <BulletCollision/CollisionShapes/btMultiSphereShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <core/engine/scene/Camera.h>

using namespace Engine;
using namespace std;

struct PhotonTorpedoOldCollisionFunctor final { void operator()(CollisionCallbackEventData& data) const {
    auto torpedoShipVoid = data.ownerBody.getUserPointer1();
    auto& torpedoProjectile = *static_cast<PhotonTorpedoOldProjectile*>(data.ownerBody.getUserPointer());

    auto otherPtrShip = data.otherBody.getUserPointer1();
    if (otherPtrShip && torpedoShipVoid) {
        if (otherPtrShip != torpedoShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherPtrShip);
            if (otherShip && torpedoProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(torpedoShipVoid);
                const auto& sourceAI = sourceShip->getAIType();
                if (sourceShip->IsPlayer() || (sourceAI != AIType::AI_None && sourceAI != AIType::Player_Other)) {
                    auto& torpedo = *static_cast<PhotonTorpedoOld*>(data.ownerBody.getUserPointer2());
                    auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                    auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    auto modelSpacePosition = glm::vec3((glm_vec3(data.otherHit) - data.otherBody.position()) * data.otherBody.rotation());

                    if (shields && data.otherBody.getUserPointer() == shields && torpedoProjectile.active) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(modelSpacePosition));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            torpedoProjectile.clientToServerImpactShields(*sourceShip, false, torpedo.m_Map.getClient(), *otherShip, modelSpacePosition, data.normalOnB, torpedo.impactRadius, torpedo.damage, torpedo.impactTime, shieldSide);
                            return;
                        }
                    }
                    if (hull && data.otherBody.getUserPointer() == hull && torpedoProjectile.active) {
                        /*
                        if (shields) {
                            const uint shieldSide = static_cast<uint>(shields->getImpactSide(local));
                            if (shields->getHealthCurrent(shieldSide) > 0) {
                                torpedoProjectile.clientToServerImpactShields(*sourceShip,false, torpedo.m_Map.getClient(), *otherShip, modelSpacePosition, data.normalOnB, torpedo.impactRadius, torpedo.damage, torpedo.impactTime, shieldSide);
                                return;
                            }
                        }
                        */
                        torpedoProjectile.clientToServerImpactHull(*sourceShip, false, torpedo.m_Map.getClient(), *otherShip, modelSpacePosition, data.normalOnB, torpedo.impactRadius, torpedo.damage, torpedo.impactTime, data.otherModelInstanceIndex);
                    }
                }
            }
        }
    }
}};

struct PhotonTorpedoOldInstanceCoreBindFunctor final { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    const auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrixRendering();
    glm::mat4 model = parentModel * i.modelMatrix();

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    auto worldPos = Math::getMatrixPosition(model);
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
struct PhotonTorpedoOldInstanceCoreUnbindFunctor final { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

struct PhotonTorpedoOldInstanceGlowBindFunctor final { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    const auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrixRendering();
    glm::mat4 model = parentModel * i.modelMatrix();

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PhotonTorpedoOldInstanceGlowUnbindFunctor final { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

struct PhotonTorpedoOldFlareInstanceBindFunctor final { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    auto& cam = *parent.scene().getActiveCamera();
    auto camOrien = cam.getOrientation();

    glm::mat4 parentModel = body.modelMatrixRendering();
    glm::mat4 model = parentModel * i.modelMatrix();
    glm::vec3 worldPos = Math::getMatrixPosition(model);

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
struct PhotonTorpedoOldFlareInstanceUnbindFunctor final { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};


PhotonTorpedoOldProjectile::PhotonTorpedoOldProjectile(EntityWrapper* target, PhotonTorpedoOld& source, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos) :torpedo(source), SecondaryWeaponTorpedoProjectile(map, final_world_position, forward, index) {
    maxTime = 30.5f;
    rotationAngleSpeed = source.rotationAngleSpeed;
    const auto photonRed = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    EntityDataRequest request(entity);
    EntityDataRequest shipRequest(source.ship.entity());

    auto& planeMesh = epriv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    auto& model    = *entity.addComponent<ComponentModel>(request, &planeMesh, (Material*)(ResourceManifest::TorpedoCoreMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles_2);
    auto& glow     = model.addModel(&planeMesh, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& body     = *entity.addComponent<ComponentBody>(request, CollisionType::Sphere);
    
    auto& core = model.getModel(0);
    core.setCustomBindFunctor(PhotonTorpedoOldInstanceCoreBindFunctor());
    core.setCustomUnbindFunctor(PhotonTorpedoOldInstanceCoreUnbindFunctor());
    core.setColor(1.0f, 1.0f, 1.0f, 1.0f);
    core.setScale(0.5f);

    glow.setColor(photonRed);
    glow.setScale(5.6f);
    glow.setCustomBindFunctor(PhotonTorpedoOldInstanceGlowBindFunctor());
    glow.setCustomUnbindFunctor(PhotonTorpedoOldInstanceGlowUnbindFunctor());

    btMultiSphereShape& sph = *static_cast<btMultiSphereShape*>(body.getCollision()->getBtShape());
    const auto& _scl = btVector3(static_cast<btScalar>(TORPEDO_COL_BASE_SCALE), static_cast<btScalar>(TORPEDO_COL_BASE_SCALE), static_cast<btScalar>(TORPEDO_COL_BASE_SCALE));
    sph.setLocalScaling(_scl);
    sph.setMargin(TORPEDO_COL_MARGIN);
    sph.setImplicitShapeDimensions(_scl);
    sph.recalcLocalAabb();

    for (uint i = 0; i < 6; ++i) {
        auto& flare = model.addModel(ResourceManifest::TorpedoFlareMesh, ResourceManifest::TorpedoFlareMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
        float randScale = Helper::GetRandomFloatFromTo(-1.1f, 1.1f);

        flare.setScale(1.1f + randScale);
        flare.setColor(photonRed);
        float angle = (360.0f / 6.0f) * i;

        float speed = 0.1f;
        float speedRand = Helper::GetRandomFloatFromTo(0.0f, 1.0f);
        if (speedRand < 0.5f)
            speed *= -1.0f;

        float randX = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
        float randY = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
        float randZ = Helper::GetRandomFloatFromTo(-1.0f, 1.0f);
        glm::vec3 randVec = glm::normalize(glm::vec3(randX, randY, randZ));

        glm::vec3 spin = glm::vec3(speed, speed, speed);
        PhotonTorpedoOldFlare flareD = PhotonTorpedoOldFlare(spin);
        flareD.start = glm::angleAxis(glm::radians(angle), randVec);
        flare.setOrientation(flareD.start);

        flares.push_back(flareD);
    }

    auto& shipBody = *source.ship.getComponent<ComponentBody>(shipRequest);
    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();

    body.setPosition(final_world_position);
    body.setLinearVelocity(shipLinVel, false);
    body.setAngularVelocity(shipAngVel, false);

    body.addCollisionFlag(CollisionFlag::NoContactResponse);
    body.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    body.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    body.setUserPointer(this);
    body.setUserPointer1(&source.ship);
    body.setUserPointer2(&source);
    body.setCollisionFunctor(PhotonTorpedoOldCollisionFunctor());
    body.setInternalPhysicsUserPointer(&body);

    auto data = source.calculatePredictedVector(target, body, chosen_target_pos);
    auto& offset = data.pedictedVector;
    hasLock = data.hasLock;
    target = data.target;
    glm_quat q = glm_quat(1.0, 0.0, 0.0, 0.0);
    Math::alignTo(q, -offset);
    body.setRotation(q);
    offset *= glm_vec3(source.travelSpeed);

    body.applyImpulse(offset.x, offset.y, offset.z, false);

    body.getBtBody().setActivationState(DISABLE_DEACTIVATION);//this might be dangerous...
    body.setDamping(static_cast<decimal>(0.0), static_cast<decimal>(0.0));
    body.setRotation(q);

    light = NEW PointLight(final_world_position, &map);
    light->setColor(photonRed);
    light->setAttenuation(LightRange::_32);
}
PhotonTorpedoOldProjectile::~PhotonTorpedoOldProjectile() {
}
void PhotonTorpedoOldProjectile::update(const double& dt) {
    if (active) {
        auto& glowModel = *entity.getComponent<ComponentModel>();
        //homing logic
        if (hasLock && target) {

        }
        //flares
        for (size_t i = 0; i < flares.size(); ++i) {
            auto& flare = glowModel.getModel(2 + i);
            flare.rotate(flares[i].spin);
        }
    }
    SecondaryWeaponTorpedoProjectile::update(dt);
}

PhotonTorpedoOld::PhotonTorpedoOld(Ship& ship, Map& map, const glm_vec3& position, const glm_vec3& forward, const float& arc, const uint& _maxCharges, const float& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume, const float& _rotAngleSpeed, const unsigned int& _modelIndex) :SecondaryWeaponTorpedo(map,WeaponType::PhotonTorpedoOld, ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume, _rotAngleSpeed, _modelIndex){

}
PhotonTorpedoOld::~PhotonTorpedoOld() {

}

void PhotonTorpedoOld::update(const double& dt) {
    SecondaryWeaponTorpedo::update(dt);
}
