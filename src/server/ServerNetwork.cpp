#include "ServerNetwork.h"
#include <string>
#include <sstream>


std::string ServerNetwork::convertToString(char* a, int size)
{
    int i;
    std::string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

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
            client_op_array.push_back(false);
            client_message_interval.push_back(new sf::Clock());
            logl("Unregistered " << new_client->getRemoteAddress() << ":" << new_client->getRemotePort() << " was accepted [" << client_array->size() << "]");
            
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
    client_message_interval.erase(client_message_interval.begin() + position);
}



bool ServerNetwork::broadcast(const char* data, sf::IpAddress exclude_address, unsigned short port) {
    bool succeed = false;
    for (size_t iterator = 0; iterator < client_array.size(); iterator++) {
        sf::TcpSocket* client = client_array[iterator];
        if (client->getRemoteAddress() != exclude_address || client->getRemotePort() != port) {
            size_t counter = 0; while (true) {
                if (data[counter] != '\0') counter++;
                else break;
            }
            if (send(data,counter,client)) succeed = true;
            else succeed = false;
        }
    }

    return succeed;
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

bool ServerNetwork::isOp(size_t iter) {
    return client_op_array[iter];
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
                handleNick(received_data,client,iterator);
                
                return;
                
            } else
            if ((strncmp("4",received_data,1) == 0)) {
                
            } else
            if ((strncmp("5",received_data,1) == 0)) {
                handleCommand(received_data,client,iterator);
            }
            else {
                disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
                return;
            }
        
        if (client_message_interval[iterator]->getElapsedTime().asMilliseconds() >= 300) {

            std::string convertedSS = sending_string.str();
            char char_array[256] = {'\0'};
            strcpy(char_array,convertedSS.c_str());
            broadcast(char_array, client->getRemoteAddress(), client->getRemotePort());
                if (clientid_array[iterator] != "\x96") logl(clientid_array[iterator] << " sent '" << received_data << "'"); 
                else logl(client->getRemoteAddress().toString() << ":" << client->getRemotePort() << " sent '" << received_data << "'"); 
            
            
            updateObjs();
            chatSend_str = "";

            client_message_interval[iterator]->restart();

        } else {
            std::string message = "6\x01 [KoolDawn] Cool it G";
            send(message.c_str(),message.length() + 1, client);

            // lastQueuer = client;
            // send_queue.push_back(sending_string.str());
            
        }
    }
}

void ServerNetwork::handleCommand(char* received_data,sf::TcpSocket* client, size_t iterator) {
    memmove(received_data, received_data+1, strlen (received_data+1) + 1);
    char* message = received_data;
    if (strcmp(message,"list") == 0) {
        std::stringstream list_all;
        list_all << "5\x01\nList of online users: ";
        for (auto x: clientid_array) {
            if (x != "\x96") list_all << x << " ";
        }
        send(list_all.str().c_str(),list_all.str().length() + 1,client);
    } else
    if (strcmp(message,"getpos") == 0){
        std::stringstream pos;
        pos << "5\x01 Current pos: " << std::to_string(iterator);
        send(pos.str().c_str(),pos.str().length() + 1, client);
    }
 
}
bool ServerNetwork::handleSend(char* received_data,std::stringstream& sending_string,sf::TcpSocket* client, size_t iterator) {
    memmove(received_data, received_data+1, strlen (received_data+1) + 1);
    chatSend_str = received_data;

    bool invld_res = (chatSend_str.find_first_not_of(" \t\n\v\f\r") == std::string::npos);

    if (invld_res) {
        std::string message = "6\x01Stop sending empty shit.";
        send(message.c_str(),message.length() + 1, client);
        return false;
    }

    if (clientid_array[iterator] != "\x96") {
        sending_string << "0" << "\x01" << received_data << "\x01" << clientid_array[iterator];

    }
    else {
        disconnectClient(client,iterator,DisconnectReason::DisconnectUnnamed);
        return false;
    }


    
    // Normal
    return true;
}

bool ServerNetwork::handleNick(char* received_data,sf::TcpSocket* client, size_t iterator) {
    memmove(received_data, received_data+1, strlen (received_data+1) + 1);
    // Check whether name already exists
    for(auto i : clientid_array) {
        if (received_data == i) {        
            disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
            logl("\tAn attempt of connection with an already online alias was blocked.");
            return false;
        }
    } 
    char char_array[256] = {'\0'};
    strcpy(char_array,received_data);
    
    chatSend_str = received_data;
    bool invld_res = (chatSend_str.find_first_not_of(" \t\n\v\f\r") == std::string::npos);

    if (invld_res) {
        std::string message = "6\x01Stop sending empty shit.";
        send(message.c_str(),message.length() + 1, client);
        return false;
    }


    std::stringstream joinMessageStream;
    joinMessageStream << "1\x01" << char_array << " connected.";

    broadcast(joinMessageStream.str().c_str(),client->getRemoteAddress(), client->getRemotePort()); 
    logl(client->getRemoteAddress().toString() << ":" << std::to_string(client->getRemotePort()) << ": Remote entry is masked to " << received_data );
    clientid_array[iterator] = received_data;


    // Normal
    return true;
}

void ServerNetwork::handleRequestedConsole(sf::TcpSocket* sock,size_t iterpos) {
    log("[OP ME]" << sock->getRemoteAddress().toString() << ":" << sock->getRemotePort() << " (AKA:" << clientid_array[iterpos] << "): Remote is requesting to become an operator. [Y/N]");
    char say;
    std::cin >> say;
    switch (say) {
        case 'Y' | 'y': 
            logl("Opped remote.");
            client_op_array[iterpos] = true;
        break;
        case 'N' | 'n':
            logl("Denied");
            return;
        break;
        default:
            logl("wrong input.");
            return;
    }      


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

void ServerNetwork::sendQueuedMessage(std::vector<std::string>& vec, sf::TcpSocket* client) {
    while (true) {
        if (client != nullptr) {
            if (!vec.empty()) {
                sf::Clock* waiter = new sf::Clock();
                for (size_t i;i < vec.size();i++) {
                    while (waiter->getElapsedTime().asMilliseconds() < 350) {
                    }
                    const char* tosend;
                    tosend = std::string(vec[i]).c_str();
                    logl(tosend);
                    broadcast(tosend, client->getRemoteAddress(),client->getRemotePort());
                    vec.erase(vec.begin() + i);
                }
            }
        }
        std::this_thread::sleep_for((std::chrono::milliseconds)50);
    }
}

void ServerNetwork::run() {
    std::thread connetion_thread(&ServerNetwork::connectClients, this, &client_array);
    std::thread queue_thread(&ServerNetwork::sendQueuedMessage, this, std::ref(send_queue),lastQueuer);

    // Manage thread for receiving and sending
    manage();
}
