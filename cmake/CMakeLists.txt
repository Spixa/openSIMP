cmake_minimum_required(VERSION 3.0.0)

project(openSIMP)

## If you want to link SFML statically


## In most cases better set in the CMake cache

if(WIN32) 
    set(SFML_STATIC_LIBRARIES TRUE)
    set(SFML_DIR "./SFML/lib/cmake/SFML")
elseif()

endif()



set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)



find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

add_executable(openSIMPserver ../src/server/server.cpp ../src/server/ServerNetwork.cpp ../src/server/ServerObject.cpp ../src/server/ChatHandler.cpp)
target_link_libraries(openSIMPserver sfml-graphics sfml-window sfml-system sfml-network ${CMAKE_THREAD_LIBS_INIT} -static)

add_executable(openSIMPclient ../src/clients/cpp/client.cpp ../src/clients/cpp/ClientNetwork.cpp)
target_link_libraries(openSIMPclient sfml-graphics sfml-window sfml-system sfml-network ${CMAKE_THREAD_LIBS_INIT} -static)