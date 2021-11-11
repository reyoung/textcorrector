cmake_minimum_required(VERSION 3.17)
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.297.tar.gz"
        SHA1 "3319fe6a3b08090df7df98dee75134d68e2ef5a3"
)
project(textcorrector)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)
hunter_add_package(Catch)
find_package(Catch2 CONFIG REQUIRED)
hunter_add_package(asio)
find_package(asio CONFIG REQUIRED)
hunter_add_package(gflags)
find_package(gflags CONFIG REQUIRED)
hunter_add_package(glog)
find_package(glog CONFIG REQUIRED)
hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)
hunter_add_package(http-parser)
find_package(http-parser CONFIG REQUIRED)
hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)
add_subdirectory(3rd)
add_subdirectory(tc)