cmake_minimum_required(VERSION 3.0)

add_definitions("-Wno-deprecated-declarations")
add_definitions("-Wno-unused-parameter")
add_definitions("-Wno-unused-variable")
add_definitions("-Wno-unused-function")
add_definitions("-Wno-reorder")
add_definitions("-Wno-switch")
add_definitions("-Wno-conversion-null")
add_definitions("-Wno-sign-compare")

project(ProjLenaky CXX)

add_subdirectory(zmq_tutorial)
add_subdirectory(basic)
