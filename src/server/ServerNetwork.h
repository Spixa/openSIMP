#ifndef SERVERNETWORK_H
#define SERVERNETWORK_H

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string.h>
#include <SFML/Network.hpp>

#define MAX_RAW_DATA 256 //Max bytes supported on Raw Data mode

#define logl(x) std::cout << x << std::endl
#define log(x) std::cout << x
#define PREFIX "[Server]"

class ServerNetwork {
    sf::TcpListener listener;
    std::vector<sf::TcpSocket*> client_array;
    std::vector<std::string> clientid_array;

    unsigned short listen_port;
    bool rawMode = false;
public:
    ServerNetwork(unsigned short, bool);
    void connectClients(std::vector<sf::TcpSocket*>*);
    void disconnectClient(sf::TcpSocket*, size_t);

    void receivePacket(sf::TcpSocket*, size_t);
    void receiveRawData(sf::TcpSocket*, size_t);

    void sendPacket(sf::Packet&, sf::IpAddress, unsigned short);
    void broadcastPacket(sf::Packet&, sf::IpAddress, unsigned short);
    void broadcastRawData(const char*, sf::IpAddress, unsigned short);

    void managePackets();
    void run();
};

#endif