#include "ServerNetwork.h"
#include <iostream>

#define logl(x) std::cout << x << std::endl


int main(int argc, char* argv[]) {

    logl("SusServer (c) from ze sus group");
    
    ServerNetwork server_network(37549, false);
    server_network.run();
    return 0;
}