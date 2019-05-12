#include "ResourceManifest.h"
#include "core/engine/Engine_ThreadManager.h"
#include "core/engine/resources/Engine_Resources.h"
#include "core/Material.h"
#include "core/engine/textures/Texture.h"

using namespace Engine;

Handle ResourceManifest::skyFromSpace;
Handle ResourceManifest::skyFromAtmosphere;
Handle ResourceManifest::groundFromSpace;

Handle ResourceManifest::TestMesh;
Handle ResourceManifest::PlanetMesh;
Handle ResourceManifest::DefiantMesh;
Handle ResourceManifest::AkiraMesh;
Handle ResourceManifest::MirandaMesh;
Handle ResourceManifest::IntrepidMesh;
Handle ResourceManifest::NorwayMesh;
Handle ResourceManifest::StarbaseMesh;
Handle ResourceManifest::RingMesh;
Handle ResourceManifest::DreadnaughtMesh;
Handle ResourceManifest::VenerexMesh;
Handle ResourceManifest::OberthMesh;
Handle ResourceManifest::LeviathanMesh;
Handle ResourceManifest::NovaMesh;

Handle ResourceManifest::StarbaseMaterial;
Handle ResourceManifest::StarMaterial;
Handle ResourceManifest::EarthSkyMaterial;
Handle ResourceManifest::DreadnaughtMaterial;
Handle ResourceManifest::DefiantMaterial;
Handle ResourceManifest::AkiraMaterial;
Handle ResourceManifest::MirandaMaterial;
Handle ResourceManifest::IntrepidMaterial;
Handle ResourceManifest::NorwayMaterial;
Handle ResourceManifest::VenerexMaterial;
Handle ResourceManifest::OberthMaterial;
Handle ResourceManifest::CrosshairMaterial;
Handle ResourceManifest::CrosshairArrowMaterial;
Handle ResourceManifest::StarFlareMaterial;
Handle ResourceManifest::GoldMaterial;
Handle ResourceManifest::NovaMaterial;

Handle ResourceManifest::CapsuleTunnelMesh;
Handle ResourceManifest::CapsuleRibbonMeshA;
Handle ResourceManifest::CapsuleRibbonMeshB;
Handle ResourceManifest::CapsuleA;
Handle ResourceManifest::CapsuleB;
Handle ResourceManifest::CapsuleC;
Handle ResourceManifest::CapsuleD;

Handle ResourceManifest::PBCSound;
Handle ResourceManifest::SniperSound;

void ResourceManifest::init(){
    Handle skyFromSpaceVert = Resources::addShader("data/Shaders/AS_skyFromSpace_vert.glsl",ShaderType::Vertex);
    Handle skyFromSpaceFrag = Resources::addShader("data/Shaders/AS_skyFromSpace_frag.glsl",ShaderType::Fragment);
    skyFromSpace = Resources::addShaderProgram("AS_SkyFromSpace",skyFromSpaceVert,skyFromSpaceFrag);

    Handle skyFromAtVert = Resources::addShader("data/Shaders/AS_skyFromAtmosphere_vert.glsl",ShaderType::Vertex);
    Handle skyFromAtFrag = Resources::addShader("data/Shaders/AS_skyFromAtmosphere_frag.glsl",ShaderType::Fragment);
    skyFromAtmosphere = Resources::addShaderProgram("AS_SkyFromAtmosphere",skyFromAtVert,skyFromAtFrag);

    Handle groundFromSpaceVert = Resources::addShader("data/Shaders/AS_groundFromSpace_vert.glsl",ShaderType::Vertex);
    Handle groundFromSpaceFrag = Resources::addShader("data/Shaders/AS_groundFromSpace_frag.glsl",ShaderType::Fragment);
    groundFromSpace = Resources::addShaderProgram("AS_GroundFromSpace",groundFromSpaceVert,groundFromSpaceFrag);
 
    TestMesh = Resources::loadMeshAsync("data/Models/1911.fbx",0.0f).at(0);

    PlanetMesh = Resources::loadMeshAsync("data/Models/planet.objcc").at(0);
    DefiantMesh = Resources::loadMeshAsync("data/Models/defiant.objcc").at(0); //220 metres long (0.22 km)
    StarbaseMesh = Resources::loadMeshAsync("data/Models/starbase.objcc").at(0); //6950 meters in height (6.95 km)
    RingMesh = Resources::loadMeshAsync("data/Models/ring.objcc").at(0);
    DreadnaughtMesh = Resources::loadMeshAsync("data/Models/dreadnaught.objcc").at(0);
    LeviathanMesh = Resources::loadMeshAsync("data/Models/leviathan.objcc").at(0);
    NovaMesh = Resources::loadMeshAsync("data/Models/nova.objcc").at(0);
    
    AkiraMesh = Resources::loadMeshAsync("data/Models/akira.objcc").at(0);
    MirandaMesh = Resources::loadMeshAsync("data/Models/miranda.objcc").at(0);
    IntrepidMesh = Resources::loadMeshAsync("data/Models/intrepid.objcc").at(0);
    NorwayMesh = Resources::loadMeshAsync("data/Models/norway.objcc").at(0);
    VenerexMesh = Resources::loadMeshAsync("data/Models/venerex.objcc").at(0);
    OberthMesh = Resources::loadMeshAsync("data/Models/oberth.objcc").at(0);

    CapsuleTunnelMesh = Resources::loadMeshAsync("data/Models/capsuleTunnel.objcc").at(0);
    CapsuleRibbonMeshA = Resources::loadMeshAsync("data/Models/ribbon1.objcc").at(0);
    CapsuleRibbonMeshB = Resources::loadMeshAsync("data/Models/ribbon2.objcc").at(0);

    Engine::epriv::threading::waitForAll();


    StarbaseMaterial = Resources::addMaterial("Starbase","data/Textures/starbase.png","data/Textures/starbase_Normal.png","data/Textures/starbase_Glow.png");
    StarMaterial = Resources::addMaterial("Star","data/Textures/Planets/Sun.dds","","","");
    ((Material*)StarMaterial.get())->setShadeless(true);
    ((Material*)StarMaterial.get())->setGlow(0.21f);

    EarthSkyMaterial = Resources::addMaterial("EarthSky","data/Textures/Planets/Earth.dds","","","");

    DreadnaughtMaterial = Resources::addMaterial("Dreadnaught","data/Textures/dreadnaught.dds","data/Textures/dreadnaught_Normal.dds","data/Textures/dreadnaught_Glow.dds");
    DefiantMaterial = Resources::addMaterial("Defiant","data/Textures/defiant.dds","data/Textures/defiant_Normal.dds","data/Textures/defiant_Glow.dds");
    AkiraMaterial = Resources::addMaterial("Akira","data/Textures/akira.dds","data/Textures/akira_Normal.png","data/Textures/akira_Glow.png");
    MirandaMaterial = Resources::addMaterial("Miranda","data/Textures/miranda.dds","data/Textures/miranda_Normal.png","data/Textures/miranda_Glow.png");
    IntrepidMaterial = Resources::addMaterial("Intrepid","data/Textures/intrepid.dds","data/Textures/intrepid_Normal.png","data/Textures/intrepid_Glow.png");
    NorwayMaterial = Resources::addMaterial("Norway","data/Textures/norway.dds","data/Textures/norway_Normal.png","data/Textures/norway_Glow.png");
    VenerexMaterial = Resources::addMaterial("Venerex","data/Textures/venerex.dds","data/Textures/venerex_Normal.png","data/Textures/venerex_Glow.png");
    OberthMaterial = Resources::addMaterial("Oberth","data/Textures/oberth.dds","data/Textures/oberth_Normal.png","data/Textures/oberth_Glow.png");
    NovaMaterial = Resources::addMaterial("Nova", "data/Textures/nova.dds", "data/Textures/nova_Normal.dds", "data/Textures/nova_Glow.dds");

    CrosshairMaterial = Resources::addMaterial("Crosshair","data/Textures/HUD/Crosshair.dds","","","");
    CrosshairArrowMaterial = Resources::addMaterial("CrosshairArrow","data/Textures/HUD/CrosshairArrow.dds","","","");

    auto& crosshairDiffuse = *(((Material*)CrosshairMaterial.get())->getComponent(MaterialComponentType::Diffuse)->texture());
    auto& crosshairArrowDiffuse = *(((Material*)CrosshairArrowMaterial.get())->getComponent(MaterialComponentType::Diffuse)->texture());

    crosshairDiffuse.setWrapping(TextureWrap::ClampToEdge);
    crosshairDiffuse.setFilter(TextureFilter::Nearest);

    StarFlareMaterial = Resources::addMaterial("SunFlare","data/Textures/Skyboxes/StarFlare.dds");
    Resources::getMaterial(StarFlareMaterial)->setShadeless(true);

    GoldMaterial = Resources::addMaterial("Gold","");
    ((Material*)GoldMaterial.get())->setMaterialPhysics(MaterialPhysics::Gold);

    PBCSound = Resources::addSoundData("data/Sounds/Effects/pbc.ogg","pbc");
    SniperSound = Resources::addSoundData("data/Sounds/Effects/snipercannon.ogg","sniper");
    
    CapsuleA = Resources::addMaterial("Capsule_A","data/Textures/Effects/capsule_a.dds");
    ((Material*)CapsuleA.get())->setShadeless(true);
    ((Material*)CapsuleA.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
    CapsuleB = Resources::addMaterial("Capsule_B","data/Textures/Effects/capsule_b.dds");
    ((Material*)CapsuleB.get())->setShadeless(true);
    ((Material*)CapsuleB.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
    CapsuleC = Resources::addMaterial("Capsule_C","data/Textures/Effects/capsule_c.dds");
    ((Material*)CapsuleC.get())->setShadeless(true);
    ((Material*)CapsuleC.get())->setGlow(1.0f);
    ((Material*)CapsuleC.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
    CapsuleD = Resources::addMaterial("Capsule_D","data/Textures/Effects/capsule_d.dds");
    ((Material*)CapsuleD.get())->setShadeless(true);  
    ((Material*)CapsuleD.get())->getComponent(MaterialComponentType::Diffuse)->texture()->setAnisotropicFiltering(2.0f);
}