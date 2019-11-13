#include "PlasmaBeam.h"
#include "../map/Map.h"
#include "../ResourceManifest.h"
#include "../Ship.h"
#include "../Helper.h"
#include "../networking/Packet.h"

#include <ecs/Components.h>
#include <core/engine/math/Engine_Math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/norm.hpp>
#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/lights/Lights.h>

#include <core/engine/Engine.h>
#include <core/engine/materials/Material.h>

#include "../ships/shipSystems/ShipSystemShields.h"
#include "../ships/shipSystems/ShipSystemHull.h"

#include <core/engine/renderer/Decal.h>
#include <core/engine/scene/Camera.h>

using namespace Engine;
using namespace std;

struct PlasmaBeamCollisionFunctor final { void operator()(CollisionCallbackEventData& data) const {
    auto phaserShipVoid = data.ownerBody.getUserPointer1();
    auto otherShipVoid = data.otherBody.getUserPointer1();
    if (otherShipVoid && phaserShipVoid) {
        if (otherShipVoid != phaserShipVoid) {//dont hit ourselves!
            Ship* otherShip = static_cast<Ship*>(otherShipVoid);
            if (otherShip) {
                auto& weapon = *static_cast<PlasmaBeam*>(data.ownerBody.getUserPointer2());
                if (weapon.firingTime > 0.0f) {
                    Ship* sourceShip = static_cast<Ship*>(phaserShipVoid);
                    auto* shields = static_cast<ShipSystemShields*>(otherShip->getShipSystem(ShipSystemType::Shields));
                    auto* hull = static_cast<ShipSystemHull*>(otherShip->getShipSystem(ShipSystemType::Hull));
                    auto modelSpacePosition = glm::vec3((glm_vec3(data.otherHit) - data.otherBody.position()) * data.otherBody.rotation());
                    auto finalDamage = static_cast<float>(Resources::dt()) * weapon.damage;
                    if (shields && data.otherBody.getUserPointer() == shields) {
                        const uint shieldSide = static_cast<uint>(shields->getImpactSide(modelSpacePosition));
                        if (shields->getHealthCurrent(shieldSide) > 0) {
                            if (weapon.firingTimeShieldGraphicPing > 0.2f) {
                                shields->receiveHit(data.normal, modelSpacePosition, weapon.impactRadius, weapon.impactTime, finalDamage, shieldSide, true);
                                weapon.firingTimeShieldGraphicPing = 0.0f;
                            }else{
                                shields->receiveHit(data.normal, modelSpacePosition, weapon.impactRadius, weapon.impactTime, finalDamage, shieldSide, false);
                            }
                            return;
                        }
                    }
                    if (hull && data.otherBody.getUserPointer() == hull) {
                        if (weapon.firingTimeShieldGraphicPing > 1.0f) {
                            hull->receiveHit(data.normal, modelSpacePosition, weapon.impactRadius, finalDamage, data.otherModelInstanceIndex, true, true);
                            weapon.firingTimeShieldGraphicPing = 0.0f;
                        }else{
                            hull->receiveHit(data.normal, modelSpacePosition, weapon.impactRadius, finalDamage, data.otherModelInstanceIndex, false, false);
                        }
                    }
                }
            }
        }
    }
}};

struct PlasmaBeamInstanceBindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_TRUE);
    auto& i = *static_cast<ModelInstance*>(r);
    Entity& parent = i.parent();
    auto& body = *parent.getComponent<ComponentBody>();
    PlasmaBeam& beam = *static_cast<PlasmaBeam*>(i.getUserPointer());

    glm::mat4 parentModel = body.modelMatrixRendering();
    glm::mat4 model = parentModel * i.modelMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    Renderer::sendUniform4Safe("Object_Color", i.color());
    Renderer::sendUniform3Safe("Gods_Rays_Color", i.godRaysColor());
    Renderer::sendUniform1Safe("AnimationPlaying", 0);
    Renderer::sendUniformMatrix4Safe("Model", model);
    Renderer::sendUniformMatrix3Safe("NormalMatrix", normalMatrix);

    auto& mesh = *(Mesh*)ResourceManifest::PhaserBeamMesh.get();
    mesh.modifyVertices(0, beam.modPts, MeshModifyFlags::Default);
    mesh.modifyVertices(1, beam.modUvs, MeshModifyFlags::Default | MeshModifyFlags::UploadToGPU);
}};
struct PlasmaBeamInstanceUnbindFunctor { void operator()(EngineResource* r) const {
    //glDepthMask(GL_FALSE);
}};

PlasmaBeam::PlasmaBeam(Ship& ship, Map& map, const glm_vec3& position, const glm_vec3& forward, const float& arc, vector<glm::vec3>& windupPts, const float& damage, const float& _chargeTimerSpeed, const float& _firingTime, const float& _impactRadius, const float& _impactTime, const float& _volume, const uint& _maxCharges, const float& _rechargeTimePerRound, const unsigned int& _modelIndex, const float& endpointExtraScale, const float& beamSizeExtraScale, const float& RangeInKM, const float& BeamLaunchSpeed) : PrimaryWeaponBeam(WeaponType::PlasmaBeam, ship, map, position, forward, arc, damage, _impactRadius, _impactTime, _volume, windupPts, _maxCharges, _rechargeTimePerRound, _chargeTimerSpeed, _firingTime, _modelIndex, endpointExtraScale, beamSizeExtraScale, RangeInKM, BeamLaunchSpeed){
    firstWindupGraphic = map.createEntity();
    secondWindupGraphic = map.createEntity();

    const auto plasmaGreen = glm::vec4(0.0f, 0.93f, 0.6f, 1.0f);
    const auto plasmaTeal = glm::vec4(0.53f, 1.0f, 0.73f, 1.0f);

    auto* model = beamGraphic.addComponent<ComponentModel>(ResourceManifest::PhaserBeamMesh, ResourceManifest::PlasmaBeamMaterial, ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& beamModel1 = model->getModel(0);
    beamModel1.hide();
    beamModel1.setScale(BEAM_SIZE_DEFAULT * additionalBeamSizeScale);

    auto& firstWindupBody = *firstWindupGraphic.addComponent<ComponentBody>();
    auto& secondWindupBody = *secondWindupGraphic.addComponent<ComponentBody>();
    auto& firstWindupModel = *firstWindupGraphic.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);
    auto& secondWindupModel = *secondWindupGraphic.addComponent<ComponentModel>(Mesh::Plane, (Material*)(ResourceManifest::TorpedoGlowMaterial).get(), ShaderProgram::Forward, RenderStage::ForwardParticles);

    auto& firstModel = firstWindupModel.getModel();
    auto& secondModel = secondWindupModel.getModel();

    firstModel.setScale(BEAM_SIZE_DEFAULT_END_POINT * additionalEndPointScale);
    secondModel.setScale(BEAM_SIZE_DEFAULT_END_POINT * additionalEndPointScale);

    firstModel.setColor(plasmaGreen);
    secondModel.setColor(plasmaGreen);

    auto& shipBody = *ship.getComponent<ComponentBody>();
    const glm::vec3 finalPosition = shipBody.position() + Math::rotate_vec3(shipBody.rotation(), position);

    firstWindupLight = new PointLight(finalPosition, &map);
    firstWindupLight->setColor(plasmaGreen);
    firstWindupLight->setAttenuation(LightRange::_7);
    firstWindupLight->deactivate();

    secondWindupLight = new PointLight(finalPosition, &map);
    secondWindupLight->setColor(plasmaGreen);
    secondWindupLight->setAttenuation(LightRange::_7);
    secondWindupLight->deactivate();

    auto& beamModel = *beamGraphic.getComponent<ComponentModel>();
    auto& beamModelOne = beamModel.getModel();
    beamModelOne.setUserPointer(this);
    //beamModelOne.setColor(plasmaGreen);
    beamModelOne.setCustomBindFunctor(PlasmaBeamInstanceBindFunctor());
    beamModelOne.setCustomUnbindFunctor(PlasmaBeamInstanceUnbindFunctor());

    beamLight->setColor(plasmaGreen);

    auto& beamEndBody = *beamEndPointGraphic.getComponent<ComponentBody>();
    auto& beamEndModel = *beamEndPointGraphic.getComponent<ComponentModel>();
    auto& beamModelEnd = beamEndModel.getModel(0);
    beamModelEnd.setColor(plasmaGreen);

    beamEndBody.setUserPointer(this);
    beamEndBody.setUserPointer1(&ship);
    beamEndBody.setUserPointer2(this);
    beamEndBody.setPosition(99999999999.9f);
    beamEndBody.setCollisionFunctor(PlasmaBeamCollisionFunctor());
}
PlasmaBeam::~PlasmaBeam() {
    firstWindupGraphic.destroy();
    secondWindupGraphic.destroy();
    firstWindupLight->destroy();
    secondWindupLight->destroy();
    SAFE_DELETE(firstWindupLight);
    SAFE_DELETE(secondWindupLight);
}
const bool PlasmaBeam::fire(const double& dt, const glm_vec3& chosen_target_pt) {
    auto res2 = isInArc(target, arc);
    targetCoordinates = chosen_target_pt;
    if (res2) {
        auto& targetBody = *target->getComponent<ComponentBody>();
        auto& shipBody = *ship.getComponent<ComponentBody>();
        const auto shipRotation = ship.getRotation();
        const auto shipPosition = ship.getPosition();
        const auto launcherPosition = shipPosition + (shipRotation * position);
        const auto distSquared = glm::distance2(launcherPosition, targetBody.position());

        if (distSquared < rangeInKMSquared) { //100 * 100 should be 10 KM
            const auto res = PrimaryWeaponBeam::fire(dt, chosen_target_pt);
            if (res) {
                return forceFire(dt);
            }
        }
    }
    return false;
}
const bool PlasmaBeam::forceFire(const double& dt) {
    //move the two end flares towards the middle using the interpolation
    if (state == BeamWeaponState::Off) {

        auto& shipBody = *ship.getComponent<ComponentBody>();
        auto shipMatrix = shipBody.modelMatrix();
        shipMatrix = glm::translate(shipMatrix, position);
        const auto finalPosition = glm::vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
        soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaBeam);
        if (soundEffect) {
            soundEffect->setVolume(volume);
            soundEffect->setPosition(finalPosition);
            soundEffect->setAttenuation(0.6f);
        }
        state = BeamWeaponState::JustStarted;
        return true;
    }
    return false;
}
void PlasmaBeam::update(const double& dt) {
    if (state == BeamWeaponState::JustStarted) {
        internal_update_initial_firing(dt);
    }else if (state == BeamWeaponState::WindingUp) {
        internal_update_winding_up(dt);
    }else if (state == BeamWeaponState::Firing) {
        internal_update_firing(dt);
    }else if (state == BeamWeaponState::JustTurnedOff) {
        internal_update_ending(dt);
    }
    PrimaryWeaponBeam::update(dt);
}