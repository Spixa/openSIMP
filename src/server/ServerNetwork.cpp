#include "ServerNetwork.h"
#include <string>
#include <sstream>


ServerNetwork::ServerNetwork(unsigned short port) : listen_port(port) {
    logl("Server: Server has begun on port " + std::to_string(port));
    if (listener.listen(listen_port) != sf::Socket::Done) {
        logl("Error > Could not establish listener.");
    }
    init();
}

void ServerNetwork::init() {

    chatSend = new Property(chatSend_str);
    handler = new ChatHandler();
    objs.push_back(handler);

    for (auto x : objs) {
        x->start();
    }

}

void ServerNetwork::connectClients(std::vector<sf::TcpSocket*>* client_array) {
    while (true) {
        sf::TcpSocket* new_client = new sf::TcpSocket();
        if (listener.accept(*new_client) == sf::Socket::Done) {
            new_client->setBlocking(false);
            client_array->push_back(new_client);
            clientid_array.push_back("\x96");
            logl("Server > " << new_client->getRemoteAddress() << ":" << new_client->getRemotePort() << " connected. [" << client_array->size() << "]");

            std::string joinMessage = "1\x01" + new_client->getRemoteAddress().toString() + ":" + std::to_string(new_client->getRemotePort()) + " connected.";

            broadcast(joinMessage.c_str(),new_client->getRemoteAddress(), new_client->getRemotePort());
            
        }
        else {
            logl("Server failed to accept new sockets\n\trestart the server");
            delete(new_client);
            break;
        }
    }
}

void ServerNetwork::disconnectClient(sf::TcpSocket* socket_pointer, size_t position, DisconnectReason reason) {
    log("Server > " << socket_pointer->getRemoteAddress() << ":" << socket_pointer->getRemotePort() << " disconnected for ");

    
    std::string leaveMessage;

    if (clientid_array[position] != "\x96") leaveMessage = "2\x01" + clientid_array[position] + " disconnected for ";
    else  leaveMessage = "2\x01" + socket_pointer->getRemoteAddress().toString() + ":" + std::to_string(socket_pointer->getRemotePort()) + " disconnected for ";


    switch (reason) {
        case DisconnectReason::DisconnectLeave:
            leaveMessage += "Generic Leave Activity";
            logl("Generic Leave Activity");
        break;
        case DisconnectReason::DisconnectKick:
            leaveMessage += "Nuisance Activities";
            logl("Undefined behavior");
        break;
        case DisconnectReason::DisconnectUnnamed:
            leaveMessage += "Unnamed user";
            logl("Unnamed user");
            logl("Kicked junk client trying to send message.");
        break;
    }
    
    broadcast(leaveMessage.c_str(),socket_pointer->getRemoteAddress(), socket_pointer->getRemotePort());

    socket_pointer->disconnect();
    delete(socket_pointer);
    client_array.erase(client_array.begin() + position);
    clientid_array.erase(clientid_array.begin() + position);
}



void ServerNetwork::broadcast(const char* data, sf::IpAddress exclude_address, unsigned short port) {
    for (size_t iterator = 0; iterator < client_array.size(); iterator++) {
        sf::TcpSocket* client = client_array[iterator];
        if (client->getRemoteAddress() != exclude_address || client->getRemotePort() != port) {
            size_t counter = 0; while (true) {
                if (data[counter] != '\0') counter++;
                else break;
            }
            send(data,counter,client);
        }
    }
}

bool ServerNetwork::send(const char* data, size_t counter, sf::TcpSocket* to) {
    if (to->send(data, counter,counter) != sf::Socket::Done) {
        logl("Could not send packet on broadcast");
        return false;
    }
    return true;
}

bool ServerNetwork::check(char* data) {
  char * ptr;
  int    ch = '\x01';
  ptr = strchr( data, ch );
  if (ptr == NULL) return true;
  return false; 
}
 
void ServerNetwork::receive(sf::TcpSocket* client, size_t iterator) {
    char received_data[MAX_RAW_DATA]; size_t received_bytes;
    memset(received_data, 0, sizeof(received_data));
    if (client->receive(received_data, sizeof(received_data), received_bytes) == sf::Socket::Disconnected) {
        disconnectClient(client, iterator,DisconnectReason::DisconnectLeave);
    }
    else if (received_bytes > 0) {


        if (!check(received_data)) {
            if (clientid_array[iterator] != "\x96") logl("Invalid send from " << clientid_array[iterator]);
            else logl("Invalid send from " << client->getRemoteAddress() << ":" << client->getRemotePort());
            logl("Integrity: Remote " << client->getRemoteAddress() << ":" << client->getRemotePort() << " sent an illegal charachter");
            disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
            return;
        }

        if (received_bytes >= 256) {
            
            if (clientid_array[iterator] != "\x96") logl("Invalid send from " << clientid_array[iterator]);
            else logl("Invalid send from " << client->getRemoteAddress() << ":" << client->getRemotePort());

            logl("Integrity: Remote " << client->getRemoteAddress() << ":" << client->getRemotePort() << " sent too many charachters");
            disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
            return;
        }
        
        std::stringstream sending_string;      

        // Handle receives
        if ((strncmp("0",received_data,1) == 0)) {
            if(!handleSend(received_data,sending_string,client,iterator)) return;
        } else
        if ((strncmp("3",received_data,1) == 0)) {
            if(!handleNick(received_data,sending_string,client,iterator)) return;
            
        }
        else {
            disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
            return;
        }
        

        std::string convertedSS = sending_string.str();
        char char_array[256] = {'\0'};
        strcpy(char_array,convertedSS.c_str());
        broadcast(char_array, client->getRemoteAddress(), client->getRemotePort());

        if (clientid_array[iterator] != "\x96") logl(clientid_array[iterator] << " sent '" << received_data << "'"); 
        else logl(client->getRemoteAddress().toString() << ":" << client->getRemotePort() << " sent '" << received_data << "'"); 
   
        
        updateObjs();
        chatSend_str = "";
    }
}

bool ServerNetwork::handleSend(char* received_data,std::stringstream& sending_string,sf::TcpSocket* client, size_t iterator) {
    memmove(received_data, received_data+1, strlen (received_data+1) + 1);
    if (clientid_array[iterator] != "\x96") {
        sending_string << "0" << "\x01" << received_data << "\x01" << clientid_array[iterator];
        chatSend_str = received_data;
    }
    else {
        disconnectClient(client,iterator,DisconnectReason::DisconnectUnnamed);
        return false;
    }
    
    // Normal
    return true;
}

bool ServerNetwork::handleNick(char* received_data,std::stringstream& sending_string,sf::TcpSocket* client, size_t iterator) {
    memmove(received_data, received_data+1, strlen (received_data+1) + 1);
    // Check whether name already exists
    for(auto i : clientid_array) {
        if (received_data == i) {        
            disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
            logl("\tAn attempt of connection with an already online alias was blocked.");
            return false;
        }
    } 
    sending_string << "3" << "\x01" << received_data << "\x01" << client->getRemoteAddress().toString() << "\x01" << std::to_string(client->getRemotePort());
    logl(client->getRemoteAddress().toString() << ":" << std::to_string(client->getRemotePort()) << " is now recognized as " << received_data );
    clientid_array[iterator] = received_data;

    // Normal
    return true;
}

void ServerNetwork::updateObjs() {
    *chatSend = Property(chatSend_str);
    for (auto x : objs) {
        x->update(*chatSend);
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
