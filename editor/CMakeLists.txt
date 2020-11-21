cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
add_definitions("-DOVIS_EMSCRIPTEN=1")

if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT DEFINED CMAKE_BUILD_TYPE)
  message(STATUS "Ovis Editor: Building in debug mode, disable optimziations.")
  add_compile_options(
    -g
    -O0
    "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    "SHELL:-s DEMANGLE_SUPPORT=1"
  )
  add_link_options(
    -g
    -O0
    "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    "SHELL:-s DEMANGLE_SUPPORT=1"
  )
else ()
  add_compile_options(
    -O3
  )
  add_link_options(
    -O3
  )
endif ()

add_executable(
  ovis-editor
  main.cpp
  editor_module.hpp editor_module.cpp
  editor_window.hpp editor_window.cpp
  editor_window_controller.hpp editor_window_controller.cpp
  editor_asset_library.hpp editor_asset_library.cpp
  loading_controller.hpp loading_controller.cpp
  
  windows/log_window.hpp windows/log_window.cpp
  windows/asset_viewer_window.hpp windows/asset_viewer_window.cpp
  windows/asset_editors/asset_editor.hpp windows/asset_editors/asset_editor.cpp
  windows/asset_editors/scene_editor.hpp windows/asset_editors/scene_editor.cpp
  windows/asset_editors/script_editor.hpp windows/asset_editors/script_editor.cpp
  windows/asset_editors/texture_editor.hpp windows/asset_editors/texture_editor.cpp
)

target_compile_options(
  ovis-editor
  PUBLIC
  "SHELL:-s FETCH=1"
)
target_link_options(
  ovis-editor
  PUBLIC
  "SHELL:-s FETCH=1"
)

add_custom_target(
  CopyIndexHtml
  ALL
  COMMAND cmake -E copy ${CMAKE_CURRENT_SOURCE_DIR}/index.html ${CMAKE_CURRENT_BINARY_DIR}/index.html
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/index.html
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/index.html
)

target_link_libraries(
  ovis-editor
  PUBLIC
    ovis::engine
    ovis::base
    ovis::rendering2d
    nlohmann_json
    ImGuiColorTextEdit
)

install(
  TARGETS ovis-editor
  RUNTIME DESTINATION .
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ovis-editor.data ${CMAKE_CURRENT_BINARY_DIR}/ovis-editor.wasm
  DESTINATION .
)