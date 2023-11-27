cmake_minimum_required(VERSION 3.26)
project(CallCentre)

set(CMAKE_CXX_STANDARD 20)

add_executable(CallCentre
        src/main.cpp
        src/Log.cpp
        src/RandGen.cpp
        src/Listener.cpp
        src/Session.cpp
        src/Operators.cpp
)

add_executable(mytest test.cpp
        src/RandGen.cpp)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS} libs/include)