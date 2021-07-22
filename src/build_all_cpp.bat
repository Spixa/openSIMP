@echo off
echo Warning: This build file compiles assuming you have SFMFL files installed at C:\SFML
echo You need SFML and MinGW i686-w64-mingw32 7.3 Dwarf
echo Compiling Server and client
echo =======================
echo    Compiling server
echo =======================
g++ server/ServerNetwork.cpp server/ServerNetwork.h server/PacketType.h server/server.cpp -o susserver -lpthread -lsfml-system -lsfml-network -LC:\SFML\lib -IC:\SFML\include
echo =======================
echo    Compiling client
echo =======================
g++ clients/cpp/ClientNetwork.cpp clients/cpp/ClientNetwork.h clients/cpp/PacketType.h clients/cpp/client.cpp clients/cpp/Utils.hpp -o susclient -lpthread -lsfml-system -lsfml-network -LC:\SFML\lib -IC:\SFML\include
echo Process done

