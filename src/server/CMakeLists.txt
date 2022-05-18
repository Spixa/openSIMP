cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_)
project(openSIMP VERSION 0.1.0)
add_compile_options(-g)

## If you want to link SFML statically
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(THREADS_PREFER_PTHREAD_FLAG ON)

# required packages
find_package(Botan REQUIRED)
find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

# Fall-back for windows 
if(WIN32)
    set(SFML_STATIC_LIBRARIES TRUE)
    set(SFML_DIR "./ext/SFML/lib/cmake/SFML")
endif()

# Sources 
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/server.cpp
    ${CMAKE_SOURCE_DIR}/src/servernetwork/servernetwork.cpp
    ${CMAKE_SOURCE_DIR}/src/cryptography/cryptography.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/command.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/executor.cpp
    ${CMAKE_SOURCE_DIR}/src/commands/lexer.cpp
)

add_executable(openSIMPserver ${SOURCES})
target_include_directories(openSIMPserver PRIVATE include ${BOTAN_INCLUDE_DIR} ${YAML_CPP_INCLUDE_DIR})
target_compile_features(openSIMPserver PRIVATE cxx_std_17)

# do not add flag -static yet. SFML is not built from source.
target_link_libraries(openSIMPserver sfml-graphics sfml-window sfml-system sfml-network botan-2 ${CMAKE_THREAD_LIBS_INIT} ${YAML_CPP_LIBRARIES})