cmake_minimum_required(VERSION 3.3)

add_definitions("-std=c++11")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -W -Wall")
#file(GLOB COLL_FILES ${COMMON_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/collector/*)

add_executable(hwserver zmq_rep_server.cpp)
target_link_libraries(hwserver zmq)
