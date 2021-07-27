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
enum class MessageType {
    ChatMessageType = 0,
    JoinMessageType = 1,
    LeaveMessageType = 2,
    IdentifMessageType = 3
};

class ClientNetwork {
    sf::TcpSocket socket;
    char buffer[256];
    std::size_t received;
    bool isConnected = false;
    MessageType mode = MessageType::ChatMessageType;
public:
    ClientNetwork();
    void connect(const char*, unsigned short);
    void receive(sf::TcpSocket*);
    void send(std::string);
    void run();
};

#endif