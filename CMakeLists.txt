cmake_minimum_required(VERSION 3.0.0)

cmake_minimum_required(VERSION 3.14)
project(openSIMPserver)

## If you want to link SFML statically
set(SFML_STATIC_LIBRARIES TRUE)

## In most cases better set in the CMake cache
set(SFML_DIR "./SFML/lib/cmake/SFML")

find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

add_executable(openSIMPserver src/server/server.cpp src/server/ServerNetwork.cpp src/server/ServerObject.cpp src/server/ChatHandler.cpp)
target_link_libraries(openSIMPserver sfml-graphics sfml-window sfml-system sfml-network)

add_executable(openSIMPclient src/clients/cpp/client.cpp src/clients/cpp/ClientNetwork.cpp)

target_link_libraries(openSIMPclient sfml-graphics sfml-window sfml-system sfml-network)