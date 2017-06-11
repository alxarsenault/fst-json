cmake_minimum_required(VERSION 3.8.0)

# Check if we run cmake at root of workspace.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Source and build directories cannot be the same.")
endif ()

project(fstJsonLib)
set(FST_JSON_VERSION 0.0.1)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_EXAMPLES "Build with examples" ON)

# Use folder.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(FST_JSON_INCLUDE_DIRECTORY ${CMAKE_SOURCE_DIR}/include/)

# Debug output path.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Debug)

# Release output path.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Release)

# Set debug as default build type.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CONFIG_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Release)
else ()
    set(CONFIG_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_SYSTEM_PROCESSOR}/Debug)
endif ()

# Compiler flags.
set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS_RELEASE}")

add_definitions("-DRESOURCES_PATH=\"${CMAKE_SOURCE_DIR}/resources\"")

file(GLOB_RECURSE FST_JSON_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/fst/*.h)
file(GLOB FOLDER_FILE_FST_JSON_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/fst/*.h)

source_group("include" FILES ${FOLDER_FILE_FST_JSON_HEADER_FILES})

add_library(fstJson STATIC EXCLUDE_FROM_ALL ${FST_JSON_HEADER_FILES})
set_target_properties(fstJson PROPERTIES LINKER_LANGUAGE CXX)

# Examples.
if(BUILD_EXAMPLES)
	add_subdirectory(example/parse)
	add_subdirectory(example/read)
endif(BUILD_EXAMPLES)
