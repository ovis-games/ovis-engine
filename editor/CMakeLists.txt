add_executable(
  ovis-editor
  src/main.cpp
  include/ovis/editor/scripting.hpp src/scripting.cpp
  include/ovis/editor/asset_provider.hpp src/asset_provider.cpp
  include/ovis/editor/editor_viewport.hpp src/editor_viewport.cpp
  include/ovis/editor/viewport_controller.hpp src/viewport_controller.cpp
  include/ovis/editor/camera_controller.hpp src/camera_controller.cpp
  include/ovis/editor/object_selection_controller.hpp src/object_selection_controller.cpp
  include/ovis/editor/transformation_tools_controller.hpp src/transformation_tools_controller.cpp

  include/ovis/editor/render_passes/selected_object_bounding_box.hpp src/render_passes/selected_object_bounding_box.cpp
  include/ovis/editor/render_passes/transformation_tools_renderer.hpp src/render_passes/transformation_tools_renderer.cpp
)

target_include_directories(
  ovis-editor
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(
  ovis-editor
  PUBLIC
    "SHELL:-s --bind"
)
target_link_options(
  ovis-editor
  PUBLIC
    "SHELL:-s FORCE_FILESYSTEM=1"
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s EXPORT_NAME=createEditorModule"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=['FS','ccall','cwrap']"
)

target_link_libraries(
  ovis-editor
  PUBLIC
    ovis::emscripten
)

target_include_assets(ovis-editor)

install(
  TARGETS ovis-editor
  RUNTIME DESTINATION .
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ovis-editor.data ${CMAKE_CURRENT_BINARY_DIR}/ovis-editor.wasm
  DESTINATION .
)
