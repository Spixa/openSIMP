#ifndef CLIENTNETWORK_H
#define CLIENTNETWORK_H

#include <iostream>
#include <thread>
#include <chrono>
#include <SFML/Network.hpp>
#include <memory.h>
#include <string>
#include <cstring>
#include "cryptography.h"

#define logl(x) std::cout << x << std::endl
#define log(x) std::cout << x
enum class MessageType {
    ChatMessageType = 0,
    JoinMessageType = 1,
    LeaveMessageType = 2,
    IdentifMessageType = 3,
    CommandResponseType = 5,
};

class ClientNetwork {
    sf::TcpSocket socket;
    char buffer[4096]{};
    std::size_t received{};
    bool isConnected = false;
    MessageType mode = MessageType::ChatMessageType;
    Cryptography* crypt;

    std::string aes_key{};
    bool hasHandshook{false};
    Botan::BigInt server_pubkey;


    enum class HandshakeStatus {
        HasntBegun,
        ReceivedPubKey,
        SentCredientials,
        SentPubKey, 
        ReceivedAESKey
    } status = HandshakeStatus::HasntBegun;
public:
    ClientNetwork();
    void connect(const char*, unsigned short);
    void receive(sf::TcpSocket*);
    void send(std::string const&);
    void send_raw(std::string const&);
    void run();
    void handshake(const std::string&, const std::string&);
};

#endif