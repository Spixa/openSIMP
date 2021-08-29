#include "ClientNetwork.h"
#include <cstdlib>
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
        sf::Socket::Status status = socket->receive(buffer,sizeof(buffer),received);
        if (status == sf::Socket::Done) {
            
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
        } else if (status == sf::Socket::Disconnected) {
            log("You were disconnected from the server.");
            ::exit(0);
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

    log("Username: ");
    std::string name_input;
    std::getline(std::cin, name_input);

    send("3" + name_input);

    while (true) {
        if (isConnected) {
            std::string user_input;
            std::getline(std::cin, user_input);

            char* tosend = const_cast<char*>(user_input.c_str());

            if (tosend[0] == '/') {
                memmove(tosend, tosend+1, strlen (tosend+1) + 1);
                user_input = "5" + user_input;
            } else user_input = "0" + user_input;

            send(user_input);
            
        }
    }
}


