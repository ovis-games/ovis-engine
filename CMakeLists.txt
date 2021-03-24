cmake_minimum_required(VERSION 3.12)
project(ovis)

option(OVIS_BUILD_TESTS "Build the Ovis unit tests" ON)
option(OVIS_BUILD_DOCS "Build the Ovis documentation" ON)

include(cmake/emscripten.cmake)
include(cmake/assets.cmake)
include(cmake/doc.cmake)
include(cmake/doc-tests.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

if (OVIS_BUILD_TESTS)
  enable_testing()
  find_program(
    NODE_EXECUTABLE
    node
    DOC "Node.js executable for running tests"
  )
endif ()

set(OVIS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(extern)
add_subdirectory(engine)
add_subdirectory(editor)
# add_subdirectory(player)