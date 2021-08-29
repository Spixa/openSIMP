#!/bin/bash
echo "Compiling server and client"
g++ -I./server/include/ server/src/commands/command.cpp server/src/commands/executor.cpp server/src/commands/lexer.cpp server/src/servernetwork/servernetwork.cpp server/src/server.cpp -o simpserver -lpthread -lsfml-system -lsfml-network
g++ clients/cpp/ClientNetwork.cpp clients/cpp/ClientNetwork.h clients/cpp/client.cpp clients/cpp/Utils.hpp -o simpclient -lpthread -lsfml-system -lsfml-network 
