#include <server/commands/command.h>

ExecutableCommand::ExecutableCommand(std::string str,std::function<simp::cmd_status(sf::TcpSocket* sock, size_t iterator, std::string args[])> f)
    : m_constructed(true)
{
    fn = f;
    this->cmd = str;
}
void ExecutableCommand::setExecutor(Executor* exectr) {
    assert(m_constructed == true);
    exectr->pushNewCommand(this);
}
simp::cmd_status ExecutableCommand::execute(sf::TcpSocket* sock, size_t iter, std::string args[]) {
    return fn(sock, iter, args);
}
std::string ExecutableCommand::str() {
    return cmd;
}
//////////////

ExecutableCommand* getCommand(std::string str,std::function<simp::cmd_status(sf::TcpSocket* sock, size_t iterator, std::string args[])> f) {
    return new ExecutableCommand(str,f);
}