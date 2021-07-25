#ifndef CLIENTNETWORK_H
#define CLIENTNETWORK_H

#include <iostream>
#include <thread>
#include <chrono>
#include <SFML/Network.hpp>
#include <memory.h>
#include <string>
#include <cstring>
#define logl(x) std::cout << x << std::endl
#define log(x) std::cout << x

class ClientNetwork {
    sf::TcpSocket socket;
    sf::Packet last_packet;

    char buffer[256];
    std::size_t received;

    bool isConnected = false;
public:
    ClientNetwork();
    void connect(const char*, unsigned short);
    void receivePackets(sf::TcpSocket*);
    void receive(sf::TcpSocket*);
    void send(char*);
    void sendPacket(sf::Packet&);
    void run();
};

#endif