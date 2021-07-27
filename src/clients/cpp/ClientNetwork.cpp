#include "ClientNetwork.h"
#include "PacketType.h"
#include "Utils.hpp"

ClientNetwork::ClientNetwork() {
    logl("Chat Client Started");
}

void ClientNetwork::connect(const char* address, unsigned short port) {
    if (socket.connect(address, port) != sf::Socket::Done) {
        logl("Could not connect to the server\n");
    }
    else {
        isConnected = true;
        logl("Connected to the server\n");
    }
}


void ClientNetwork::receive(sf::TcpSocket* socket) {
    while (true) {
        if (socket->receive(buffer,sizeof(buffer),received) == sf::Socket::Done) {
            
            logl(received << " bytes were received");
            log("\tReceived: ");

            size_t counter=1;
            for (size_t r = 0; r < received; r++)
                if (buffer[r] == '\x01') { 
                    log(" -- ");
                    counter++;
                }
                else log(buffer[r]);
            logl('\n' << counter << " receivees in total.");
        }
        std::this_thread::sleep_for((std::chrono::milliseconds)100);
    }
}

void ClientNetwork::send(std::string sent) {
    if (sent.length() > 0 && socket.send(sent.c_str(),sent.length() + 1) != sf::Socket::Done) {
        logl("Could not send data");
    }

}

void ClientNetwork::run() {
    std::thread reception_thred(&ClientNetwork::receive, this, &socket);

    while (true) {
        if (isConnected) {
            std::string user_input;
            std::string args[512];
            std::getline(std::cin, user_input);

            send(user_input);
        }
    }
}


