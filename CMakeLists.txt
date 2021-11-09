cmake_minimum_required(VERSION 3.17)
include("cmake/HunterGate.cmake")
HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.297.tar.gz"
        SHA1 "3319fe6a3b08090df7df98dee75134d68e2ef5a3"
)
project(textcorrector)

set(CMAKE_CXX_STANDARD 20)
find_package(Catch2 CONFIG REQUIRED)
add_subdirectory(3rd)
add_subdirectory(tc)