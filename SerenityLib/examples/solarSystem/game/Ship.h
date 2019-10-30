#pragma once
#ifndef GAME_SHIP_H
#define GAME_SHIP_H

#include <ecs/Components.h>
#include <glm/vec2.hpp>
#include <core/engine/utils/Utils.h>
#include <core/engine/events/Engine_EventObject.h>
#include "Client.h"

#include "ships/shipSystems/ShipSystemBaseClass.h"

#define WARP_PHYSICS_MODIFIER 1.333333333f
#define PHYSICS_PACKET_TIMER_LIMIT 0.25f

class  GameCamera;
class  Ship;
class  Map;
class  Decal;
struct PacketPhysicsUpdate;
struct PacketCloakUpdate;
struct PacketHealthUpdate;
struct PacketProjectileImpact;
struct ShipLogicFunctor;
struct GameCameraLogicFunctor;
class  ShipSystemReactor;
class  ShipSystemMainThrusters;
class  ShipSystemYawThrusters;
class  ShipSystemPitchThrusters;
class  ShipSystemRollThrusters;
class  ShipSystemCloakingDevice;
class  ShipSystemWarpDrive;
class  ShipSystemSensors;
class  ShipSystemShields;
class  ShipSystemWeapons;
class  ShipSystemHull;
class  ShipSystem;

struct PrimaryWeaponBeam;
struct PrimaryWeaponCannon;
struct SecondaryWeaponTorpedo;

class Ship: public EntityWrapper, public EventObserver {
    friend struct GameCameraLogicFunctor;
    friend struct ShipLogicFunctor;
    friend  class ShipSystemReactor;
    friend  class ShipSystemMainThrusters;
	friend  class ShipSystemYawThrusters;
	friend  class ShipSystemPitchThrusters;
	friend  class ShipSystemRollThrusters;
    friend  class ShipSystemCloakingDevice;
    friend  class ShipSystemWarpDrive;
    friend  class ShipSystemSensors;
    friend  class ShipSystemShields;
    friend  class ShipSystemWeapons;
    friend  class ShipSystemHull;
	friend  class ShipSystem;
    protected:
        Client&                              m_Client;
        std::unordered_map<uint,ShipSystem*> m_ShipSystems;
        bool                                 m_IsPlayer;
        GameCamera*                          m_PlayerCamera;
		glm::dvec2                           m_MouseFactor;
        bool                                 m_IsWarping;
        float                                m_WarpFactor;
        std::string                          m_ShipClass;
        std::vector<Decal*>                  m_DamageDecals;
        std::vector<glm::vec3>               m_AimPositionDefaults;
        glm::vec3                            m_CameraOffsetDefault;
    public:
        Ship(
            Client& client,
            const std::string& shipClass,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Ship",     //Name
            const glm_vec3 = glm_vec3(0),         //Position
            const glm_vec3 = glm_vec3(1),         //Scale
            CollisionType::Type = CollisionType::ConvexHull,
            const glm::vec3 aimPosDefault = glm::vec3(0.0f),
            const glm::vec3 camOffsetDefault = glm::vec3(0.0f)
        );
        virtual ~Ship();

        virtual void update(const double& dt);

        void onEvent(const Event&);

        void destroy();

        void addHullTargetPoints(std::vector<glm::vec3>& points);

        const std::string getName();
        const glm_vec3 getWarpSpeedVector3();
        const glm_vec3 getPosition();
        const glm::vec3 getAimPositionDefault();
        const glm::vec3 getAimPositionRandom();
        const glm::vec3 getAimPositionDefaultLocal();
        const glm::vec3 getAimPositionRandomLocal();
        const uint getAimPositionRandomLocalIndex();
        const glm::vec3 getAimPositionLocal(const uint index);
        const glm_quat getRotation();
        const glm_vec3 getPosition(const EntityDataRequest& dataRequest);
        const glm_quat getRotation(const EntityDataRequest& dataRequest);

        void updatePhysicsFromPacket(const PacketPhysicsUpdate& packet, Map& map, std::vector<std::string>& info);
        void updateCloakFromPacket(const PacketCloakUpdate& packet);
        void updateHealthFromPacket(const PacketHealthUpdate& packet);

        void updateDamageDecalsCloak(const float& alpha);
        void updateProjectileImpact(const PacketProjectileImpact& packet);

        void setModel(Handle& handle);
        const float updateShipDimensions();

        const glm_vec3 getLinearVelocity();

        void translateWarp(const double& amount, const double& dt);
        void toggleWarp();
        bool canSeeCloak();
        bool cloak(const bool sendPacket = true);
        bool decloak(const bool sendPacket = true);

        const std::string& getClass() const;
        GameCamera* getPlayerCamera();
        const bool IsPlayer() const;
        const bool IsWarping() const;
        const bool isCloaked();
        const bool isFullyCloaked();
        Entity& entity();
        ShipSystem* getShipSystem(const uint type);
        EntityWrapper* getTarget();      
        void setTarget(EntityWrapper* entityWrapper, const bool sendPacket);
        void setTarget(const std::string&, const bool sendPacket);

        const glm_vec3& forward();
        const glm_vec3& right();
        const glm_vec3& up();


        PrimaryWeaponBeam& getPrimaryWeaponBeam(const uint index);
        PrimaryWeaponCannon& getPrimaryWeaponCannon(const uint index);
        SecondaryWeaponTorpedo& getSecondaryWeaponTorpedo(const uint index);
};
#endif
