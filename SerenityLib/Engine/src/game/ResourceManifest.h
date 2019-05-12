#pragma once
#ifndef GAME_RESOURCE_MANIFEST_H
#define GAME_RESOURCE_MANIFEST_H

struct Handle;

struct ResourceManifest{
    static Handle skyFromSpace;
    static Handle skyFromAtmosphere;
    static Handle groundFromSpace;

    static Handle TestMesh;
    static Handle PlanetMesh;
    static Handle DefiantMesh;
    static Handle AkiraMesh;
    static Handle MirandaMesh;
    static Handle IntrepidMesh;
    static Handle NorwayMesh;
    static Handle StarbaseMesh;
    static Handle RingMesh;
    static Handle DreadnaughtMesh;
    static Handle VenerexMesh;
    static Handle OberthMesh;
    static Handle LeviathanMesh;
    static Handle NovaMesh;

    static Handle StarbaseMaterial;
    static Handle StarMaterial;
    static Handle EarthSkyMaterial;
    static Handle DreadnaughtMaterial;
    static Handle DefiantMaterial;
    static Handle AkiraMaterial;
    static Handle MirandaMaterial;
    static Handle IntrepidMaterial;
    static Handle NorwayMaterial;
    static Handle VenerexMaterial;
    static Handle NovaMaterial;
    static Handle OberthMaterial;
    static Handle CrosshairMaterial;
    static Handle CrosshairArrowMaterial;
    static Handle StarFlareMaterial;
    static Handle GoldMaterial;

    static Handle CapsuleTunnelMesh;
    static Handle CapsuleRibbonMeshA;
    static Handle CapsuleRibbonMeshB;
    static Handle CapsuleA;
    static Handle CapsuleB;
    static Handle CapsuleC;
    static Handle CapsuleD;

    static Handle PBCSound;
    static Handle SniperSound;

    static void init();
};

#endif