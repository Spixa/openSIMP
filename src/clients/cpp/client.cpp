#include "ClientNetwork.h"


int main() {
    ClientNetwork client_network;
    client_network.connect("127.0.0.1", 37549);
    client_network.run();
    return 0;
}