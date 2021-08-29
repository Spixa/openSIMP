#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <string>
#include <vector>
#include <SFML/Network.hpp>

class ServerNetwork;
class ExecutableCommand;
class Executor {
public:
    void pushNewCommand(ExecutableCommand* e);
    void iterate(std::string e, sf::TcpSocket* sock, size_t iter);
    std::vector<ExecutableCommand*> getVector();
private:
    std::vector<ExecutableCommand*> m_cmds;

    void checkDuplication(ExecutableCommand* toCheckDuplication);

};
#endif