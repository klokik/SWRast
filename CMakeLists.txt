cmake_minimum_required(VERSION 2.8)

project(SWRast)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "--std=c++14 -g ${CMAKE_CXX_FLAGS}")

find_package(OpenCV REQUIRED)

add_executable(start start.cc)

target_link_libraries(start ${OpenCV_LIBS})
