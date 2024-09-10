cmake_minimum_required(VERSION 3.30.2)

set(ProjectName "StrategySim")

project(${ProjectName} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(includes)

file(GLOB SOURCES "src/*.cpp")

add_executable(${ProjectName} ${SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")