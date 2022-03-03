cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
project(openSIMP VERSION 0.1.0)

## If you want to link SFML statically
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(THREADS_PREFER_PTHREAD_FLAG ON)

# required packages
find_package(Botan REQUIRED)
find_package(Threads REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

# Fall-back for windows
if(WIN32)
    set(SFML_STATIC_LIBRARIES TRUE)
    set(SFML_DIR "./ext/SFML/lib/cmake/SFML")
endif()

# Sources 
set(SOURCES
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/cryptography.cpp
)

add_executable(client ${SOURCES})
target_include_directories(client PRIVATE include ${BOTAN_INCLUDE_DIR})

# do not add flag -static yet. SFML is not built from source.
target_link_libraries(client sfml-graphics sfml-window sfml-system sfml-network botan-2 ${CMAKE_THREAD_LIBS_INIT})