#pragma once
#ifndef ENGINE_NETWORKING_SOCKET_UDP_H
#define ENGINE_NETWORKING_SOCKET_UDP_H

#include <core/engine/networking/SocketInterface.h>
#include <core/engine/networking/Packet.h>
#include <string>
#include <queue>

namespace Engine::priv {
    class SocketManager;
}
namespace Engine::Networking {
    class SocketUDP : public ISocket, public Engine::NonCopyable {
        friend class Engine::priv::SocketManager;
        private:
            struct UDPPacketInfo final {
                sf::Packet      packet;
                unsigned short  port;
                sf::IpAddress   ip;
            };

            sf::UdpSocket               m_SocketUDP;
            unsigned short              m_Port      = 0;
            std::string                 m_IP        = "";
            std::queue<UDPPacketInfo>   m_PartialPackets;

            SocketUDP() = delete;

            sf::IpAddress internal_get_ip(const std::string& ipString) const;

            sf::Socket::Status internal_send_partial_packets_loop();
            sf::Socket::Status internal_send_packet(UDPPacketInfo& packetInfoUDP);

            void update(const float dt) override;
        public:
            SocketUDP(const unsigned short port, const std::string& ip = "");
            ~SocketUDP();

            void                 setBlocking(bool blocking) override;
            bool                 isBlocking() const override;
            bool                 isBound() const;
            sf::UdpSocket&       socket() override;
            unsigned short       localPort() const override;

            sf::Socket::Status   bind(const std::string& ip = "");
            void                 unbind();
            void                 changePort(const unsigned short newPort);
            
            sf::Socket::Status   send(Engine::Networking::Packet& packet, const std::string& ip = "");
            sf::Socket::Status   send(sf::Packet& packet, const std::string& ip = "");
            sf::Socket::Status   send(const void* data, size_t size, const std::string& ip = "");
            sf::Socket::Status   receive(sf::Packet& packet);
            sf::Socket::Status   receive(void* data, size_t size, size_t& received);

            sf::Socket::Status   send(const unsigned short port, Engine::Networking::Packet& packet, const std::string& ip = "");
            sf::Socket::Status   send(const unsigned short port, sf::Packet& packet, const std::string& ip = "");
            sf::Socket::Status   send(const unsigned short port, const void* data, size_t size, const std::string& ip = "");
    };
};

#endif