cmake_minimum_required (VERSION 2.6)
project (projply)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PROJ4 REQUIRED proj>=6)

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
set (projply_VERSION_MINOR 3)
set (projply_VERSION_PATCH 0)
set (projply_GIT_BRANCH "\"${GIT_BRANCH}\"")
set (projply_GIT_COMMIT "\"${GIT_COMMIT}\"")

configure_file (
  "${PROJECT_SOURCE_DIR}/projply.h.in"
  "${PROJECT_BINARY_DIR}/projply.h"
  )

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/tinyply/source")
include_directories(${PROJ4_INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(projply main.cpp projply.cpp ply_io.cpp)

target_compile_options(projply PUBLIC ${PROJ4_CFLAGS_OTHER})
target_link_libraries(projply tinyply ${PROJ4_LIBRARIES})


install(TARGETS projply DESTINATION /usr/bin)
