#include "Core.h"

#include "HUD.h"
#include "Server.h"
#include "Client.h"
#include "Ship.h"
#include "SolarSystem.h"
#include "Packet.h"
#include "Planet.h"
#include "GameCamera.h"
#include "GameSkybox.h"
#include "ResourceManifest.h"

#include <core/engine/Engine.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/lights/SunLight.h>
#include <ecs/Components.h>

using namespace std;
using namespace Engine;

Core::Core() {
    m_HUD               = nullptr;
    m_Server            = nullptr;
    m_Client            = nullptr;
    m_Initalized        = false;
    m_GameState         = GameState::Main_Menu;

    ResourceManifest::init();
    //const std::string& iconPath = ResourceManifest::BasePath + "data/Textures/icon.png";
    //Resources::getWindow().setIcon(iconPath);
}
Core::~Core() {
    if (m_Client) {
        Packet p;
        p.PacketType = PacketType::Client_To_Server_Request_Disconnection;
        const auto status = m_Client->send(p);
    }
    SAFE_DELETE(m_Client);
    SAFE_DELETE(m_ChosenShip);
    SAFE_DELETE(m_Server);
    SAFE_DELETE(m_HUD);
}
Server* Core::getServer() {
    return m_Server;
}
Client* Core::getClient() {
    return m_Client;
}

void Core::startServer(const unsigned short& port, const std::string& mapname) {
    if (!m_Server) {
        m_Server = new Server(*this, port, false);
        m_Server->startup(mapname);
    }
}
void Core::shutdownServer() {
    if (m_Server) {
        m_Server->shutdown();
        SAFE_DELETE(m_Server);
    }
}
void Core::startClient(const unsigned short& port, const string& name, const string& ip) {
    if (!m_Client) {
        m_Client = new Client(*this, port, ip);
        m_Client->m_username = name;
    }
    if (!m_Client->m_IsCurrentlyConnecting) {
        m_Client->changeConnectionDestination(port, ip);
        m_Client->m_username = name;
        auto status = m_Client->connect(10);
    }
}
void Core::shutdownClient(const bool& serverShutdownFirst) {
    if (m_Client) {
        if (!serverShutdownFirst) {
            Packet p;
            p.PacketType = PacketType::Client_To_Server_Request_Disconnection;
            const auto status = m_Client->send(p);
        }
        m_Client->m_TcpSocket->setBlocking(false);
        m_Client->m_IsCurrentlyConnecting = false;
        m_Client->disconnect();
    }
}
void Core::requestValidation(const string& name) {
    Packet p;
    p.PacketType = PacketType::Client_To_Server_Request_Connection;
    p.data = name;
    const auto& status = m_Client->send(p);
    if (status == sf::Socket::Status::Done) {
        cout << "Client: requesting validation connection to the server..." << endl;
    }else{
        m_HUD->setErrorText("Connection timed out");
    }
}
void Core::enterMap(const string& mapFile, const string& playership) {
    auto& window = Resources::getWindow();
    Resources::setCurrentScene(mapFile);

    SolarSystem& map = *static_cast<SolarSystem*>(Resources::getScene(mapFile));
    auto& model = *map.getPlayer()->entity().getComponent<ComponentModel>();
    auto& handles = ResourceManifest::Ships.at(playership);
    map.getPlayer()->setModel(handles.get<0>());
    model.setModelMaterial(handles.get<1>(), 0);

    //map->setBackgroundColor(1, 0, 0, 1.0f);
    //Renderer::fog::enable();

    window.keepMouseInWindow(true);
    window.setMouseCursorVisible(false);
}
void Core::onResize(const uint& width, const uint& height) {
    m_HUD->onResize(width, height);
}
void Core::init() {
    if (m_Initalized) return;

    auto& window = Resources::getWindow();
    window.setKeyRepeatEnabled(false);
    window.setFramerateLimit(60);

    Scene* s = new Scene("Menu");
    Resources::setCurrentScene(s);


    GameSkybox* box = new GameSkybox(ResourceManifest::BasePath + "data/Textures/Skyboxes/SolarSystem/Skybox.dds", 0, s);
    s->setSkybox(box);

    Camera* main_camera = new Camera(60,Resources::getWindowSize().x / static_cast<float>(Resources::getWindowSize().y), 0.1f, 15000.0f, s);
    GameCamera* ship_camera = new GameCamera(0.1f, 50.0f, s);
    s->setActiveCamera(*main_camera);
    s->getMainViewport().setSkyboxVisible(false);

    SunLight* light = new SunLight(glm::vec3(0.0f), LightType::Sun, s);
    light->setColor(1.55f, 1.55f, 1.3f);
    light->setPosition(0, 100, -1000);
    auto e1 = s->createEntity();
    auto model1 = e1.addComponent<ComponentModel>(ResourceManifest::PlanetMesh, ResourceManifest::StarMaterial);



    m_ChosenShip = new EntityWrapper(*s);

    auto e = m_ChosenShip->entity();
    ship_camera->setTarget(e);
    auto& body  = *e.addComponent<ComponentBody>();
    body.setPosition(0, 0, 8500);
    auto& model = *e.addComponent<ComponentModel>(ResourceManifest::DefiantMesh, ResourceManifest::DefiantMaterial);
    model.hide();

    m_HUD        = new HUD(*s,*ship_camera, m_GameState, *this);
    m_Initalized = true;
    m_HUD->go_to_main_menu();
}
void Core::update(const double& dt) {
    if (Engine::isKeyDown(KeyboardKey::Escape)) {
        Engine::stop();
    }
    /*
    if (Engine::isKeyDownOnce(KeyboardKey::Space)) {
        //Engine::pause(!Engine::paused());
        m_GameState = GameState::Game;
        if (!m_Client->connected()) {
            m_Client->connect();
        }else{
            SolarSystem& scene = *static_cast<SolarSystem*>(Resources::getCurrentScene());
            PacketPhysicsUpdate p(*scene.getPlayer());
            p.PacketType = PacketType::Client_To_Server_Ship_Physics_Update;
            m_Client->send(p);
        }
    }
    */
    m_HUD->update(dt);
    if(m_Client) epriv::ClientInternalPublicInterface::update(m_Client);
    if(m_Server) m_Server->update(m_Server);
}
void Core::render() {
    m_HUD->render();
}

const GameState::State& Core::gameState() const {
    return m_GameState;
}