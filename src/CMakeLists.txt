cmake_minimum_required (VERSION 2.6)
project (projply)

add_subdirectory(tinyply)

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

set (projply_VERSION_MAJOR 1)
set (projply_VERSION_MINOR 2)
set (projply_VERSION_PATCH 0)
set (projply_GIT_BRANCH "\"${GIT_BRANCH}\"")
set (projply_GIT_COMMIT "\"${GIT_COMMIT}\"")

configure_file (
  "${PROJECT_SOURCE_DIR}/projply.h.in"
  "${PROJECT_BINARY_DIR}/projply.h"
  )

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/tinyply/source")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#find_package(PCL 1.8 REQUIRED COMPONENTS common io)
#include_directories(${PCL_INCLUDE_DIRS})
#link_directories({${PCL_DEFINITIONS})

add_executable(projply main.cpp projply.cpp ply_io.cpp)

# ${PCL_COMMON_LIBRARIES} ${PCL_IO_LIBRARIES} adds too much unused symbols
#target_link_libraries(projply pcl_common pcl_io boost_system boost_filesystem proj)
target_link_libraries(projply tinyply proj)

install(TARGETS projply DESTINATION /usr/bin)
