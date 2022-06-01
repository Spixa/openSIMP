cmake_minimum_required (VERSION 3.5)
project (simp CXX)
add_compile_options(-static -Bstatic)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)


set(SFML_STATIC TRUE)
ExternalProject_Add(SFML 
		GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.5.1 
    		CMAKE_ARGS 
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DSFML_BUILD_GRAPHICS=OFF
			-DSFML_BUILD_WINDOW=OFF
			-DSFML_BUILD_AUDIO=OFF)
ExternalProject_Get_Property(SFML INSTALL_DIR)
set(SFML_LIB ${INSTALL_DIR})

ExternalProject_Add(YAMLCPP 
		GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
    		CMAKE_ARGS 
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
ExternalProject_Get_Property(YAMLCPP INSTALL_DIR)
set(YAMLCPP_LIB ${INSTALL_DIR})

ExternalProject_Add(BOTAN
    GIT_REPOSITORY https://github.com/randombit/botan.git
    CONFIGURE_COMMAND python ${CMAKE_SOURCE_DIR}/build/BOTAN-prefix/src/BOTAN/configure.py --prefix=.
    BUILD_COMMAND "make"
		UPDATE_COMMAND ""
)
ExternalProject_Get_Property(BOTAN INSTALL_DIR)
set(BOTAN_LIB ${INSTALL_DIR})

message(${BOTAN_LIB})
include_directories(${BOTAN_LIB}/src/BOTAN-build/include/botan-3 ${SFML_LIB}/include ${YAMLCPP_LIB}/include)
link_directories(${BOTAN_LIB}/lib ${SFML_LIB}/lib ${YAMLCPP_LIB}/lib)

add_subdirectory(clients/cpp)
add_subdirectory(server)
