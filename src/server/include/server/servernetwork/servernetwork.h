#ifndef SERVERNETWORK_H
#define SERVERNETWORK_H



#ifdef __unix__  
    #define OS_Windows 0
#elif defined(_WIN32) || defined(WIN32)    
    #define OS_Windows 1

#endif

// std
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <memory.h>
#include <cstring>
#include <map>
#include <unordered_map>


// ext
#include <SFML/Network.hpp>
#include <yaml-cpp/yaml.h>

// from this project
#include <server/commands/executor.h>
#include <server/commands/command.h>
#include <server/cryptography/cryptography.h>


#define MAX_RAW_DATA 4096 //Max bytes supported on Raw Data mode

#define __logl(x) std::cout << "[INFO] " << x << std::endl
#define __log(x) std::cout << x
#define warn(x) std::cout << "[WARN] " << x << std::endl
#define error(x) std::cout << "[ERROR] " << x << std::endl
#define CommandLambda [&](sf::TcpSocket* sock,size_t iterator, std::string args[])
class ServerObject;
class ChatHandler;

using namespace simp;
enum class DisconnectReason {
    DisconnectLeave = 0,
    DisconnectKick = 1,
    DisconnectUnnamed = 2,

};
enum AuthStatus {
        KeyReceived = 1,
        Done = 2,
        Failed = 3,
        Undone = 4,
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

struct UserData {
    std::string username;
    std::string password;
};

class ServerNetwork {
    sf::TcpListener listener;
    std::vector<sf::TcpSocket*> client_array;
    std::vector<std::string> clientid_array;
    std::vector<bool> client_op_array;
    std::vector<sf::Clock*> client_message_interval;
    std::vector<AuthStatus> client_authenticated_array;

    std::vector<UserData> registered_users;

    unsigned short listen_port;

    std::vector<ServerObject*> objs;

    std::vector<std::string> send_queue;
    sf::TcpSocket* lastQueuer;

    std::string chatSend_str;

    static ServerNetwork* m_instance;

    Executor* cmd_executor;
    Cryptography* crypt;

    void openUserdata();

    ServerNetwork();
    ServerNetwork(ServerNetwork const&) = delete;
    ServerNetwork& operator=(ServerNetwork const&) = delete;
    
    std::string SERVER_KEY{"2B7E151628AED2A6ABF7158809CF4F3C2B7E151628AED2A6ABF7158809CF4F3C"};

public:
    static ServerNetwork* Get();
    

    void init(unsigned short);
    // Connect & disconnect
    void connectClients(std::vector<sf::TcpSocket*>*);
    void disconnectClient(sf::TcpSocket*, size_t, DisconnectReason);

    std::string convertToString(char*,int);

    
    void sendString(std::string, sf::TcpSocket*);
    void broadcastString(std::string, sf::IpAddress, unsigned short);
    // Receive & send
    void receive(sf::TcpSocket*, size_t);
    bool broadcast(const char*, sf::IpAddress, unsigned short);

    bool send(const char*, size_t counter, sf::TcpSocket*);
    bool send_unencrypted(const char*, size_t counter, sf::TcpSocket*);

    bool check(char*);
    bool isOp(size_t iter);

    void updateObjs();
    
    bool nick(std::string, sf::TcpSocket* sock, size_t );
    void handleCommand(std::string, sf::TcpSocket*, size_t);

    bool handleSend(std::string& ,std::stringstream&,sf::TcpSocket*, size_t);
    bool handleNick(std::string ,sf::TcpSocket*, size_t);
    void handleRequestedConsole(sf::TcpSocket*, size_t);
    // Core

    void sendQueuedMessage(std::vector<std::string>&,sf::TcpSocket* client);
    void manage();
    void run();

    void end() {exit(EXIT_SUCCESS);}
    ChatHandler* handler;

};
void openDLL();


#endif
