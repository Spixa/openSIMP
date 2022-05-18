#!/bin/bash
echo "Compiling server and client"
g++ ClientNetwork.cpp ClientNetwork.h PacketType.h client.cpp Utils.hpp -o susclient -lpthread -lsfml-system -lsfml-network
