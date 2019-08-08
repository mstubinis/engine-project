#pragma once
#ifndef GAME_RESOURCE_MANIFEST_H
#define GAME_RESOURCE_MANIFEST_H

#include <string>
#include <boost/tuple/tuple.hpp>
#include <core/engine/resources/Handle.h>
#include <unordered_map>

struct ResourceManifest{
    static Handle skyFromSpace;
    static Handle skyFromAtmosphere;
    static Handle groundFromSpace;

    static Handle PlanetMesh;
    static Handle DefiantMesh;
    static Handle RingMesh;


	/*extras*/
	static Handle NovaMesh;
    static Handle MirandaMesh;
	static Handle VenerexMesh;
	static Handle IntrepidMesh;
    static Handle ExcelsiorMesh;
    static Handle ConstitutionMesh;
    static Handle LeviathanMesh;
    static Handle ShrikeMesh;

	static Handle NovaMaterial;
    static Handle MirandaMaterial;
	static Handle VenerexMaterial;
	static Handle IntrepidMaterial;
    static Handle ExcelsiorMaterial;
    static Handle ConstitutionMaterial;
    static Handle DefiantSharkMaterial;
    static Handle ShrikeMaterial;


    static Handle StarMaterial;
    static Handle EarthSkyMaterial;
    static Handle DefiantMaterial;
    static Handle CrosshairMaterial;
    static Handle CrosshairArrowMaterial;
    static Handle StarFlareMaterial;

    //sounds
    static Handle MenuMusic;
    static Handle SoundCloakingActivated;
    static Handle SoundCloakingDeactivated;


    static std::string BasePath;

    static std::unordered_map<std::string, boost::tuple<Handle, Handle, float, float, float>> Ships;

    static void init();
};

#endif