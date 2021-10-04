if (OVIS_EMSCRIPTEN)

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

endif (OVIS_EMSCRIPTEN)

add_executable(
  ovis-editor
  main.cpp
  editor_window.hpp editor_window.cpp
  editor_asset_library.hpp editor_asset_library.cpp
  clipboard.hpp clipboard.cpp
  editor_viewport.hpp editor_viewport.cpp

  imgui_extensions/input_json.hpp imgui_extensions/input_json.cpp
  imgui_extensions/input_math.hpp imgui_extensions/input_math.cpp
  imgui_extensions/input_asset.hpp imgui_extensions/input_asset.cpp
  imgui_extensions/splitter.hpp imgui_extensions/splitter.cpp
  imgui_extensions/auto_complete_input.hpp imgui_extensions/auto_complete_input.cpp

  windows/log_window.hpp windows/log_window.cpp
  windows/asset_viewer_window.hpp windows/asset_viewer_window.cpp
  windows/inspector_window.hpp windows/inspector_window.cpp
  windows/dockspace_window.hpp windows/dockspace_window.cpp
  windows/toolbar.hpp windows/toolbar.cpp
  windows/toast_window.hpp windows/toast_window.cpp
  windows/profiling_window.hpp windows/profiling_window.cpp
  
  windows/modal/modal_window.hpp windows/modal/modal_window.cpp
  windows/modal/loading_window.hpp windows/modal/loading_window.cpp
  windows/modal/packaging_window.hpp windows/modal/packaging_window.cpp
  windows/modal/message_box_window.hpp windows/modal/message_box_window.cpp

  windows/asset_editors/asset_editor.hpp windows/asset_editors/asset_editor.cpp
  windows/asset_editors/settings_editor.hpp windows/asset_editors/settings_editor.cpp
  windows/asset_editors/script_editor.hpp windows/asset_editors/script_editor.cpp
  windows/asset_editors/script_library_editor.hpp windows/asset_editors/script_library_editor.cpp
  windows/asset_editors/visual_script_editor.hpp windows/asset_editors/visual_script_editor.cpp
  windows/asset_editors/texture_editor.hpp windows/asset_editors/texture_editor.cpp
  windows/asset_editors/scene_view_editor.hpp windows/asset_editors/scene_view_editor.cpp
  windows/asset_editors/scene_editor.hpp windows/asset_editors/scene_editor.cpp
  windows/asset_editors/scene_object_editor.hpp windows/asset_editors/scene_object_editor.cpp

  windows/asset_editors/scene_editor/scene_editor_render_pass.hpp windows/asset_editors/scene_editor/scene_editor_render_pass.cpp

  windows/asset_editors/editing_controllers/editor_controller.hpp windows/asset_editors/editing_controllers/editor_controller.cpp
  windows/asset_editors/editing_controllers/editor_camera_controller.hpp windows/asset_editors/editing_controllers/editor_camera_controller.cpp
  windows/asset_editors/editing_controllers/object_selection_controller.hpp windows/asset_editors/editing_controllers/object_selection_controller.cpp
  windows/asset_editors/editing_controllers/transformation_tools_controller.hpp windows/asset_editors/editing_controllers/transformation_tools_controller.cpp
  windows/asset_editors/editing_controllers/physics2d_shape_controller.hpp windows/asset_editors/editing_controllers/physics2d_shape_controller.cpp

  windows/asset_editors/editor_render_passes/editor_render_pass.hpp windows/asset_editors/editor_render_passes/editor_render_pass.cpp
  windows/asset_editors/editor_render_passes/selected_object_bounding_box.hpp windows/asset_editors/editor_render_passes/selected_object_bounding_box.cpp
  windows/asset_editors/editor_render_passes/transformation_tools_renderer.hpp windows/asset_editors/editor_render_passes/transformation_tools_renderer.cpp
  windows/asset_editors/editor_render_passes/physics2d_shape_renderer.hpp windows/asset_editors/editor_render_passes/physics2d_shape_renderer.cpp

  windows/asset_importers/asset_importer.hpp windows/asset_importers/asset_importer.cpp
)

if (OVIS_EMSCRIPTEN)
target_link_options(
  ovis-editor
  PUBLIC
    "SHELL:-s FORCE_FILESYSTEM=1"
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s EXPORT_NAME=createEditorModule"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=['FS','ccall','cwrap']"
)
endif ()

target_add_assets(
  ovis-editor

  assets/line.shader.frag
  assets/line.shader.vert
  assets/FontAwesomeSolid.font.otf
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
    ovis::application
    nlohmann_json
    ImGuiColorTextEdit
    stb
    microtar
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
