#include "ServerNetwork.h"
#include <string>
#include <sstream>
#include "PacketType.h"

ServerNetwork::ServerNetwork(unsigned short port) : listen_port(port) {
    logl("Server: Server has begun on port " + std::to_string(port));
    if (listener.listen(listen_port) != sf::Socket::Done) {
        logl("Error > Could not establish listener.");
    }
}

void ServerNetwork::connectClients(std::vector<sf::TcpSocket*>* client_array) {
    while (true) {
        sf::TcpSocket* new_client = new sf::TcpSocket();
        if (listener.accept(*new_client) == sf::Socket::Done) {
            new_client->setBlocking(false);
            client_array->push_back(new_client);
            logl("Server > " << new_client->getRemoteAddress() << ":" << new_client->getRemotePort() << " connected. [" << client_array->size() << "]");
        }
        else {
            logl("Server failed to accept new sockets\n\trestart the server");
            throw std::runtime_error("failure to establish TCP stream to accept new sockets.");
            delete(new_client);
            break;
        }
    }
}

void ServerNetwork::disconnectClient(sf::TcpSocket* socket_pointer, size_t position) {
    logl("Server > " << socket_pointer->getRemoteAddress() << ":" << socket_pointer->getRemotePort() << " disconnected.");

    

    socket_pointer->disconnect();
    delete(socket_pointer);
    client_array.erase(client_array.begin() + position);
}



void ServerNetwork::broadcast(const char* data, sf::IpAddress exclude_address, unsigned short port) {
    for (size_t iterator = 0; iterator < client_array.size(); iterator++) {
        sf::TcpSocket* client = client_array[iterator];
        if (client->getRemoteAddress() != exclude_address || client->getRemotePort() != port) {
            send(data,client);
        }
    }
}

bool ServerNetwork::send(const char* data, sf::TcpSocket* to) {
    size_t counter = 0; while (true) {
        if (data[counter] != '\0') counter++;
        else break;
    }
    if (to->send(data, counter,counter) != sf::Socket::Done) {
        logl("Could not send packet on broadcast");
        return false;
    }
    return true;
}

void ServerNetwork::receive(sf::TcpSocket* client, size_t iterator) {
    char received_data[MAX_RAW_DATA]; size_t received_bytes;
    memset(received_data, 0, sizeof(received_data));
    if (client->receive(received_data, sizeof(received_data), received_bytes) == sf::Socket::Disconnected) {
        disconnectClient(client, iterator);
    }
    else if (received_bytes > 0) {
        std::stringstream sending_string;
        sending_string << received_data << "\x00\x09\x00" << client->getRemoteAddress().toString() << "\x00\x00\x00" << std::to_string(client->getRemotePort());

        std::string convertedSS = sending_string.str();

        char char_array[256] = {'\0'};
        strcpy(char_array,convertedSS.c_str());

        broadcast(char_array, client->getRemoteAddress(), client->getRemotePort());
        logl(client->getRemoteAddress().toString() << ":" << client->getRemotePort() << " sent '" << char_array << "'");
    }
}

void ServerNetwork::manage() {
    while (true) {
        for (size_t iterator = 0; iterator < client_array.size(); iterator++) 
            receive(client_array[iterator], iterator);
        
        std::this_thread::sleep_for((std::chrono::milliseconds)100);
    }
}

void ServerNetwork::run() {
    std::thread connetion_thread(&ServerNetwork::connectClients, this, &client_array);

    // Manage thread for receiving and sending
    manage();
}
