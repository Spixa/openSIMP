
#include "ServerNetwork.h"

#include <iostream>

#define logl(x) std::cout << x << std::endl

void repo() {
    log(__func__ << "(): repo is https://github.com/spixa/opensimp\n\t");
}

void contributers() {
    log(__func__ << "(): list of contributors: KasraIDK, phoenix_ir_ (phnixir), Spixa");
}

int main() {

    //logl("Server\n\nSuschat - SusServer\n\tContributors:\n\thttps://github.com/phnixir\n\thttps://github.com/KasraIDK\n\thttps://github.com/Spixa");
    log("openSIMP Server v0.1\n\t");
    repo();
    contributers();
    logl("");

    ServerNetwork::Get()->init(37549);
    ServerNetwork::Get()->run(); 

    return 0;
}