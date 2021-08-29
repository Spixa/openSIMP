#include "Executor.h"
#include "Command.h"
#include "ServerNetwork.h"
#include "Lexer.h"

// stdafx
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
    std::string command[512];
    Lexer::lex(e, command, ' ');

    for (auto x : m_cmds) {
        if (command[0] == x->str()) {
            assert(executed == false); // Duplicated command!
            if (x->execute(sock, iter, command) == cmd_status::ERROR) {
                logl("an internal error occured while executing command (by: ServerExecutor." << sock->getRemoteAddress().toString() << ".send_cmd<" << command[0] << ".cmd>");
            }
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