#include "ClientNetwork.h"

int main() {
    ClientNetwork client_network;
    client_network.connect("localHost", 37549);
    client_network.run();
    return 0;
}