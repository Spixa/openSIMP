#ifndef SERVERNETWORK_H
#define SERVERNETWORK_H



#ifdef __unix__  
    #define OS_Windows 0
#elif defined(_WIN32) || defined(WIN32)    
    #define OS_Windows 1

#endif


#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <SFML/Network.hpp>
#include <memory.h>
#include <cstring>
#include <map>
#include <unordered_map>

#include "ChatHandler.h"
#include "ServerObject.h"
#define MAX_RAW_DATA 256 //Max bytes supported on Raw Data mode

#define logl(x) std::cout << x << std::endl
#define log(x) std::cout << x
#define PREFIX "[Server]"

enum class DisconnectReason {
    DisconnectLeave = 0,
    DisconnectKick = 1,
    DisconnectUnnamed = 2,

};

enum class MessageType {
    ChatMessageType = 0,
    JoinMessageType = 1,
    LeaveMessageType = 2,
    IdentifyMessageType = 3,
    RequestConsole = 4,
    CommandResponse = 5,
    DirectMessage = 6,
};

class ServerNetwork {
    sf::TcpListener listener;
    std::vector<sf::TcpSocket*> client_array;
    std::vector<std::string> clientid_array;
    std::vector<bool> client_op_array;
    std::vector<sf::Clock*> client_message_interval;
    unsigned short listen_port;

    std::vector<ServerObject*> objs;

    std::vector<std::string> send_queue;
    sf::TcpSocket* lastQueuer;

    Property* chatSend;
    std::string chatSend_str;
public:
    ServerNetwork(unsigned short);
    void init();
    // Connect & disconnect
    void connectClients(std::vector<sf::TcpSocket*>*);
    void disconnectClient(sf::TcpSocket*, size_t, DisconnectReason);

    std::string convertToString(char*,int);

    // Receive & send
    void receive(sf::TcpSocket*, size_t);
    bool broadcast(const char*, sf::IpAddress, unsigned short);

    bool send(const char*, size_t counter, sf::TcpSocket*);
    bool check(char*);
    bool isOp(size_t iter);

    void updateObjs();
    
    void handleCommand(char*, sf::TcpSocket*, size_t);

    bool handleSend(char*,std::stringstream&,sf::TcpSocket*, size_t);
    bool handleNick(char*,sf::TcpSocket*, size_t);
    void handleRequestedConsole(sf::TcpSocket*, size_t);
    // Core

    void sendQueuedMessage(std::vector<std::string>&,sf::TcpSocket* client);
    void manage();
    void run();
    ChatHandler* handler;
};

#endif
