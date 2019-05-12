#pragma once
#ifndef GAME_RESOURCE_MANIFEST_H
#define GAME_RESOURCE_MANIFEST_H

#include <string>

struct Handle;
struct ResourceManifest{
    static Handle skyFromSpace;
    static Handle skyFromAtmosphere;
    static Handle groundFromSpace;

    static Handle PlanetMesh;
    static Handle DefiantMesh;
    static Handle RingMesh;

    static Handle StarMaterial;
    static Handle EarthSkyMaterial;
    static Handle DefiantMaterial;
    static Handle CrosshairMaterial;
    static Handle CrosshairArrowMaterial;
    static Handle StarFlareMaterial;

    static std::string BasePath;

    static void init();
};

#endif