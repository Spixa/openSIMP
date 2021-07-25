#!/bin/bash
echo "Compiling server and client"
g++ server/ServerNetwork.cpp server/ServerNetwork.h server/PacketType.h server/server.cpp -o susserver -lpthread -lsfml-system -lsfml-network
