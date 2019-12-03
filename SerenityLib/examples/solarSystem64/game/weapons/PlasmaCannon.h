#pragma once
#ifndef GAME_WEAPON_PLASMA_CANNON_H
#define GAME_WEAPON_PLASMA_CANNON_H

#include "../ships/shipSystems/ShipSystemWeapons.h"
#include <vector>

class  PlasmaCannon;
struct PlasmaCannonCollisionFunctor;
struct PlasmaCannonInstanceBindFunctor;
struct PlasmaCannonInstanceUnbindFunctor;
struct PlasmaCannonTailInstanceBindFunctor;
struct PlasmaCannonTailInstanceUnbindFunctor;
struct PlasmaCannonOutlineInstanceBindFunctor;
struct PlasmaCannonOutlineInstanceUnbindFunctor;

struct PlasmaCannonProjectile final : public PrimaryWeaponCannonProjectile {
    PlasmaCannonProjectile(EntityWrapper* target, PlasmaCannon&, Map& map, const glm_vec3& final_world_position, const glm_vec3& forward, const int index, const glm_vec3& chosen_target_pos);
    ~PlasmaCannonProjectile();
};

class PlasmaCannon final : public PrimaryWeaponCannon {
    friend struct PlasmaCannonCollisionFunctor;
    friend struct PlasmaCannonInstanceBindFunctor;
    friend struct PlasmaCannonInstanceUnbindFunctor;
    friend struct PlasmaCannonTailInstanceBindFunctor;
    friend struct PlasmaCannonTailInstanceUnbindFunctor;
    friend struct PlasmaCannonOutlineInstanceBindFunctor;
    friend struct PlasmaCannonOutlineInstanceUnbindFunctor;
    public:
        PlasmaCannon(
            Ship&,
            Map&,
            const glm_vec3& position,
            const glm_vec3& forward,
            const float& arc,
            const uint& maxCharges = 5,
            const float& damage = 100.0f,
            const float& _rechargePerRound = 1.0f,
            const float& _impactRadius = 2.5f,
            const float& _impactTime = 1.8f,
            const float& _travelSpeed = 40.5f,
            const float& _volume = 100.0f,
            const unsigned int& _modelIndex = 0
        );
        ~PlasmaCannon();

        void update(const double& dt);
};

#endif