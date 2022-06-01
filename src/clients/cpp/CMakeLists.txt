cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_)
project(openSIMP VERSION 0.1.0)
## If you want to link SFML statically
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)

# Sources 
set(SOURCES
    ${CMAKE_SOURCE_DIR}/clients/cpp/src/client.cpp
    ${CMAKE_SOURCE_DIR}/clients/cpp/src/cryptography.cpp
    ${CMAKE_SOURCE_DIR}/clients/cpp/src/ClientNetwork.cpp
    ${CMAKE_SOURCE_DIR}/clients/cpp/src/cryptography.cpp
)

add_executable(openSIMPclient ${SOURCES})
target_include_directories(openSIMPclient PRIVATE include ${SFML_LIB}/include ${BOTAN_LIB}/include/botan-3)
link_directories(${SFML_LIB} ${BOTAN_LIB})

target_compile_features(openSIMPclient PRIVATE cxx_std_17)

# do not add flag -static yet. SFML is not built from source.
target_link_libraries(openSIMPclient sfml-graphics sfml-window sfml-system sfml-network botan-3 dl rt ${CMAKE_THREAD_LIBS_INIT})

if (WIN32)
    set(SHARED_FORMAT .dll)
elseif(UNIX)
    set(SHARED_FORMAT .so.0)
endif()

add_custom_command(TARGET openSIMPclient POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${BOTAN_LIB}/src/BOTAN-build/libbotan-3${SHARED_FORMAT}"      # <--this is in-file
        $<TARGET_FILE_DIR:openSIMPclient>)                 # <--this is out-file path