#include "Client.h"
#include "Packet.h"
#include "Core.h"
#include "HUD.h"
#include "Helper.h"
#include "ResourceManifest.h"
#include "gui/Button.h"
#include "gui/Text.h"
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "gui/specifics/ServerLobbyShipSelectorWindow.h"
#include "gui/specifics/ServerHostingMapSelectorWindow.h"

#include "map/Map.h"
#include "map/Anchor.h"
#include "GameSkybox.h"
#include "Ship.h"
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Camera.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

using namespace std;
using namespace Engine;

const float  PHYSICS_PACKET_TIMER_LIMIT  = 0.05f;
const double DISTANCE_CHECK  = 1000000.0 * 1000000.0;
const double DISTANCE_CHECK2 = 100000.0  * 100000.0;

struct ShipSelectorButtonOnClick final {void operator()(Button* button) const {
    ServerLobbyShipSelectorWindow& window = *static_cast<ServerLobbyShipSelectorWindow*>(button->getUserPointer());
    for (auto& widget : window.getWindowFrame().content()) {
        widget->setColor(0.5f, 0.5f, 0.5f, 0.0f);
    }
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);

    auto& shipname = button->text();
    auto& handles = ResourceManifest::Ships.at(shipname);
    window.m_ChosenShipName = shipname;
    EntityWrapper& entity = *static_cast<EntityWrapper*>(window.getUserPointer());
    ComponentModel& model = *entity.entity().getComponent<ComponentModel>();
    model.setModelMesh(handles.get<0>(),0);
    model.setModelMaterial(handles.get<1>(), 0);
    model.show();

    auto& camera = const_cast<Camera&>(window.getShipDisplay().getCamera());
    camera.entity().getComponent<ComponentLogic2>()->call(-0.0001);
}};

Client::Client(Core& core, sf::TcpSocket* socket) : m_Core(core){
    m_TcpSocket = new Networking::SocketTCP(socket);
    internalInit();
}
Client::Client(Core& core, const ushort& port, const string& ipAddress) : m_Core(core) {
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
    internalInit();
}

Client::~Client() {
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
}
void Client::internalInit() {
    m_username = "";
    m_mapname = "";
    m_Validated = false;
    m_PingTime = 0.0;
    m_Timeout = 0.0;
    m_InitialConnectionThread = nullptr;
    m_IsCurrentlyConnecting = false;
}
void Client::changeConnectionDestination(const ushort& port, const string& ipAddress) {
    m_IsCurrentlyConnecting = false;
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
}
const sf::Socket::Status Client::connect(const ushort& timeout) {
    if (m_TcpSocket->isBlocking()) {
        auto conn = [&](Client* client, const ushort timeout) {
            client->m_IsCurrentlyConnecting = true;
            m_Core.m_HUD->setNormalText("Connecting...", static_cast<float>(timeout) + 2.2f);
            const auto status = client->m_TcpSocket->connect(timeout);
            if (status == sf::Socket::Status::Done) {
                m_Core.m_HUD->setGoodText("Connected!", 2);
                client->m_TcpSocket->setBlocking(false);
                m_Core.requestValidation(m_username);
            }else if (status == sf::Socket::Status::Error) {
                m_Core.m_HUD->setErrorText("Connection to the server failed",20);
            }else if (status == sf::Socket::Status::Disconnected) {
                m_Core.m_HUD->setErrorText("Disconnected from the server",20);
            }
            client->m_IsCurrentlyConnecting = false;
            return status;
        };
        //return conn(this, timeout);
        SAFE_DELETE_FUTURE(m_InitialConnectionThread);
        m_InitialConnectionThread = new std::future<sf::Socket::Status>(std::move(std::async(std::launch::async, conn, this, timeout)));
    }else{
        const auto status = m_TcpSocket->connect(timeout);
        if (status == sf::Socket::Status::Done) {
            m_Core.requestValidation(m_username);
        }
        return status;
    }
    return sf::Socket::Status::Error;
}
void Client::disconnect() {
    m_TcpSocket->disconnect();
}
const sf::Socket::Status Client::send(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    const auto status = m_TcpSocket->send(sf_packet);
    return status;
}
const sf::Socket::Status Client::send(sf::Packet& packet) {
    const auto status = m_TcpSocket->send(packet);
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size) {
    const auto status = m_TcpSocket->send(data, size);
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size, size_t& sent) {
    const auto status = m_TcpSocket->send(data, size, sent);
    return status;
}
const sf::Socket::Status Client::receive(sf::Packet& packet) {
    const auto status = m_TcpSocket->receive(packet);
    return status;
}
const sf::Socket::Status Client::receive(void* data, size_t size, size_t& received) {
    const auto status = m_TcpSocket->receive(data,size,received);
    return status;
}
const string& Client::username() const {
    return m_username;
}

void epriv::ClientInternalPublicInterface::update(Client* _client) {
    if (!_client) 
        return;
    auto& client = *_client;
    const auto& dt = Resources::dt();
    client.m_Timeout += dt;
    if (client.m_Core.gameState() == GameState::Game) {
        client.m_PingTime += dt;
        if (client.m_PingTime > PHYSICS_PACKET_TIMER_LIMIT) {
            //keep pinging the server, sending your ship physics info
            auto& map = *static_cast<Map*>(Resources::getCurrentScene());
            auto& playerShip = *map.getPlayer();

            Anchor* finalAnchor = map.getRootAnchor();
            const auto& list = map.getClosestAnchor();
            for (auto& closest : list) {
                finalAnchor = finalAnchor->getChildren().at(closest);
            }
            PacketPhysicsUpdate p(playerShip, map, finalAnchor, list);
            p.PacketType = PacketType::Client_To_Server_Ship_Physics_Update;
            client.send(p);

            auto playerPos = playerShip.getComponent<ComponentBody>()->position();
            auto nearestAnchorPos = finalAnchor->getPosition();
            double dist = static_cast<double>(glm::distance2(nearestAnchorPos, playerPos));
            
            if (dist > DISTANCE_CHECK) {
                for (auto& otherShips : map.getShips()) {
                    if (otherShips.first != playerShip.getComponent<ComponentName>()->name()) {
                        auto otherPos = otherShips.second->getComponent<ComponentBody>()->position();
                        double otherDist = static_cast<double>(glm::distance2(otherPos, playerPos));
                        if (otherDist < DISTANCE_CHECK2) {
                            glm::vec3 midpoint = Math::midpoint(otherPos, playerPos);

                            PacketMessage pOut;
                            pOut.PacketType = PacketType::Client_To_Server_Request_Anchor_Creation;
                            pOut.r = midpoint.x - nearestAnchorPos.x;
                            pOut.g = midpoint.y - nearestAnchorPos.y;
                            pOut.b = midpoint.z - nearestAnchorPos.z;
                            pOut.data = "";
                            pOut.data += std::to_string(list.size());
                            for (auto& closest : list) {
                                pOut.data += "," + closest;
                            }
                            //we want to create an anchor at r,g,b (the midpoint between two nearby ships), we send the nearest valid anchor as a reference
                            client.send(pOut);

                            break;
                        }
                    }
                }
            }
            client.m_PingTime = 0.0;
        }
    }
    client.onReceive();
}
void Client::onReceive() {
    sf::Packet sf_packet;
    const auto& status = receive(sf_packet);
    const auto& dt = Resources::dt();
    if (status == sf::Socket::Status::Done) {
        Packet* pp = Packet::getPacket(sf_packet);
        auto& p = *pp;
        if (pp && p.validate(sf_packet)) {
            // Data extracted successfully...
            m_Timeout = 0.0;
            HUD& hud = *m_Core.m_HUD;
            switch (p.PacketType) {
                case PacketType::Server_To_Client_Anchor_Creation_Deep_Space_Initial: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto& map = *static_cast<Map*>(Resources::getCurrentScene());
                    auto& anchorPos = map.getSpawnAnchor()->getPosition();
                    map.internalCreateDeepspaceAnchor(pI.r + anchorPos.x, pI.g + anchorPos.y, pI.b + anchorPos.z,pI.data);
                    //std::cout << "creating " << pI.data << " Initial" << std::endl;
                    break;
                }
                case PacketType::Server_To_Client_Anchor_Creation: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto& map = *static_cast<Map*>(Resources::getCurrentScene());
                    auto info = Helper::SeparateStringByCharacter(pI.data, ',');

                    const unsigned int& size = stoi(info[0]);
                    Anchor* closest = map.getRootAnchor();
                    for (unsigned int i = 1; i < 1 + size; ++i) {
                        closest = closest->getChildren().at(info[i]);
                    }
                    auto anchorPos = closest->getPosition();
                    const float x = pI.r + anchorPos.x;
                    const float y = pI.g + anchorPos.y;
                    const float z = pI.b + anchorPos.z;
                    map.internalCreateDeepspaceAnchor(x, y, z);
                    //std::cout << "creating deep space anchor" << std::endl;
                    break;
                }
                case PacketType::Server_To_Client_Ship_Physics_Update: {
                    if (m_Core.gameState() == GameState::Game) { //TODO: figure out a way for the server to only send phyiscs updates to clients in the map
                        PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(pp);
                        auto& map = *static_cast<Map*>(Resources::getCurrentScene());

                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        auto& playername = info[1];
                        auto& shipclass = info[0];

                        Ship* ship = nullptr;
                        if (!map.getShips().count(playername)) {
                            auto handles = ResourceManifest::Ships[shipclass];
                            auto spawnPosition = map.getSpawnAnchor()->getPosition();
                            auto x = Helper::GetRandomFloatFromTo(-400, 400);
                            auto y = Helper::GetRandomFloatFromTo(-400, 400);
                            auto z = Helper::GetRandomFloatFromTo(-400, 400);
                            auto randOffsetForSafety = glm::vec3(x, y, z);
                            ship = new Ship(handles.get<0>(), handles.get<1>(), shipclass, false, playername, spawnPosition + randOffsetForSafety, glm::vec3(1.0f), CollisionType::ConvexHull, &map);
                        }else{
                            ship = map.getShips().at(playername);
                        }
                        ship->updateFromPacket(pI, map, info);
                    }
                    break;
                }
                case PacketType::Server_To_Client_New_Client_Entered_Map: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    
                    auto info = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map

                    

                    Map& map = *static_cast<Map*>(Resources::getScene(info[1]));

                    auto spawn = map.getSpawnAnchor()->getPosition();

                    auto& handles = ResourceManifest::Ships.at(info[0]);
                    Ship* ship = new Ship(handles.get<0>(), handles.get<1>(), info[0], false, pI.name, glm::vec3(pI.r + spawn.x, pI.g + spawn.y, pI.b + spawn.z), glm::vec3(1.0f), CollisionType::ConvexHull, &map);

                    break;
                }
                case PacketType::Server_To_Client_Approve_Map_Entry: {
                    //ok the server let me in, let me tell the server i successfully went in
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);

                    auto info = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map

                    hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);
                    m_Core.enterMap(info[1], info[0], pI.name, pI.r, pI.g, pI.b);
                    hud.m_Next->setText("Next");
                    hud.m_GameState = GameState::Game;//ok, ive entered the map
                    Map& map = *static_cast<Map*>(Resources::getScene(info[1]));

                    auto dist = Helper::GetRandomFloatFromTo(200, 250);
                    auto sin = Helper::GetRandomFloatFromTo(0, 2 * 3.14159f);
                    auto cos = Helper::GetRandomFloatFromTo(0, 2 * 3.14159f);

                    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                    Math::rotate(orientation, sin, cos, 0.0f);


                    glm::mat4 modelMatrix = glm::mat4(1.0f);
                    auto position = glm::vec3(0, 0, -dist);
                    modelMatrix = glm::mat4_cast(orientation) * glm::translate(position);
                    auto& playerBody = *map.getPlayer()->getComponent<ComponentBody>();
                    auto spawn = map.getSpawnAnchor()->getPosition();
                    playerBody.setPosition(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);

                    Math::alignTo(orientation, playerBody.position()-spawn);
                    playerBody.setRotation(orientation);

                    PacketMessage pOut(pI);
                    pOut.PacketType = PacketType::Client_To_Server_Successfully_Entered_Map;
                    pOut.r = modelMatrix[3][0] - spawn.x;
                    pOut.g = modelMatrix[3][1] - spawn.y;
                    pOut.b = modelMatrix[3][2] - spawn.z;
                    send(pOut);
                    break;
                }
                case PacketType::Server_To_Client_Reject_Map_Entry: {
                    break;
                }
                case PacketType::Server_To_Client_Map_Data: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto& mapname = pI.name;
                    m_mapname = mapname;
                    Map* map = static_cast<Map*>(Resources::getScene(mapname));
                    if (!map) {
                        map = new Map(mapname, ResourceManifest::BasePath + "data/Systems/" + mapname + ".txt");
                    }
                    auto& menuScene = *const_cast<Scene*>(Resources::getScene("Menu"));
                    auto* menuSkybox = menuScene.skybox();
                    SAFE_DELETE(menuSkybox);
                    GameSkybox* newMenuSkybox = new GameSkybox(map->skyboxFile(), 0);
                    menuScene.setSkybox(newMenuSkybox);
                    menuScene.setGlobalIllumination(map->getGlobalIllumination());

                    hud.m_ServerLobbyShipSelectorWindow->clear();
                    auto ships = map->allowedShips();
                    for (auto& ship : ships) {
                        Button* shipbutton = new Button(*hud.m_Font, 0, 0, 100, 40);
                        shipbutton->setText(ship);
                        shipbutton->setColor(0.5f, 0.5f, 0.5f, 0.0f);
                        shipbutton->setTextColor(1, 1, 1, 1);
                        shipbutton->setAlignment(Alignment::TopLeft);
                        shipbutton->setWidth(600);
                        shipbutton->setTextAlignment(TextAlignment::Left);
                        shipbutton->setUserPointer(hud.m_ServerLobbyShipSelectorWindow);
                        shipbutton->setOnClickFunctor(ShipSelectorButtonOnClick());
                        hud.m_ServerLobbyShipSelectorWindow->addContent(shipbutton);
                    }
                    hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(true);
                    break;
                }
                case PacketType::Server_To_Client_Chat_Message: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto message = pI.name + ": " + pI.data;

                    Text* text = new Text(0, 0, *hud.m_Font, message);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text);
                    break;
                
                }
                case PacketType::Server_To_Client_Client_Joined_Server: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto message = pI.name + ": Has joined the server";


                    Text* text = new Text(0, 0, *hud.m_Font, pI.name);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyConnectedPlayersWindow->addContent(text);

                    Text* text1 = new Text(0, 0, *hud.m_Font, message);
                    text1->setColor(0.8f, 1, 0.2f, 1);
                    text1->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text1);
                    break;
                }
                case PacketType::Server_To_Client_Client_Left_Server:{
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto message = pI.name + ": Has left the server";

                    Text* text = new Text(0, 0, *hud.m_Font, pI.name);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyConnectedPlayersWindow->removeContent(pI.name);

                    Text* text1 = new Text(0, 0, *hud.m_Font, message);
                    text1->setColor(0.907f, 0.341f, 0.341f, 1.0f);
                    text1->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text1);
                    break;
                }
                case PacketType::Server_To_Client_Accept_Connection: {
                    m_Validated = true;
                    if (m_Core.m_GameState != GameState::Host_Server_Lobby_And_Ship && m_Core.m_GameState == GameState::Host_Server_Port_And_Name_And_Map) {
                        m_Core.m_GameState = GameState::Host_Server_Lobby_And_Ship;
                        hud.m_Next->setText("Enter Game");
                    }else if (m_Core.m_GameState != GameState::Join_Server_Server_Lobby && m_Core.m_GameState == GameState::Join_Server_Port_And_Name_And_IP) {
                        m_Core.m_GameState = GameState::Join_Server_Server_Lobby;
                        hud.m_Next->setText("Enter Game");
                    }
                    hud.m_ServerLobbyConnectedPlayersWindow->clear();
                    hud.m_ServerLobbyChatWindow->clear();
                    stringstream ss(pp->data);
                    vector<string> result;
                    while (ss.good()){
                        string substr;
                        getline(ss, substr, ',');
                        result.push_back(substr);
                    }
                    if (result.size() == 0 && !pp->data.empty()) {
                        result.push_back(pp->data);
                    }
                    //result is a vector of connected players
                    for (auto& _name : result) {
                        if (!_name.empty()) {
                            Text* text = new Text(0, 0, *hud.m_Font, _name);
                            text->setColor(1, 1, 0, 1);
                            text->setTextScale(0.62f, 0.62f);
                            hud.m_ServerLobbyConnectedPlayersWindow->addContent(text);
                        }
                    }
                    break;
                }case PacketType::Server_To_Client_Reject_Connection: {
                    m_Validated = false;
                    hud.setErrorText("Someone has already chosen that name");
                    break;
                }case PacketType::Server_Shutdown: {
                    m_Validated = false;
                    m_Core.shutdownClient(true);
                    hud.setErrorText("Disconnected from the server",600);
                    m_Core.m_GameState = GameState::Main_Menu;
                    hud.m_Next->setText("Next");

                    hud.m_ServerLobbyChatWindow->clear();
                    hud.m_ServerLobbyConnectedPlayersWindow->clear();
                    break;
                }default: {
                    break;
                }
            }
        }
        SAFE_DELETE(pp);
    }
}