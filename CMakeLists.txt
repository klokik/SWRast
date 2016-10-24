cmake_minimum_required(VERSION 2.8)

project(SWRast)

find_package(OpenCV REQUIRED)

add_executable(start start.cc)

target_link_libraries(DisplayImage ${OpenCV_LIBS})