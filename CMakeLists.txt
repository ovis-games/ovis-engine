cmake_minimum_required(VERSION 3.12)
project(ovis)

option(OVIS_BUILD_TESTS "Build the Ovis unit tests" ON)
option(OVIS_BUILD_DOCS "Build the Ovis documentation" ON)

include(cmake/emscripten.cmake)
include(cmake/assets.cmake)
include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

if (OVIS_BUILD_TESTS)
  enable_testing()
endif ()

find_program(
  NODE_EXECUTABLE
  node
  DOC "Node.js executable for running tests"
)

CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:nlohmann/json@3.11.2")
CPMAddPackage("gh:catchorg/Catch2@3.1.1")
CPMAddPackage("gh:ovis-games/stb-cmake-wrapper@0.1")
CPMAddPackage("gh:libsdl-org/SDL#release-2.24.1")

add_subdirectory(utils)
add_subdirectory(vm)
add_subdirectory(core)
add_subdirectory(test)
add_subdirectory(graphics)
add_subdirectory(rendering)
add_subdirectory(input)
add_subdirectory(networking)
# add_subdirectory(audio)
add_subdirectory(rendering2d)
add_subdirectory(application)

if (OVIS_EMSCRIPTEN)
  add_subdirectory(editor)
endif ()
