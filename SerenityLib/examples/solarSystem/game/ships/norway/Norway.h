#pragma once
#ifndef GAME_SHIP_NORWAY_CLASS_H
#define GAME_SHIP_NORWAY_CLASS_H

#include "../../Ship.h"

class Norway final : public Ship {
    private:

    public:
        Norway(
            Client& client,
            Map& map,
            bool player = false,                  //Player Ship?
            const std::string& name = "Norway Class Ship",     //Name
            glm::vec3 = glm::vec3(0),             //Position
            glm::vec3 = glm::vec3(1),             //Scale
            CollisionType::Type = CollisionType::ConvexHull
        );
        ~Norway();
};

#endif