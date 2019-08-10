#pragma once
#ifndef GAME_SHIP_SYSTEM_BASE_CLASS_H
#define GAME_SHIP_SYSTEM_BASE_CLASS_H

#include <core/engine/utils/Utils.h>

struct ShipSystemType {enum Type {
    Reactor,
    ThrustersPitch,
    ThrustersYaw,
    ThrustersRoll,
    CloakingDevice,
    Shields,
    ThrustersMain,
    WarpDrive,
    Sensors,
_TOTAL};};

class Ship;
class ShipSystem {
    friend class Ship;
    protected:
        Ship& m_Ship;
        uint  m_Type;
        float m_Health;
        float m_Power;
    public:
        ShipSystem(const uint& type, Ship&);
        virtual ~ShipSystem();

        const bool isOnline() const;

        virtual void update(const double& dt);
};


#endif