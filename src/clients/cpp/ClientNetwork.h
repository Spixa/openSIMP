#ifndef CLIENTNETWORK_H
#define CLIENTNETWORK_H

#include <iostream>
#include <thread>
#include <chrono>
#include <SFML/Network.hpp>

#define logl(x) std::cout << x << std::endl
#define log(x) std::cout << x

class ClientNetwork {
    sf::TcpSocket socket;
    sf::Packet last_packet;

    bool isConnected = false;
public:
    ClientNetwork();
    void connect(const char*, unsigned short);
    void receivePackets(sf::TcpSocket*);
    void sendPacket(sf::Packet&);
    void run();
};

#endif