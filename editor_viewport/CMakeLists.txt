add_executable(
  ovis-editor-viewport
  src/main.cpp
  include/ovis/editor_viewport/editor_viewport.hpp src/editor_viewport.cpp
  include/ovis/editor_viewport/viewport_controller.hpp src/viewport_controller.cpp
  include/ovis/editor_viewport/camera_controller.hpp src/camera_controller.cpp
  include/ovis/editor_viewport/object_selection_controller.hpp src/object_selection_controller.cpp

  include/ovis/editor_viewport/render_passes/selected_object_bounding_box.hpp src/render_passes/selected_object_bounding_box.cpp
)

target_include_directories(
  ovis-editor-viewport
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (OVIS_EMSCRIPTEN)
  target_compile_options(
    ovis-editor-viewport
    PUBLIC
    "SHELL:-s FETCH=1"
    "SHELL:-s --bind"
    "SHELL:-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0"
  )
  target_link_options(
    ovis-editor-viewport
    PUBLIC
      "SHELL:-s FETCH=1"
      "SHELL:-s --bind"
      "SHELL:-s FORCE_FILESYSTEM=1"
      "SHELL:-s MODULARIZE=1"
      "SHELL:-s ALLOW_MEMORY_GROWTH=1"
      "SHELL:-s EXPORT_NAME=createEditorViewportModule"
      "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=['FS','ccall','cwrap']"
      "SHELL:-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0"
  )
endif (OVIS_EMSCRIPTEN)

target_link_libraries(
  ovis-editor-viewport
  PUBLIC
    ovis::application
)

target_include_assets(ovis-editor-viewport)

install(
  TARGETS ovis-editor-viewport
  RUNTIME DESTINATION .
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ovis-editor-viewport.data ${CMAKE_CURRENT_BINARY_DIR}/ovis-editor-viewport.wasm
  DESTINATION .
)
