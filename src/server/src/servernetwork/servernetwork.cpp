#include <server/servernetwork/servernetwork.h>
#include <string>
#include <regex>
#include <iterator>
#include <sstream>
#include <fstream>
#include <filesystem>

ServerNetwork* ServerNetwork::m_instance = nullptr;

void ServerNetwork::sendString(std::string str, sf::TcpSocket* client) {
    str = "5\x01" + str;
    send(str.c_str(),str.length()+1, client);
}

std::string ServerNetwork::convertToString(char* a, int size)
{
    int i;
    std::string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

ServerNetwork::ServerNetwork() {
    openUserdata();
}

ServerNetwork* ServerNetwork::Get() {
        if (!m_instance) {
            m_instance = new ServerNetwork;
        }

        return m_instance;  
}

void ServerNetwork::init(unsigned short port) {
    listen_port = port;
    __logl("Generating keypairs...");
    crypt = new Cryptography();

    __logl("Server has begun on port " + std::to_string(port));
    if (listener.listen(listen_port) != sf::Socket::Done) {
        error("Could not establish listener.");
    }
    cmd_executor = new Executor();

    /*
        From @Spixa:
        How to register new commands.
        In order to register commands you need to use the getCommand() funtion which needs a string for the command itself
        and a function to execute upon running it, I made use of lambdas.  
        Note: available arguments during a functions are: TcpSocket* sock, size_t iteration_pisiton, std::string args[]
    */

   // Add commands:
    getCommand("help",CommandLambda { 
        std::string to_send;
        if (args[1] == "") {
            to_send = "Avail commands: ";
            for (auto x : cmd_executor->getVector()) {
                to_send += '/' + x->str() + ' ';
            }
        } else {
            to_send += "Multiple args is not supported by the command: help"; 
            sendString(to_send, sock);
            return cmd_status::ERROR;
        }
        sendString(to_send, sock);
        return cmd_status::OK;
    })->setExecutor(cmd_executor);

    getCommand("suicide",CommandLambda {
        broadcastString(clientid_array[iterator] + " killed themselves.", sock->getRemoteAddress(), sock->getRemotePort());
        disconnectClient(sock, iterator, DisconnectReason::DisconnectKick);
        return cmd_status::OK;
    })->setExecutor(cmd_executor);

    getCommand("plugins", CommandLambda {
        sendString("Plugins (0): ", sock);
        return cmd_status::OK;
    })->setExecutor(cmd_executor);



    std::string a2 = "sus";
    auto d = crypt->encrypt(a2, SERVER_KEY.c_str());
    
    auto g = crypt->decrypt(d, SERVER_KEY.c_str());
    
    for (auto x : g["iv"]) {
        std::cout << x;
    }
    std::cout << " -- ";

    for (auto x : g["msg"]) {
        std::cout << x;
    }
    std::cout << std::endl;
}

void ServerNetwork::broadcastString(std::string str, sf::IpAddress ip, unsigned short port) {
    str = "6\x01" + str; 
    broadcast(str.c_str(), ip, port); // broadcasts
}
void ServerNetwork::connectClients(std::vector<sf::TcpSocket*>* client_array) {
    while (true) {
        sf::TcpSocket* new_client = new sf::TcpSocket();
        if (listener.accept(*new_client) == sf::Socket::Done) {
            new_client->setBlocking(false);
            client_array->push_back(new_client);
            clientid_array.push_back("\x96");
            client_op_array.push_back(false);
            client_authenticated_array.push_back(AuthStatus::Undone);
            client_message_interval.push_back(new sf::Clock());
            __logl("Unregistered " << new_client->getRemoteAddress() << ":" << new_client->getRemotePort() << " was accepted [" << client_array->size() << "]");
            std::stringstream str;
            str << crypt->getPublicKey();

            send_unencrypted(str.str().c_str(), str.str().length() + 1, new_client);
        }
        else {
            error("Server failed to accept new sockets\n\trestart the server");
            delete(new_client);
            break;
        }
    }
}

void ServerNetwork::disconnectClient(sf::TcpSocket* socket_pointer, size_t position, DisconnectReason reason) {  
    std::string leaveMessage;
    if (clientid_array[position] != "\x96") leaveMessage = clientid_array[position] + " disconnected for ";
    else  leaveMessage = socket_pointer->getRemoteAddress().toString() + ":" + std::to_string(socket_pointer->getRemotePort()) + " disconnected for ";
    switch (reason) {
        case DisconnectReason::DisconnectLeave:
            leaveMessage += "requesting leave.";
        break;
        case DisconnectReason::DisconnectKick:
            leaveMessage += "forcibly being kicked by an admin or automatic server moderation.";
        break;
        case DisconnectReason::DisconnectUnnamed:
            leaveMessage += "being unnamed.";
            warn("Previous client \"" << socket_pointer->getRemoteAddress() << ":" << std::to_string(socket_pointer->getRemotePort()) << "\" was kicked due to being unnamed.");
        break;
        default:
            leaveMessage += "an unknown odd reason.";
        break;
    }

    
    __logl(leaveMessage);
    // Add "2" to signify packet type as LeavePacket
    leaveMessage = "2\x01" + leaveMessage; 
    // broadcasts leave message
    broadcast(leaveMessage.c_str(),socket_pointer->getRemoteAddress(), socket_pointer->getRemotePort());

    socket_pointer->disconnect();
    delete(socket_pointer);

    if (client_authenticated_array[position] == AuthStatus::Done)
        crypt->removeClientKey(position);

    client_array.erase(client_array.begin() + position);
    clientid_array.erase(clientid_array.begin() + position);
    client_op_array.erase(client_op_array.begin() + position);
    client_authenticated_array.erase(client_authenticated_array.begin() + position);
    client_message_interval.erase(client_message_interval.begin() + position);


}



bool ServerNetwork::broadcast(const char* data, sf::IpAddress exclude_address, unsigned short port) {
    bool succeed = false;
    for (size_t iterator = 0; iterator < client_array.size(); iterator++) {
        sf::TcpSocket* client = client_array[iterator];
        if (client->getRemoteAddress() != exclude_address || client->getRemotePort() != port) {
            size_t counter = 0; while (true) {
                if (data[counter] != '\0') counter++;
                else break;
            }
            if (send(data,counter,client)) succeed = true;
            else succeed = false;
        }
    }

    return succeed;
}

bool ServerNetwork::send(const char* data, size_t counter, sf::TcpSocket* to) {

    auto ct = crypt->encrypt(std::string{data}, SERVER_KEY.c_str() );
    std::string enc_data{ct.begin(), ct.end()};
    auto length = enc_data.length();
    sf::Socket::Status s = to->send(enc_data.c_str(), enc_data.length(),length);

    if (s != sf::Socket::Done) {
        // if (s == sf::Socket::Error) {
        //     return;
        // }
        error("Could not send most recent packet on stack (from: localhost, to: " << to->getRemoteAddress() << ")");
        return false;
    }
    return true;
}


bool ServerNetwork::send_unencrypted(const char* data, size_t counter, sf::TcpSocket* to) {

    sf::Socket::Status s = to->send(data, counter,counter);

    if (s != sf::Socket::Done) {
        // if (s == sf::Socket::Error) {
        //     return;
        // }
        error("Could not send most recent packet on stack (from: localhost, to: " << to->getRemoteAddress() << ")");
        return false;
    }
    return true;
}

bool ServerNetwork::check(char* data) {
  char * ptr;
  int    ch = '\x01';
  ptr = strchr( data, ch );
  if (ptr == NULL) return true;
  return false; 
}

void ServerNetwork::openUserdata()
{    

    // note: userdata is opened in constructor.
//     YAML::Emitter out;
//     out << YAML::BeginMap;
//     out << YAML::Key << "moge";
//     out << YAML::Value << "amog";
//     out << YAML::Key << "pog";
//     out << YAML::Value << "tf";
//     out << YAML::EndMap;

//     if (std::filesystem::exists("userdata.yml"))
//     {
//         YAML::Node config = YAML::LoadFile("config.yaml");
//     }           
//     else {
//         std::ofstream writer{"userdata.yml"};
//         assert(writer.is_open() && "opensimp: failed to create \"userdata.yml\"\n\tperhaps lacking permission");
                
//         writer << out.c_str();
//     }

// }


    YAML::Node config;
    if (std::filesystem::exists("userdata.yml"))
        config = YAML::LoadFile("userdata.yml");

    const std::string username = config["username"].as<std::string>();
    const std::string password = config["password"].as<std::string>();

    std::cout << "username: " << username << " -- password: " << password << std::endl;
    std::ofstream fout("userdata.yml");
    fout << config;
}


bool ServerNetwork::isOp(size_t iter) {
    return client_op_array[iter];
}
 
void ServerNetwork::receive(sf::TcpSocket* client, size_t iterator) {
    char received_data[MAX_RAW_DATA]; size_t received_bytes;
        memset(received_data, 0, sizeof(received_data));
        if (client->receive(received_data, sizeof(received_data), received_bytes) == sf::Socket::Disconnected) {
            disconnectClient(client, iterator,DisconnectReason::DisconnectLeave);
    }
    else if (received_bytes > 0) {
            // Unathenticated __logic
            if (client_authenticated_array[iterator] == AuthStatus::Undone) {
                // key
                secure_vector<uint8_t> v;
                try {
                
                  secure_vector<uint8_t> vec{};
                  vec.resize(received_bytes);
                  std::transform(received_data, received_data + received_bytes, vec.begin(), [](char v) {return static_cast<uint8_t>(v);});


                  v = crypt->RSA_decrypt(vec);
                } catch(std::exception e) {
                    std::cout << "ayo " << iterator << std::endl;
                    __logl("CXX exception: " << e.what() << "\n\tat " << "botan.decrypt\nMoving on...");
                    return;
                }

                std::ostringstream convert;
                for (int a = 0; a < v.size(); a++) {
                    convert << v[a];
                }

                std::string decstr = convert.str();

                enum _AuthStatus {
                    GotUsername,
                    GotUsernameAndPassword,
                    GotNeither
                } user_status = _AuthStatus::GotNeither;
                
                std::string uname;
                std::string passwd;

                for (auto x : decstr)
                {
                    if (x == '\x01') {
                        if (user_status == _AuthStatus::GotUsername) {
                            user_status = _AuthStatus::GotUsernameAndPassword;
                        }
                        else if (user_status == _AuthStatus::GotNeither)
                        {
                            user_status = _AuthStatus::GotUsername;
                        }
                    } else if (user_status == _AuthStatus::GotUsername)
                    {
                        passwd+= x;
                    } else if (user_status == _AuthStatus::GotNeither)
                    {
                        uname+= x;
                    }
                }       

                std::cout << "Infomration of newly joined user: ";
                std::cout << "\n\tUsername: " << uname << "\n\tPassword: " << passwd << '\n'; 

                YAML::Node config = YAML::LoadFile("userdata.yml");
                if (config[uname]) {
                    if (config[uname]["password"]) {
                        if (config[uname]["password"].as<std::string>() == passwd) {
                            __logl("success");
                            if (!nick(uname, client, iterator)) return;
                        } else {
                            disconnectClient(client, iterator, DisconnectReason::DisconnectKick);
                            __logl("Remote " << client->getRemoteAddress() << ":" << client->getRemotePort() << " sent bad authentication request.");
                            return;
                        }
                    } else {
                        __logl("Server did not find password for this username");\
                        disconnectClient(client, iterator, DisconnectReason::DisconnectKick);
                        __logl("Remote " << client->getRemoteAddress() << ":" << client->getRemotePort() << " sent bad authentication request.");
                        return;
                    }
                } else {
                    __logl("Server did not find this username");
                    disconnectClient(client, iterator, DisconnectReason::DisconnectKick);
                    __logl("Remote " << client->getRemoteAddress() << ":" << client->getRemotePort() << " sent bad authentication request.");
                    return;
                }
                
                client_authenticated_array[iterator] = AuthStatus::KeyReceived;
                
                std::string s{"spb"}; // REQUESTING SEND PUBLIC KEY
                std::cout << iterator << std::endl;
                send_unencrypted(s.c_str(),s.length(), client); 
                return; 
            } 
            else if (client_authenticated_array[iterator] == AuthStatus::KeyReceived) {
                crypt->pushNewClientKey(BigInt(std::string(received_data)));
                //std::cout << "Got publickeyof(" << iterator << ") == " << received_data << std::endl;

                auto a = crypt->RSA_encrypt(iterator, SERVER_KEY);
                std::string server_key_str{a.begin(), a.end()};
                std::cout << server_key_str.length() << std::endl;
                
                send_unencrypted(server_key_str.c_str(), server_key_str.length(), client);
                
                client_authenticated_array[iterator] = AuthStatus::Done;
                
                return;
            }

            secure_vector<uint8_t> vec{};
            vec.resize(received_bytes);
            std::transform(received_data, received_data + received_bytes, vec.begin(), [](char v) {return static_cast<uint8_t>(v);});
            
            auto x = crypt->decrypt(vec, SERVER_KEY.c_str());
            std::string sent_data{x["msg"].begin(), x["msg"].end()};

            
            if (!check(sent_data.data())) {
                if (clientid_array[iterator] != "\x96") error("Invalid send from " << clientid_array[iterator]);
                else error("Invalid send from " << client->getRemoteAddress() << ":" << client->getRemotePort());

                __logl("Remote " << client->getRemoteAddress() << ":" << client->getRemotePort() << " sent an illegal character");
                disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
                return;
            }

            if (sent_data.length() >= 4096) {
                
                if (clientid_array[iterator] != "\x96") error("Invalid send from " << clientid_array[iterator]); 
                else error("Invalid send from " << client->getRemoteAddress() << ":" << client->getRemotePort());

                __logl("Remote " << client->getRemoteAddress() << ":" << client->getRemotePort() << " sent too many charachters");
                disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
                return;
            }
            
            std::stringstream sending_string;      


            // Handle receives
            if ((strncmp("0",sent_data.c_str(),1) == 0)) {
                    if(!handleSend(sent_data,sending_string,client,iterator)) return;
            } else
            if ((strncmp("3",sent_data.c_str(),1) == 0)) {
                handleNick(sent_data,client,iterator);
                
                return;
                
            } else
            if ((strncmp("4",sent_data.c_str(),1) == 0)) {
                __logl("[INFO] Denied request.");
            } else
            if ((strncmp("5",sent_data.c_str(),1) == 0)) {
                handleCommand(sent_data,client,iterator);
                return;
            }
            else {
                disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
                return;
            }
        
        if (client_message_interval[iterator]->getElapsedTime().asMilliseconds() >= 300) {

            std::string convertedSS = sending_string.str();
            char char_array[256] = {'\0'};
            strcpy(char_array,convertedSS.c_str());
            broadcast(char_array, client->getRemoteAddress(), client->getRemotePort());
                if (clientid_array[iterator] != "\x96") __logl(clientid_array[iterator] << " sent '" << sent_data << "'"); 
                else __logl(client->getRemoteAddress().toString() << ":" << client->getRemotePort() << " sent '" << sent_data << "'"); 
            
            
            updateObjs();
            chatSend_str = "";

            client_message_interval[iterator]->restart();

        } else {
            std::string message = "6\x01You are sending messages too fast. Cool down.";
            send(message.c_str(),message.length() + 1, client);
            // lastQueuer = client;
            // send_queue.push_back(sending_string.str());          
        }
    }
}

void ServerNetwork::handleCommand(std::string  received_data,sf::TcpSocket* client, size_t iterator) {
    received_data.erase(0, 2);
    // if (strcmp(message,"list") == 0) {
    //     std::stringstream list_all;
    //     list_all << "5\x01\nList of online users: ";
    //     for (auto x: clientid_array) {
    //         if (x != "\x96") list_all << x << " ";
    //     }
    //     send(list_all.str().c_str(),list_all.str().length() + 1,client);
    // } else
    // if (strcmp(message,"getpos") == 0){
    //     std::stringstream pos;
    //     pos << "5\x01 Current pos: " << std::to_string(iterator);
    //     send(pos.str().c_str(),pos.str().length() + 1, client);
    // } else
    // if (strcmp(message,"help") == 0) {
    //     std::stringstream say;
    //     say << "5\x01You can currently reach these commands: \n\thelp \n\tgetpos \n\tlist";
    //     send(say.str().c_str(),say.str().length() +1, client);
    // }
    // if (strcmp(message,"hidden_command") == 0) {
    //     std::stringstream say;
    //     say << "5\x01 Damn you so quirky girl";
    //     send(say.str().c_str(),say.str().length() +1, client);
    //     disconnectClient(client,iterator,DisconnectReason::DisconnectLeave);
    // }
    __logl(clientid_array[iterator] << " issued command: " << received_data);
    cmd_executor->iterate(received_data, client, iterator);
  
}
bool ServerNetwork::handleSend(std::string& received_data,std::stringstream& sending_string,sf::TcpSocket* client, size_t iterator) {
    received_data.erase(0, 1);
    chatSend_str = received_data.c_str();

    bool invld_res = (chatSend_str.find_first_not_of(" \t\n\v\f\r") == std::string::npos);

    if (invld_res) {
        std::string message = "6\x01Stop sending empty shit.";
        send(message.c_str(),message.length() + 1, client);
        return false;
    }

    if (clientid_array[iterator] != "\x96") {
        sending_string << "0" << "\x01" << received_data << "\x01" << clientid_array[iterator];

    }
    else {
        disconnectClient(client,iterator,DisconnectReason::DisconnectUnnamed);
        return false;
    }
    // Normal
    return true;
}

bool ServerNetwork::nick(std::string received_data, sf::TcpSocket* client, size_t iterator) {
     for(auto i : clientid_array) {
        if (received_data == i) {        
            disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
            warn("An attempt of connection with an already online alias was blocked.");
            return false;
        }
    }
    if (received_data[2] == '\0') {
        disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
        warn("Previous remote's alias was too short. (kicked)");
        return false;
    }

    std::regex r("^[a-zA-Z0-9_]*$");
    if (!(std::regex_match(received_data, r))) {
        disconnectClient(client,iterator,DisconnectReason::DisconnectKick);
        warn("Previous remote masked themselves to a non-alphanumeric alias. (kicked)");
        return false;
    }

    char char_array[256] = {'\0'};
    strcpy(char_array,received_data.c_str());
    
    chatSend_str = received_data;
    bool invld_res = (chatSend_str.find_first_not_of(" \t\n\v\f\r") == std::string::npos);

    if (invld_res) {
        std::string message = "6\x01Stop sending empty stuff.";
        send(message.c_str(),message.length() + 1, client);
        return false;
    }


    std::stringstream joinMessageStream;
    joinMessageStream << "1\x01" << char_array << " connected.";

    broadcast(joinMessageStream.str().c_str(),client->getRemoteAddress(), client->getRemotePort()); 
    __logl(client->getRemoteAddress().toString() << ":" << std::to_string(client->getRemotePort()) << ": Remote entry is masked to " << received_data );
    clientid_array[iterator] = received_data;

    return true;
}

/// \brief Handles nicks
bool ServerNetwork::handleNick(std::string received_data,sf::TcpSocket* client, size_t iterator) {
    received_data.erase(0, 2);
    std::cout << "nick is: " << received_data << std::endl;
    // Check whether name already exists
   
    return nick(received_data, client, iterator);
}   

void ServerNetwork::handleRequestedConsole(sf::TcpSocket* sock,size_t iterpos) {
    __log("op " << sock->getRemoteAddress().toString() << ":" << sock->getRemotePort() << " (AKA:" << clientid_array[iterpos] << "): Remote is requesting to become an operator. [Y/N]");
    char say;
    std::cin >> say;
    switch (say) {
        case 'Y' | 'y': 
            __logl("Opped remote.");
            client_op_array[iterpos] = true;
        break;
        case 'N' | 'n':
            __logl("Denied");
            return;
        break;
        default:
            __logl("wrong input.");
            return;
    }      


}

void ServerNetwork::updateObjs() {
    for (auto x : objs) {
 
    }
}

void ServerNetwork::manage() {
    while (true) {
        for (size_t iterator = 0; iterator < client_array.size(); iterator++) 
            receive(client_array[iterator], iterator);
        
        std::this_thread::sleep_for((std::chrono::milliseconds)100);
    }

}

void ServerNetwork::sendQueuedMessage(std::vector<std::string>& vec, sf::TcpSocket* client) {
    while (true) {
        if (client != nullptr) {
            if (!vec.empty()) {
                sf::Clock* waiter = new sf::Clock();
                for (size_t i;i < vec.size();i++) {
                    while (waiter->getElapsedTime().asMilliseconds() < 350) {
                    }
                    const char* tosend;
                    tosend = std::string(vec[i]).c_str();
                    __logl(tosend);
                    broadcast(tosend, client->getRemoteAddress(),client->getRemotePort());
                    vec.erase(vec.begin() + i);
                }
            }
        }
        std::this_thread::sleep_for((std::chrono::milliseconds)50);
    }
}

void ServerNetwork::run() {
    std::thread connetion_thread(&ServerNetwork::connectClients, this, &client_array);
    std::thread queue_thread(&ServerNetwork::sendQueuedMessage, this, std::ref(send_queue),lastQueuer);

    // Manage thread for receiving and sending
    manage();
}


#ifdef __unix__  

void openDLL() {
    
}

#elif defined(_WIN32) || defined(WIN32)    

void openDLL() {
    
}

#endif
