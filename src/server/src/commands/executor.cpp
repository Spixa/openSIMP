#include <server/commands/executor.h>
#include <server/commands/command.h>
#include <server/servernetwork/servernetwork.h>
#include <server/commands/lexer.h>
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

    std::vector<std::string> v(std::begin(command), std::end(command));

    for (auto x : m_cmds) {
        if (command[0]  == x->str()) {
            assert(executed == false); // Duplicated command!
            if (x->execute(sock, iter, v) == cmd_status::ERROR) {
                warn("an internal error occured while executing command.");
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