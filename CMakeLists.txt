cmake_minimum_required(VERSION 3.30.2)

set(ProjectName "StrategySim")

project(${ProjectName} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# build configurations
set(SUPPORTED_BUILD_TYPES Debug Release DebugRelease)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

list(FIND SUPPORTED_BUILD_TYPES "${CMAKE_BUILD_TYPE}" BUILD_TYPE_INDEX)
if(BUILD_TYPE_INDEX EQUAL -1)
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}. Supported build types: ${SUPPORTED_BUILD_TYPES}")
endif()

# common compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
# -g is implicitly set when using Debug, so don't put it here
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")

elseif(CMAKE_BUILD_TYPE STREQUAL "DebugRelease")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")

endif()

# source files
include_directories(includes)

file(GLOB SOURCES "src/*.cpp")

add_executable(${ProjectName} ${SOURCES})

# build configurations

