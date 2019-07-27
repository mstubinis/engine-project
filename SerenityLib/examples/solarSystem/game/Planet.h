#pragma once
#ifndef GAME_PLANET_H
#define GAME_PLANET_H

#include <ecs/Components.h>

class SunLight;
class Ring;
class Planet;
class Map;

struct PlanetLogicFunctor;
struct PlanetaryRingModelInstanceBindFunctor;
struct AtmosphericScatteringGroundModelInstanceBindFunctor;
struct AtmosphericScatteringGroundModelInstanceUnbindFunctor;
struct AtmosphericScatteringSkyModelInstanceBindFunctor;
struct AtmosphericScatteringSkyModelInstanceUnbindFunctor;
struct StarModelInstanceBindFunctor;
struct StarModelInstanceUnbindFunctor;

struct PlanetType{ enum Type {
    Rocky,Ice,GasGiant,IceGiant,Moon,Star,Asteroid,
};};
struct RotationInfo final{
    float tilt;
    float speed;
    RotationInfo(float _tilt, float _days){
        tilt = _tilt;
        speed = 1.0f / (_days * 86400.0f * 360.0f);
    }
};
class OrbitInfo final{
    public:
        glm::vec4 info; //x = eccentricity, y = days, z = minorRadius, w = majorRadius
        float inclination;
        float angle;
        Planet* parent;
        OrbitInfo(float eccentricity, float days, float majorRadius,float angle, Planet& parent, float inclination = 0);
        ~OrbitInfo(){}
        void setOrbitalPosition(float angle, Planet& planet);
        glm::vec3 getOrbitalPosition(float angle, Planet& planet);
};
struct RingInfo final{
    uint position;
    uint size;
    uint alphaBreakpoint;
    glm::ivec3 color;
    RingInfo(uint p, uint s, glm::ivec3 col,uint ab = 1){ 
        position = p;
        size = s; 
        color = col; 
        alphaBreakpoint = ab;
    }
};
class Planet:public EntityWrapper {
    friend class  ::Ring;
    friend struct ::PlanetLogicFunctor;
    friend struct ::PlanetaryRingModelInstanceBindFunctor;
    friend struct ::AtmosphericScatteringGroundModelInstanceBindFunctor;
    friend struct ::AtmosphericScatteringGroundModelInstanceUnbindFunctor;
    friend struct ::AtmosphericScatteringSkyModelInstanceBindFunctor;
    friend struct ::AtmosphericScatteringSkyModelInstanceUnbindFunctor;
    friend struct ::StarModelInstanceBindFunctor;
    friend struct ::StarModelInstanceUnbindFunctor;
    protected:
        std::vector<Ring*>  m_Rings;
        PlanetType::Type    m_Type;
        OrbitInfo*          m_OrbitInfo;
        RotationInfo*       m_RotationInfo;
        float               m_AtmosphereHeight;
    public:
        Planet(
            Handle& materialHandle,               //Material
            PlanetType::Type = PlanetType::Rocky, //Type
            glm::vec3 = glm::vec3(0),             //Position
            float = 1,                            //Radius
            std::string = "Planet",               //Name
            float = 0,                            //Atmosphere size
            Map * = nullptr
        );
        virtual ~Planet();

        glm::vec3 getPosition();
        glm::vec2 getGravityInfo();
        OrbitInfo* getOrbitInfo() const;
        float getGroundRadius();
        float getRadius();
        float getAtmosphereHeight();

        void setPosition(float x,float y,float z);
        void setPosition(glm::vec3 pos);

        void addRing(Ring*);

        void setOrbit(OrbitInfo* o);
        void setRotation(RotationInfo* r);
};
class Star: public Planet{
    private:
        SunLight* m_Light;
    public:
        Star(
            glm::vec3 = glm::vec3(1.0f,1.0f,0.0f),   //Star Color
            glm::vec3 = glm::vec3(1.0f,1.0f,1.0f),   //Sunlight Color
            glm::vec3 = glm::vec3(1.0f,1.0f,1.0f),   //GodRays Color
            glm::vec3 = glm::vec3(0.0f),             //Position
            float = 1,                               //Scale
            std::string = "Star",                    //Name
            Map * = nullptr
        );
        virtual ~Star();
};
class Ring final{
    friend class ::Planet;
    private:
        Handle m_MaterialHandle;
        Planet* m_Parent;
        void _makeRingImage(const std::vector<RingInfo>&);
    public:
        Ring(std::vector<RingInfo>&,Planet*);
        ~Ring();
};
#endif
