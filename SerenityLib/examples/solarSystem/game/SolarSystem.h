#pragma once
#ifndef GAME_SOLARSYSTEM_H
#define GAME_SOLARSYSTEM_H

#include "core/Scene.h"

class Star;
class Ship;
class Planet;
class GameObject;
class EntityWrapper;

class SolarSystem: public Scene{
    private:
        std::unordered_map<std::string,Planet*> m_Planets;

        Ship* player;
        float giGlobal;
        float giDiffuse;
        float giSpecular;

        void _loadFromFile(std::string);
    public:
        std::vector<EntityWrapper*> m_Objects;

        SolarSystem(std::string name, std::string file);
        virtual ~SolarSystem();

        virtual void onEvent(const Event& e);
        virtual void update(const float& dt);

        Ship* getPlayer(){ return player; }
        void setPlayer(Ship* p){ player = p; }

        std::unordered_map<std::string,Planet*>& getPlanets() { return m_Planets; }
};
#endif