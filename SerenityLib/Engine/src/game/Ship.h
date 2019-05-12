#pragma once
#ifndef GAME_SHIP_H
#define GAME_SHIP_H

#include "ecs/Components.h"

class GameCamera;
class Ship;
class SolarSystem;
typedef std::uint32_t   uint;

struct ShipLogicFunctor;

struct ShipSystemType {enum Type {
    Reactor,
    ThrustersPitch,
    ThrustersYaw,
    ThrustersRoll,
    Shields,
    ThrustersMain,
    WarpDrive,
    Sensors,
_TOTAL}; };

class ShipSystem{
    protected:
        Ship* m_Ship;
        uint m_Type;
        float m_Health;
        float m_Power;
    public:
        ShipSystem(uint type,Ship*);
        virtual ~ShipSystem();

        const bool isOnline() const { if(m_Health > 0 && m_Power > 0) return true; return false; }

        virtual void update(const float& dt);
};
class ShipSystemReactor final: public ShipSystem{
    private:
        float m_TotalPowerMax;
        float m_TotalPower;
    public:
        ShipSystemReactor(Ship*, float maxPower, float currentPower = -1);
        ~ShipSystemReactor();

        void update(const float& dt);
};
class ShipSystemMainThrusters final: public ShipSystem{
    public:
        ShipSystemMainThrusters(Ship*);
        ~ShipSystemMainThrusters();

        void update(const float& dt);
};
class ShipSystemPitchThrusters final: public ShipSystem{
    public:
        ShipSystemPitchThrusters(Ship*);
        ~ShipSystemPitchThrusters();

        void update(const float& dt);
};
class ShipSystemYawThrusters final: public ShipSystem{
    public:
        ShipSystemYawThrusters(Ship*);
        ~ShipSystemYawThrusters();

        void update(const float& dt);
};
class ShipSystemRollThrusters final: public ShipSystem{
    public:
        ShipSystemRollThrusters(Ship*);
        ~ShipSystemRollThrusters();

        void update(const float& dt);
};
class ShipSystemShields final: public ShipSystem{
    public:
        ShipSystemShields(Ship*);
        ~ShipSystemShields();

        void update(const float& dt);
};
class ShipSystemWarpDrive final: public ShipSystem{
    public:
        ShipSystemWarpDrive(Ship*);
        ~ShipSystemWarpDrive();

        void update(const float& dt);
};
class ShipSystemSensors final: public ShipSystem{
    public:
        ShipSystemSensors(Ship*);
        ~ShipSystemSensors();

        void update(const float& dt);
};

class Ship: public EntityWrapper {
    friend struct ::ShipLogicFunctor;
    protected:
        std::unordered_map<uint,ShipSystem*> m_ShipSystems;
        bool m_IsPlayer;
        GameCamera* m_PlayerCamera;
        bool m_IsWarping;
        float m_WarpFactor;
        Entity m_Target;
    public:
        Ship(
            Handle& meshHandle,       //Mesh
            Handle& materialHandle,   //Material
            bool = false,             //Player Ship?
            std::string = "Ship",     //Name
            glm::vec3 = glm::vec3(0), //Position
            glm::vec3 = glm::vec3(1), //Scale
            CollisionType::Type = CollisionType::ConvexHull,
            SolarSystem* = nullptr
        );
        virtual ~Ship();

        void onEvent(const Event&);

        void translateWarp(float amount,float dt);
        void toggleWarp(){
            m_IsWarping = !m_IsWarping;
            m_WarpFactor = 0;
        }
        GameCamera* getPlayerCamera(){ return m_PlayerCamera; }
        bool IsPlayer(){ return m_IsPlayer; }
        bool IsWarping(){ return m_IsWarping; }
        ShipSystem* getShipSystem(uint type){ return m_ShipSystems[type]; }
        Entity& getTarget() { return m_Target; }
        Entity& entity() { return m_Entity; }
        void setTarget(const Entity&);
        void setTarget(const std::string&);
};
#endif