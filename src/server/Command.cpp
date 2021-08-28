
#include "Command.h"

ExecutableCommand::ExecutableCommand(std::string str,std::function<simp::cmd_status(sf::TcpSocket* sock, size_t iterator)> f)
    : m_constructed(true)
{
    fn = f;
    this->cmd = str;
}
void ExecutableCommand::setExecutor(Executor* exectr) {
    assert(m_constructed == true);
    exectr->pushNewCommand(this);
}
void ExecutableCommand::execute(sf::TcpSocket* sock, size_t iter) {
    fn(sock, iter);
}
std::string ExecutableCommand::str() {
    return cmd;
}
//////////////

ExecutableCommand* getCommand(std::string str,std::function<simp::cmd_status(sf::TcpSocket* sock, size_t iterator)> f) {
    return new ExecutableCommand(str,f);
}