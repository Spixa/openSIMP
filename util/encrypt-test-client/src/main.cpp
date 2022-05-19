#include "cryptography.h"

#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

using namespace Botan;
#define SERVER_POSITION 0

BigInt server_pubkey;
sf::TcpSocket sock;
Cryptography* crypt;
std::string aes_key;
bool hasHandshook{false};

bool send(const char* data, size_t counter, sf::TcpSocket* to) {
    if (to->send(data, counter,counter) != sf::Socket::Done) {
        std::cout << "Could not send most recent packet on stack (from: localhost, to: " << to->getRemoteAddress();
        return false;
    }
    return true;
}


bool send_enc(const char* data, size_t counter, sf::TcpSocket* to) {

    auto var = crypt->encrypt(std::string{data}, aes_key.c_str());

    std::string sendee{var.begin(), var.end()};

    size_t length = sendee.length();
    if (to->send(sendee.c_str(),length ,length) != sf::Socket::Done) {
        std::cout << "Could not send most recent packet on stack (from: localhost, to: " << to->getRemoteAddress();
        return false;
    }
    return true;
}

enum class HandshakeStatus {
    HasntBegun,
    ReceivedPubKey,
    SentCredientials,
    SentPubKey, 
    ReceivedAESKey
} status = HandshakeStatus::HasntBegun;

void handshake(const std::string& UNAME, const std::string& PASSWD) {
        char data[4096]; size_t bytes;
        if (sock.receive(data,sizeof(data), bytes) == sf::Socket::Done) {
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

                send(str.str().c_str(), str.str().length(), &sock);
                status = HandshakeStatus::SentCredientials;
            } else 
            if (status == HandshakeStatus::SentCredientials) {
                std::stringstream str;
                str << crypt->getPublicKey();
                send(str.str().c_str(), str.str().length(), &sock);
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

void receive() {
    while (true) {
        char data[4096]; size_t bytes;
        if (sock.receive(data,sizeof(data), bytes) == sf::Socket::Done) {
            secure_vector<uint8_t> vec{};
            vec.resize(bytes);
            std::transform(data, data + bytes, vec.begin(), [](char v) {return static_cast<uint8_t>(v);});

            auto g = crypt->decrypt(vec, aes_key.c_str());
            std::cout << " --> ";
            for (auto x : g["msg"]) {
                std::cout << x;
            }
            std::cout << std::endl;  
        }
    }
}

int main() {
    std::cout << "Generating keypair...\n";
    crypt = new Cryptography();
    std::cout << "Generated keypair\n";


        std::string uname,passwd;
    std::cout << "Username: ";
    std::getline(std::cin, uname);
    std::cout << "Password: ";
    std::getline(std::cin, passwd);

    sock.connect("localhost", 37549);



    std::cout << "Logging in...\n\n";
    while (!hasHandshook) {
        handshake(uname, passwd);
    }
    std::cout << "Logged in\n\n";

    std::thread reception_thred(&receive);

    while (true) {
        std::string user_input;
        std::getline(std::cin, user_input);

        char* tosend = const_cast<char*>(user_input.c_str());

        if (tosend[0] == '/') {
            memmove(tosend, tosend+1, strlen (tosend+1) + 1);
            user_input = "5" + user_input;
        } else user_input = "0" + user_input;

        send_enc(user_input.c_str(), user_input.length(), &sock);
    }
} 
