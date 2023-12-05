cmake_minimum_required(VERSION 3.26)
project(CallCentre)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_INCLUDE_LIBRARIES system thread asio beast)
set(BOOST_ENABLE_CMAKE ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.82.0/boost-1.82.0.tar.xz
        URL_MD5 9ef03341798f1caf9ac910bb60cde9d6
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        USES_TERMINAL_DOWNLOAD TRUE 
        GIT_PROGRESS TRUE
        FIND_PACKAGE_ARGS 1.81
)


FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/76bb2afb8b522d24496ad1c757a49784fbfa2e42.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(Boost googletest)

file(GLOB SRCS_CPP
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
list(REMOVE_ITEM SRCS_CPP "${CMAKE_SOURCE_DIR}/src/main.cpp")

add_executable(CallCentre
        ${SRCS_CPP}
        ${CMAKE_SOURCE_DIR}/src/main.cpp
)

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
)


include(GoogleTest)
gtest_discover_tests(CallCentreTest)

# если существует таргет Boost::boost, то сработал find_package,  
# иначе сработал FetchContent, подключение происходит немного по другому
if (TARGET Boost::boost)
        message(STATUS "FIND_PACKAGE Boost::bost existing")
        target_link_libraries(CallCentreTest Boost::boost)
        target_link_libraries(CallCentre Boost::boost)
else()
        message(STATUS "FETCH_CONTENT")
        target_link_libraries(CallCentreTest Boost::system Boost::thread Boost::asio Boost::beast)
        target_link_libraries(CallCentre Boost::system Boost::thread Boost::asio Boost::beast)
endif()

message(STATUS "BOOST VERSION = ${Boost_VERSION}")
