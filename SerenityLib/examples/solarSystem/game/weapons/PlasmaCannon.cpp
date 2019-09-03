#include "PlasmaCannon.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"

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

#include <ecs/Components.h>
#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

using namespace Engine;
using namespace std;

struct PlasmaCannonCollisionFunctor final {
void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
    auto plasmaCannonShipVoid = owner.getUserPointer1();
    auto& plasmaCannonProjectile = *static_cast<PlasmaCannonProjectile*>(owner.getUserPointer());

    auto otherPtrShip = other.getUserPointer1();
    if (otherPtrShip && plasmaCannonShipVoid) {
        if (otherPtrShip != plasmaCannonShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherPtrShip);
            if (otherShip && plasmaCannonProjectile.active) {
                Ship* sourceShip = static_cast<Ship*>(plasmaCannonShipVoid);
                PlasmaCannon& plasmaCannon = *static_cast<PlasmaCannon*>(owner.getUserPointer2());
                auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                auto local = otherHit - other.position();
                if (shields && shields->getHealthCurrent() > 0 && other.getUserPointer() == shields) {
                    shields->receiveHit(normal, local, plasmaCannon.impactRadius, plasmaCannon.impactTime, plasmaCannon.damage);
                    plasmaCannonProjectile.destroy();
                    return;
                }
                if (hull && other.getUserPointer() == hull) {
                    hull->receiveHit(normal, local, plasmaCannon.impactRadius, plasmaCannon.impactTime, plasmaCannon.damage);
                    plasmaCannonProjectile.destroy();
                }
            }
        }
    }
}
};

struct PlasmaCannonInstanceBindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    glm::mat4 parentModel = body.modelMatrix();

    glm::mat4 modelMatrix = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PlasmaCannonInstanceUnbindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_FALSE);
}};

struct PlasmaCannonTailInstanceBindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_TRUE);
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
    glm::mat4 scaleMatrix = glm::scale(body.getScale() * i.getScale());

    glm::mat4 modelMatrix = glm::mat4(1.0f) * translation * rotationMatrix * scaleMatrix;
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", modelMatrix);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);
}};
struct PlasmaCannonTailInstanceUnbindFunctor {void operator()(EngineResource* r) const {
    glDepthMask(GL_FALSE);
}};


PlasmaCannonProjectile::PlasmaCannonProjectile(PlasmaCannon& source, Map& map, const glm::vec3& position, const glm::vec3& forward) {
    entity = map.createEntity();
    currentTime = 0.0f;
    maxTime = 2.5f;

    auto& model = *entity.addComponent<ComponentModel>(ResourceManifest::CannonEffectMesh, Material::WhiteShadeless, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& outline = model.addModel(ResourceManifest::CannonEffectOutlineMesh, ResourceManifest::CannonOutlineMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& head = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& tail = model.addModel(Mesh::Plane, (Material*)(ResourceManifest::CannonTailMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& cannonBody = *entity.addComponent<ComponentBody>(CollisionType::Box);
    model.setCustomBindFunctor(PlasmaCannonInstanceBindFunctor());
    model.setCustomUnbindFunctor(PlasmaCannonInstanceUnbindFunctor());
    model.getModel(0).setColor(0.82f, 1.00f, 0.8f, 1.0f);
    outline.setColor(0.29f, 1.0f, 0.47f, 1.0f);
    head.setColor(0.29f, 1.0f, 0.47f, 1.0f);
    tail.setColor(0.29f, 1.0f, 0.47f, 1.0f);

    head.setPosition(0.0f, 0.0f, -0.38996f);
    head.setScale(0.142f, 0.142f, 0.142f);
    tail.setPosition(0.0f, 0.0f, 0.38371f);
    tail.setScale(0.102f, 0.102f, 0.102f);
    head.setCustomBindFunctor(PlasmaCannonTailInstanceBindFunctor());
    head.setCustomUnbindFunctor(PlasmaCannonTailInstanceUnbindFunctor());
    tail.setCustomBindFunctor(PlasmaCannonTailInstanceBindFunctor());
    tail.setCustomUnbindFunctor(PlasmaCannonTailInstanceUnbindFunctor());

    active = true;
    Ship& s = source.ship;
    auto& shipBody = *s.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position + glm::vec3(0, 0, -model.getModel().mesh()->getRadiusBox().z));
    glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
    cannonBody.setPosition(finalPosition);
    cannonBody.addCollisionFlag(CollisionFlag::NoContactResponse);
    cannonBody.setCollisionGroup(CollisionFilter::_Custom_2); //i belong to weapons (group 2)
    cannonBody.setCollisionMask(CollisionFilter::_Custom_1 | CollisionFilter::_Custom_3); //i should only collide with shields and hull (group 1 and group 3)

    auto shipLinVel = shipBody.getLinearVelocity();
    auto shipAngVel = shipBody.getAngularVelocity();

    cannonBody.setLinearVelocity(shipLinVel, false);
    cannonBody.setAngularVelocity(shipAngVel, false);

    auto offset = source.calculatePredictedVector(cannonBody);
    offset *= glm::vec3(source.travelSpeed);
    cannonBody.applyImpulse(offset.x, offset.y, offset.z, false);
    glm::quat q;
    Math::alignTo(q, -offset);
    cannonBody.setRotation(q); //TODO: change rotation based on launching vector

    cannonBody.setUserPointer(this);
    cannonBody.setUserPointer1(&source.ship);
    cannonBody.setUserPointer2(&source);
    cannonBody.setCollisionFunctor(PlasmaCannonCollisionFunctor());
    cannonBody.setInternalPhysicsUserPointer(&cannonBody);
    const_cast<btRigidBody&>(cannonBody.getBtBody()).setDamping(0.0f, 0.0f);


    light = new PointLight(finalPosition, &map);
    light->setColor(0.64f, 1.0f, 0.75f, 1.0f);
    light->setAttenuation(LightRange::_20);
}
PlasmaCannonProjectile::~PlasmaCannonProjectile() {

}
void PlasmaCannonProjectile::destroy() {
    if (active) {
        active = false;
        entity.destroy();
        if (light) {
            light->destroy();
            SAFE_DELETE(light);
        }
    }
}
void PlasmaCannonProjectile::update(const double& dt) {
    if (active) {
        const float fdt = static_cast<float>(dt);
        currentTime += fdt;
        if (light) {
            auto& lightBody = *light->getComponent<ComponentBody>();
            lightBody.setPosition(entity.getComponent<ComponentBody>()->position());
        }
        if (currentTime >= maxTime) {
            destroy();
        }
    }
}

PlasmaCannon::PlasmaCannon(Ship& ship, Map& map, const glm::vec3& position, const glm::vec3& forward, const float& arc, const uint& _maxCharges, const uint& _damage, const float& _rechargePerRound, const float& _impactRadius, const float& _impactTime, const float& _travelSpeed, const float& _volume) :PrimaryWeaponCannon(ship, position, forward, arc, _maxCharges, _damage, _rechargePerRound, _impactRadius, _impactTime, _travelSpeed, _volume), m_Map(map) {

}
PlasmaCannon::~PlasmaCannon() {

}

void PlasmaCannon::update(const double& dt) {
    for (auto& projectile : m_ActiveProjectiles) {
        projectile->update(dt);
    }
    for (auto& projectile : m_ActiveProjectiles) {
        if (!projectile->active) {
            removeFromVector(m_ActiveProjectiles, projectile);
        }
    }
    PrimaryWeaponCannon::update(dt);
}
const bool PlasmaCannon::fire() {
    auto res = PrimaryWeaponCannon::fire();
    if (res) {
        forceFire();
        return true;
    }
    return false;
}
void PlasmaCannon::forceFire() {
    auto* projectile = new PlasmaCannonProjectile(*this, m_Map, position, forward);
    m_ActiveProjectiles.push_back(projectile);

    auto& shipBody = *ship.getComponent<ComponentBody>();
    auto shipMatrix = shipBody.modelMatrix();
    shipMatrix = glm::translate(shipMatrix, position);
    const glm::vec3 finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);

    auto* sound = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaCannon);
    if (sound) {
        sound->setVolume(volume);
        sound->setPosition(finalPosition);
        sound->setAttenuation(0.1f);
    }
}