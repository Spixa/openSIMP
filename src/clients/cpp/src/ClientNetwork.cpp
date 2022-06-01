#include "ClientNetwork.h"
#include <cstdlib>
#include "Utils.hpp"
#include <sstream>

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

            Botan::secure_vector<uint8_t> vec{};
            vec.resize(received);
            std::transform(buffer, buffer + received, vec.begin(), [](char v) {return static_cast<uint8_t>(v);});

            auto g = crypt->decrypt(vec, aes_key.c_str());
            std::string pt{g["msg"].begin(), g["msg"].end()};

            // size_t counter{};
            // for (auto x : pt) {
            //     if (x == '\x01') {
            //         log(" -- ");
            //         counter++;
            //     } else {
            //         log(x);
            //     }
            // }
            // logl('\n' << counter << " receivees in total.");
            std::string args[100];
            Utils::lexer(pt, args, '\x01');

            if (args[0] == "0") {
                logl("<" << args[2] << "> " << args[1]);
            } else
            if (args[0] == "1" || args[0] == "2") {
                logl(args[1] + ((args[0] == "1") ? " joined" : " left"));
                log("List: ");
                for (int i = 2; i < 100; i++) {
                    log(args[i] + " ");
                }
                logl("");
            }  else
            if (args[0] == "5") {
                logl("(Command) " + args[1]);
            } else
            if (args[0] == "6") {
                logl("[Direct] " + args[1]);
            }

        } else if (status == sf::Socket::Disconnected) {
            log("You were disconnected from the server.");
            ::exit(0);
        }
        std::this_thread::sleep_for((std::chrono::milliseconds)100);
    }
}

void ClientNetwork::handshake(const std::string& UNAME, const std::string& PASSWD) {
        char data[4096]; size_t bytes;
        if (socket.receive(data,sizeof(data), bytes) == sf::Socket::Done) {
            if (status == HandshakeStatus::HasntBegun) {
                server_pubkey = BigInt(data);
                crypt->pushNewClientKey(server_pubkey);
                status = HandshakeStatus::ReceivedPubKey;

                std::stringstream str;
                std::stringstream credits;
                credits << UNAME << '\x01' << PASSWD;
                    auto a = crypt->RSA_encrypt(0, credits.str());
                    for (auto x : a) {
                        str << x;
                    }


                send_raw(str.str());
                status = HandshakeStatus::SentCredientials;
            } else 
            if (status == HandshakeStatus::SentCredientials) {
                std::stringstream str;
                str << crypt->getPublicKey();
                send_raw(str.str());
                status = HandshakeStatus::SentPubKey;
            } else
            if (status == HandshakeStatus::SentPubKey) {
                secure_vector<uint8_t> vec{};
                vec.resize(bytes);
                std::transform(data, data + bytes, vec.begin(), [](char v) {return static_cast<uint8_t>(v);});
                std::string server_key_str_enc{vec.begin(), vec.end()};

                status = HandshakeStatus::ReceivedAESKey;
                auto v= crypt->RSA_decrypt(vec);
                aes_key = std::string{v.begin(), v.end()};
            
                std::cout << std::endl;
                hasHandshook = true;
        } 
    }
}

void ClientNetwork::send(std::string const& sent) {
    
    auto var = crypt->encrypt(sent, aes_key.c_str());

    std::string sendee{var.begin(), var.end()};

    size_t length = sendee.length();
    if (socket.send(sendee.c_str(),length ,length) != sf::Socket::Done) {
        std::cout << "Could not send most recent packet on stack (from: localhost, to: " << socket.getRemoteAddress();
    }
}

void ClientNetwork::send_raw(std::string const& sent) {
    
    if (sent.length() > 0 && socket.send(sent.c_str(),sent.length()) != sf::Socket::Done) {
        logl("Could not send data");
    }

}

void ClientNetwork::run() {
    crypt = new Cryptography();
    std::cout << "You may now authenticate\n";

    log("Username: ");
    std::string uname, password;
    std::getline(std::cin, uname);
    log("Password: ");
    std::getline(std::cin, password);
    
    while (!hasHandshook) {
        handshake(uname, password);
    }

    std::thread reception_thred(&ClientNetwork::receive, this, &socket);
    while (true) {
        if (isConnected) {
            std::string user_input;
            std::getline(std::cin, user_input);
            if (user_input.empty()) {
                user_input = "AA";
            }

            char* tosend = const_cast<char*>(user_input.c_str());


            if (tosend[0] == '/') {
                memmove(tosend, tosend+1, strlen (tosend+1) + 1);
                user_input = "5" + user_input;
            } else user_input = "0" + user_input;
            
            send(user_input);
            
        }
    }
}


