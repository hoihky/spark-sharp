cmake_minimum_required(VERSION 3.28)
project(SparkSharp VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to the Spark C++ engine repository (must expose SparkEngine target or prebuilt lib).
set(SPARK_ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../spark" CACHE PATH "Spark engine source root")

if(NOT EXISTS "${SPARK_ENGINE_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "SPARK_ENGINE_ROOT does not contain Spark engine: ${SPARK_ENGINE_ROOT}")
endif()

set(SPARK_BUILD_DEMO OFF CACHE BOOL "" FORCE)
set(SPARK_BUILD_GAMEPLAY ON CACHE BOOL "" FORCE)
set(SPARK_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPARK_BUILD_SPARK_EDITOR OFF CACHE BOOL "" FORCE)

if(NOT TARGET SparkEngine)
    add_subdirectory("${SPARK_ENGINE_ROOT}" "${CMAKE_BINARY_DIR}/_spark_engine" EXCLUDE_FROM_ALL)
endif()

option(SPARK_SHARP_BUILD_SCRIPT_HOST "Build SparkScriptHost (CoreCLR)" OFF)
add_subdirectory(native)
