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

target_compile_options(
  ovis-player
  PUBLIC
  "SHELL:-s FETCH=1"
)
target_link_options(
  ovis-player
  PUBLIC
    "SHELL:-s FETCH=1"
    "SHELL:-s FORCE_FILESYSTEM=1"
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s EXPORT_NAME=createEditorModule"
    "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=['FS','ccall','cwrap']"
    "--preload-file ${CMAKE_BINARY_DIR}/assets@/ovis_assets"
)

target_link_libraries(
  ovis-player
  PUBLIC
    ovis::engine
    ovis::base
    ovis::rendering2d
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