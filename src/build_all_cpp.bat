@echo off
echo Warning: This build file compiles assuming you have SFMFL files installed at C:\SFML
echo You need SFML and MinGW i686-w64-mingw32 7.3 Dwarf
echo Compiling Server and client
del susserver.exe
del susclient.exe
echo =======================
echo    Compiling server
echo =======================
g++ -I./server/include/ server/src/commands/command.cpp server/src/commands/executor.cpp server/src/commands/lexer.cpp server/src/cryptography/cryptography.cpp server/src/servernetwork/servernetwork.cpp server/src/server.cpp -o server -lpthread -lsfml-system -lsfml-network -lbotan-3 -I./server/deps/include -L./server/deps/lib -LC:\SFML\lib -IC:\SFML\include -std=c++17
echo =======================
echo    Compiling client
echo =======================
g++ clients/cpp/ClientNetwork.cpp clients/cpp/ClientNetwork.h clients/cpp/client.cpp clients/cpp/Utils.hpp -o susclient -lpthread -lsfml-system -lsfml-network -LC:\SFML\lib -IC:\SFML\include
echo Process done

