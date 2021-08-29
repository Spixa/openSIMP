#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include <SFML/Network.hpp>
#include <functional>
#include <assert.h>

//#include <server/commands/command.h>
class Executor;

namespace simp {
    enum class cmd_status {
        OK,
        FAILED,
        ERROR,
    };
};

class ExecutableCommand {
public:
    ExecutableCommand(std::string str,std::function<simp::cmd_status(sf::TcpSocket* sock, size_t iterator, std::string[])> f);
    void setExecutor(Executor* exectr);
    simp::cmd_status execute(sf::TcpSocket* sock, size_t iter, std::string[]);
    std::string str();

private:
    bool m_constructed = false;
    std::function<simp::cmd_status(sf::TcpSocket* sock, size_t iterator, std::string[])> fn;
    std::string cmd;
};

ExecutableCommand* getCommand(std::string str,std::function<simp::cmd_status(sf::TcpSocket* sock, size_t iterator, std::string[])> f);



// depreciated in 8/27/2021
// added in 8/26/2021 - didnt last long lol
// class Command {
// public:
//     Command(const std::string& cmd);
//     void setExecutor(Executor& e);
//     void bruh() {}
//     virtual void operate(sf::TcpSocket* socket,size_t iterator);
// private:
// };
// depreciated
//#define MAKE_CLASS(x,y,z) class x : public Command{ public: x() : Command(y) {} };
#endif
