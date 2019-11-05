#pragma once
#ifndef GAME_WEAPONS_H
#define GAME_WEAPONS_H

#include "WeaponIncludes.h"
#include "../Ship.h"
#include "../networking/Packet.h"
#include "../Helper.h"
#include "../ResourceManifest.h"
#include "../map/Map.h"
#include <core/engine/math/Engine_Math.h>
#include <core/engine/sounds/Engine_Sounds.h>

#include "DisruptorCannon.h"
#include "PlasmaCannon.h"
#include "PulsePhaser.h"

#include "PhaserBeam.h"
#include "PlasmaBeam.h"

#include "PhotonTorpedo.h"
#include "PhotonTorpedoOld.h"
#include "QuantumTorpedo.h"
#include "KlingonPhotonTorpedo.h"
#include "PlasmaTorpedo.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;


class Weapons {
    public:
        static void spawnProjectile(EntityWrapper* target, ShipWeapon& weapon, Ship& ship, Map& map, const glm_vec3& localPosition, const glm_vec3& forward, const int projectile_index, glm_vec3& chosen_target_pos, const unsigned int modelIndex) {
            auto& shipBody = *ship.getComponent<ComponentBody>();
            auto& shipModel = *ship.getComponent<ComponentModel>();
            glm_vec3 final_world_position;
            
            if (modelIndex > 0) {
                auto& model = shipModel.getModel(modelIndex);
                auto modelPos = glm_vec3(model.position());
                auto modelRot = model.orientation();
                auto part1 = Math::rotate_vec3(modelRot, localPosition);
                glm_vec3 localPos = modelPos + part1;
                auto shipMatrix = shipBody.modelMatrix();
                shipMatrix = glm::translate(shipMatrix, localPos);
                final_world_position = glm_vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
            }else{
                auto shipMatrix = shipBody.modelMatrix();
                shipMatrix = glm::translate(shipMatrix, localPosition);
                final_world_position = glm_vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
            }
            if(target)
                chosen_target_pos = target->getComponent<ComponentBody>()->position() + chosen_target_pos;

            switch (weapon.type) {
                case WeaponType::BorgTorpedo: {
                    break;
                }
                case WeaponType::CuttingBeam: {
                    break;
                }
                case WeaponType::DisruptorBeam: {
                    break;
                }
                case WeaponType::DisruptorCannon: {
                    auto& w = static_cast<DisruptorCannon&>(weapon);
                    auto* projectile = new DisruptorCannonProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addCannonProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundDisruptorCannon);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::KlingonPhotonTorpedo: {
                    auto& w = static_cast<KlingonPhotonTorpedo&>(weapon);
                    auto* projectile = new KlingonPhotonTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundKlingonTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PhaserBeam: {
                    break;
                }
                case WeaponType::PhotonTorpedo: {
                    auto& w = static_cast<PhotonTorpedo&>(weapon);
                    auto* projectile = new PhotonTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPhotonTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PhotonTorpedoOld: {
                    auto& w = static_cast<PhotonTorpedoOld&>(weapon);
                    auto* projectile = new PhotonTorpedoOldProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPhotonTorpedoOld);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PlasmaBeam: {
                    break;
                }
                case WeaponType::PlasmaCannon: {
                    auto& w = static_cast<PlasmaCannon&>(weapon);
                    auto* projectile = new PlasmaCannonProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addCannonProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaCannon);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PlasmaTorpedo: {
                    auto& w = static_cast<PlasmaTorpedo&>(weapon);
                    auto* projectile = new PlasmaTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PulsePhaser: {
                    auto& w = static_cast<PulsePhaser&>(weapon);
                    auto* projectile = new PulsePhaserProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addCannonProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPulsePhaser);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::QuantumTorpedo: {
                    auto& w = static_cast<QuantumTorpedo&>(weapon);
                    auto* projectile = new QuantumTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundQuantumTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(0.9f);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                default: {
                    break;
                }

            }
        }
};

#endif
