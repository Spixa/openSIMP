#include "ClientNetwork.h"
#include "PacketType.h"

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
            std::string received_string; std::string sender_address; unsigned short sender_port; int type_int, messagetype_int;
            last_packet >> type_int >> messagetype_int >> received_string  >> sender_address >> sender_port;

            if (static_cast<PacketType>(type_int) == PacketType::MessagePacket) {
                if (static_cast<MessageType>(messagetype_int) == MessageType::ChatMessage)
                    logl("<" << sender_address << ":" << sender_port << ">: " << received_string);
                else if (static_cast<MessageType>(messagetype_int) == MessageType::BroadcastMessage)
                    logl("[Broadcast: " << sender_address << ":" << sender_port << "] " << received_string);
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

            std::getline(std::cin, user_input);
            

            sf::Packet reply_packet;
            reply_packet << static_cast<int>(PacketType::MessagePacket) << static_cast<int>(MessageType::ChatMessage) << user_input;

            sendPacket(reply_packet);
        }
    }
}