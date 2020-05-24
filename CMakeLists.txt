cmake_minimum_required(VERSION 3.13)

project(MFS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)


file(GLOB SOURCES_LIBCORE "libraries/core/*.cpp" "libraries/core/*.hpp")
message( "${SOURCES_LIBCORE}")
add_library(libcore ${SOURCES_LIBCORE})

file(GLOB SOURCES_LIBCLIENT "libraries/client/*.cpp" "libraries/client/*.hpp")
add_library(libclient ${SOURCES_LIBCLIENT} libraries/client/Handler.h)

file(GLOB SOURCES_LIBMANAGER "libraries/manager/*.cpp" "libraries/manager/*.hpp")
add_library(libmanager ${SOURCES_LIBMANAGER})


file(GLOB SOURCES_MANAGER "manager/*.cpp" "manager/*.hpp")
add_executable(manager ${SOURCES_MANAGER})
target_link_libraries(libmanager PUBLIC libcore)
target_link_libraries(manager PUBLIC libmanager)

file(GLOB SOURCES_CLIENT "client/*.cpp" "client/*.hpp")
add_executable(client ${SOURCES_CLIENT})
target_link_libraries(libclient PUBLIC libcore)
target_link_libraries(client PUBLIC libclient)

MESSAGE("adding tests")

file(GLOB TESTS "tests/*.cpp")

foreach(TEST ${TESTS})
    MESSAGE("adding ${TEST}")
    get_filename_component(NAME "test_${TEST}" NAME_WE)
    add_executable(${NAME} ${TEST})
    target_link_libraries(${NAME} PUBLIC libcore)
    target_link_libraries(${NAME} PUBLIC libclient)
    target_link_libraries(${NAME} PUBLIC libmanager)

endforeach()