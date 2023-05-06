cmake_minimum_required(VERSION 3.0)
project(client)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "D:/libs/boost_1_80_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "D:/libs/boost_1_80_0/stage/lib")

find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS system) 
include_directories(${OpenCV_INCLUDE-DIRECTORIES})
include_directories(${Boost_INCLUDE_DIRS})
add_executable(client client.cpp)

target_link_libraries(client ${OpenCV_LIBS})
target_link_libraries(client ${Boost_LIBRARIES})