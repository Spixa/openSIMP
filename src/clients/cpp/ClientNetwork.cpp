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

void ClientNetwork::receivePackets(sf::TcpSocket* socket) {
    while (true) {
        if (socket->receive(last_packet) == sf::Socket::Done) {
            std::string received_string; std::string sender_address; unsigned short sender_port; unsigned short type_int, messagetype_int;
            last_packet >> type_int >> messagetype_int >> received_string  >> sender_address >> sender_port;

            if (static_cast<PacketType>(type_int) == PacketType::MessagePacket) {
                if (static_cast<MessageType>(messagetype_int) == MessageType::ChatMessage)
                    logl("<" << sender_address << ":" << sender_port << ">: " << received_string);
                else if (static_cast<MessageType>(messagetype_int) == MessageType::BroadcastMessage)
                    logl("[Broadcast: " << sender_address << ":" << sender_port << "] " << received_string);
                else if (static_cast<MessageType>(messagetype_int) == MessageType::DirectMessage) {
                    logl("[Directed from " << sender_address << "] " << received_string);
                }
            }
            
            else if (static_cast<PacketType>(type_int) == PacketType::JoinPacket) {
                logl(received_string << " has connected.");
            }
            else if (static_cast<PacketType>(type_int) == PacketType::LeavePacket) {
                logl(received_string << " has disconnected.");
            }


           // TODO: Make this a switch statement (nevermind)
        }

        std::this_thread::sleep_for((std::chrono::milliseconds)100);
    }
}

void ClientNetwork::sendPacket(sf::Packet& packet) {
    if (packet.getDataSize() > 0 && socket.send(packet) != sf::Socket::Done) {
        logl("Could not send packet");
    }
}

void ClientNetwork::run() {
    std::thread reception_thred(&ClientNetwork::receivePackets, this, &socket);

    while (true) {
        if (isConnected) {
            std::string user_input;
            std::string args[512];
            std::getline(std::cin, user_input);
            

            sf::Packet reply_packet;
            Utils::lexer(user_input,args,' ');
            if (args[0] == "/broadcast") {
                std::string new_message;
                for (int i=1; i < 256;i++)
                    new_message += args[i] +  " ";
             
                reply_packet << static_cast<unsigned short>(PacketType::MessagePacket) << static_cast<unsigned short>(MessageType::BroadcastMessage) << new_message;
            } else
                reply_packet << static_cast<unsigned short>(PacketType::MessagePacket) << static_cast<unsigned short>(MessageType::ChatMessage) << user_input;
            
            sendPacket(reply_packet);
        }
    }
}


