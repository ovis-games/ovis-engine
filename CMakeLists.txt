cmake_minimum_required(VERSION 3.12)
project(ovis)

option(OVIS_BUILD_TESTS "Build the Ovis unit tests" ON)
option(OVIS_BUILD_DOCS "Build the Ovis documentation" ON)
option(OVIS_BUILD_BENCHMARKS "Build the benchmarks" ON)

include(cmake/emscripten.cmake)
include(cmake/assets.cmake)
include(cmake/module.cmake)
include(cmake/doc.cmake)
include(cmake/doc-tests.cmake)

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

set(OVIS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(extern)

add_subdirectory(utils)
add_subdirectory(vm)
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(rendering)
add_subdirectory(input)
add_subdirectory(networking)
# add_subdirectory(audio)
add_subdirectory(rendering2d)
# add_subdirectory(physics2d)
# add_subdirectory(imgui)
add_subdirectory(application)

if (OVIS_EMSCRIPTEN)
  add_subdirectory(player)
  add_subdirectory(editor_viewport)
  add_subdirectory(script_compiler)
endif ()
