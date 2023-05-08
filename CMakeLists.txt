cmake_minimum_required(VERSION 3.20)
project(ovis)

option(OVIS_BUILD_TESTS "Build the Ovis unit tests" ON)
option(OVIS_BUILD_DOCS "Build the Ovis documentation" ON)

include(cmake/emscripten.cmake)
include(cmake/assets.cmake)
include(cmake/schemas.cmake)
include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

if (OVIS_BUILD_TESTS)
  enable_testing()
endif ()

CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:nlohmann/json@3.11.2")
CPMAddPackage("gh:catchorg/Catch2@3.1.1")
CPMAddPackage("gh:ovis-games/stb-cmake-wrapper@0.1")
if (NOT OVIS_EMSCRIPTEN)
  # We use the built-in version of SDL when using emscripten
  CPMAddPackage("gh:libsdl-org/SDL#release-2.24.1")
endif ()

find_program(
  NODE_EXECUTABLE
  node
  DOC "Node.js executable for running tests"
)

find_program(
  QUICKTYPE_EXECUTABLE
  quicktype
  DOC "Quicktype is used to generate C++ code based on JSON schemas."
)
if (NOT QUICKTYPE_EXECUTABLE)
  message(SEND_ERROR "Failed to find quicktime executable.\nPlease install it using `npm install -g quicktype`.")
endif ()

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
add_subdirectory(sdl)

if (OVIS_EMSCRIPTEN)
  add_subdirectory(emscripten)
  add_subdirectory(editor)
endif ()
