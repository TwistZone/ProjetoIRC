cmake_minimum_required(VERSION 3.15)
project(IRCproj C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wall -Wextra -pthread")

add_executable(Server server.c headers.h)
add_executable(Client client.c)
add_executable(Proxy proxy.c)