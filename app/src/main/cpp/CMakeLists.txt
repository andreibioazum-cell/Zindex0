cmake_minimum_required(VERSION 3.10)
project(jsaicraft)
add_library(jsaicraft SHARED main.cpp)
target_link_libraries(jsaicraft android log EGL GLESv3)
