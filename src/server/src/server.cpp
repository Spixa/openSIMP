
#include <server/servernetwork/servernetwork.h>

int main() {
    ServerNetwork::Get()->init(37549);
    ServerNetwork::Get()->run();
    exit(EXIT_SUCCESS);
}