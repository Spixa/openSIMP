#include "Executor.h"
#include "Command.h"
#include "ServerNetwork.h"
#include <iostream>
void Executor::pushNewCommand(ExecutableCommand* e) {
    // check duplicate
    for (auto x : m_cmds) {
        assert(x->str() != e->str());
    }
    m_cmds.push_back(e);
}

void Executor::iterate(std::string e, sf::TcpSocket* sock, size_t iter) {
    bool executed = false;
    for (auto x : m_cmds) {
        if (e == x->str()) {
            assert(executed == false); // Duplicated command!
            x->execute(sock,iter);
            executed = true;
        }
    }
    if (!executed) {
        ServerNetwork::Get()->sendString("Command " + e + " was not found. Type /help to get help.", sock);
    }
}
std::vector<ExecutableCommand*> Executor::getVector() {
    return m_cmds;
}