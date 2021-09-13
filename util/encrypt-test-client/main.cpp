#include "cryptography.h"

#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <sstream>

using namespace Botan;
#define SERVER_POSITION 0

BigInt server_pubkey;
sf::TcpSocket sock;
Cryptography* crypt;

bool send(const char* data, size_t counter, sf::TcpSocket* to) {
    if (to->send(data, counter,counter) != sf::Socket::Done) {
        std::cout << "Could not send most recent packet on stack (from: localhost, to: " << to->getRemoteAddress();
        return false;
    }
    return true;
}

void receive() {
    char data[4096]; size_t bytes;
    if (sock.receive(data,sizeof(data), bytes) == sf::Socket::Done) {
        server_pubkey = BigInt(data);
        crypt->pushNewClientKey(server_pubkey);
        
        // send
        std::string encrypted = crypt->RSA_encrypt(0, "spixa");

        // with bytes
        // BigInt x = BigInt(hex_decode(encrypted));
        // std::stringstream str;
        // str << x;

        // with hex
        std::stringstream str;
        str << crypt->RSA_encrypt(0, "spixa");

        send(str.str().c_str(), str.str().length(), &sock);
    } 

}


int main() {
    std::cout << "Generating keypair...\n";
    crypt = new Cryptography();
    std::cout << "Generated keypair\n";
    sock.connect("localhost", 37549);

    while (true) {
        receive();
    }
}