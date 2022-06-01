cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_)
project(openSIMP VERSION 0.1.0)
add_compile_options(-g -static  )


set(THREADS_PREFER_PTHREAD_FLAG ON)

# required packages
find_package(Threads REQUIRED)

# Sources 
set(SOURCES
    ${CMAKE_SOURCE_DIR}/server/src/server.cpp
    ${CMAKE_SOURCE_DIR}/server/src/servernetwork/servernetwork.cpp
    ${CMAKE_SOURCE_DIR}/server/src/cryptography/cryptography.cpp
    ${CMAKE_SOURCE_DIR}/server/src/commands/command.cpp
    ${CMAKE_SOURCE_DIR}/server/src/commands/executor.cpp
    ${CMAKE_SOURCE_DIR}/server/src/commands/lexer.cpp
)

add_executable(openSIMPserver ${SOURCES})
target_include_directories(openSIMPserver PRIVATE include ${SFML_LIB}/include ${BOTAN_LIB}/include/botan-3 ${YAML_CPP_INCLUDE_DIR})
link_directories(${SFML_LIB} ${BOTAN_LIB})

target_compile_features(openSIMPserver PRIVATE cxx_std_17)

# do not add flag -static yet. SFML is not built from source.
target_link_libraries(openSIMPserver sfml-graphics sfml-window sfml-system sfml-network botan-3 dl rt ${CMAKE_THREAD_LIBS_INIT} yaml-cpp)


if (WIN32)
    set(SHARED_FORMAT .dll)
elseif(UNIX)
    set(SHARED_FORMAT .so.0)
endif()

add_custom_command(TARGET openSIMPserver POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${BOTAN_LIB}/src/BOTAN-build/libbotan-3.so.0"      # <--this is in-file
        $<TARGET_FILE_DIR:openSIMPserver>)                 # <--this is out-file path