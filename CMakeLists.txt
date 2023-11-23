cmake_minimum_required(VERSION 3.26)
project(CallCentre)

set(CMAKE_CXX_STANDARD 20)

add_executable(CallCentre
        src/server.cpp
        src/Log.cpp
)

add_executable(mytest test.cpp)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS} libs/include)