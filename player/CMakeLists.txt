add_executable(
  ovis-player
  include/ovis/player/loading_controller.hpp src/loading_controller.cpp
  src/main.cpp
)

target_include_directories(
  ovis-player
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (OVIS_EMSCRIPTEN)
  target_compile_options(
    ovis-player
    PUBLIC
    "SHELL:-s FETCH=1"
    "SHELL:-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0"
  )
  target_link_options(
    ovis-player
    PUBLIC
      "SHELL:-s FETCH=1"
      "SHELL:-s FORCE_FILESYSTEM=1"
      "SHELL:-s MODULARIZE=1"
      "SHELL:-s ALLOW_MEMORY_GROWTH=1"
      "SHELL:-s EXPORT_NAME=createPlayerModule"
      "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=['FS','ccall','cwrap']"
  )
endif (OVIS_EMSCRIPTEN)

target_link_libraries(
  ovis-player
  PUBLIC
    ovis::application
    microtar
)

install(
  TARGETS ovis-player
  RUNTIME DESTINATION .
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ovis-player.data ${CMAKE_CURRENT_BINARY_DIR}/ovis-player.wasm
  DESTINATION .
)
