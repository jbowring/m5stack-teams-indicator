cmake_minimum_required(VERSION 3.20)
project(HostGraphClient)

set(CMAKE_CXX_STANDARD 14)

add_executable(HostGraphClient main.cpp cJSON.c cJSON.h Auth.cpp Auth.h Graph.cpp Graph.h)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(HostGraphClient ${CURL_LIBRARIES})