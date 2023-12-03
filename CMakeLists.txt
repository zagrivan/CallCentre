cmake_minimum_required(VERSION 3.26)
project(CallCentre)

set(CMAKE_CXX_STANDARD 20)

file(GLOB all_SRCS
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(CallCentre
        ${all_SRCS}
)

find_package(Boost)
target_include_directories(CallCentre PRIVATE ${Boost_INCLUDE_DIRS} libs/include)


######################TESTS#######################

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/76bb2afb8b522d24496ad1c757a49784fbfa2e42.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB all_test_SRCS
        "${CMAKE_SOURCE_DIR}/tests/*.cpp"
)

add_executable(CallCentreTest
        ${all_test_SRCS}
        src/IncomingCallsQueue.cpp
        src/RandGen.cpp
        src/Log.cpp
        src/Call.cpp
        src/handle_req_resp.cpp
)

target_include_directories(CallCentreTest PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests ${Boost_INCLUDE_DIRS} libs/include)

target_link_libraries(CallCentreTest
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(CallCentreTest)
