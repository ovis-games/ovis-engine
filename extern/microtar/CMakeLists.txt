add_library(
  microtar

  src/microtar.c
)

target_include_directories(
  microtar
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
