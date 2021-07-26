
#include "ServerNetwork.h"

#include <iostream>

#define logl(x) std::cout << x << std::endl


int main() {

    logl("Server\n\nSuschat - SusServer\n\tContributors:\n\thttps://github.com/phnixir\n\thttps://github.com/KasraIDK\n\thttps://github.com/Spixa");
    
    ServerNetwork server_network(37549);
    server_network.run();
    return 0;
}