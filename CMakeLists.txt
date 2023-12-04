cmake_minimum_required(VERSION 3.26)
project(CallCentre)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_INCLUDE_LIBRARIES system thread)
set(BOOST_ENABLE_CMAKE ON)

include(FetchContent)

FetchContent_Declare(
        Boost
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG boost-1.82.0
        FIND_PACKAGE_ARGS 1.67 REQUIRED COMPONENTS system thread
)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/76bb2afb8b522d24496ad1c757a49784fbfa2e42.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Boost googletest)

message(STATUS "BOOST VERSION = ${Boost_VERSION}")

file(GLOB SRCS_CPP
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
list(REMOVE_ITEM SRCS_CPP "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_executable(CallCentre
        ${SRCS_CPP}
        ${CMAKE_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(CallCentre PRIVATE Boost::boost Boost::system Boost::thread)
target_include_directories(CallCentre PRIVATE ${CMAKE_SOURCE_DIR}/libs/include)


######################TESTS#######################
enable_testing()

file(GLOB all_test_SRCS
        "${CMAKE_SOURCE_DIR}/tests/*.cpp"
)

add_executable(CallCentreTest
        ${all_test_SRCS}
        ${SRCS_CPP}
)

target_include_directories(CallCentreTest PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/libs/include)

target_link_libraries(CallCentreTest
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
        Boost::boost Boost::system Boost::thread
)

include(GoogleTest)
gtest_discover_tests(CallCentreTest)
