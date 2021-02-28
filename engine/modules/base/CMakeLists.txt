add_library(
  ovis-base
  
  include/ovis/base/base_module.hpp src/base_module.cpp
  include/ovis/base/clear_render_pass.hpp src/clear_render_pass.cpp
  include/ovis/base/imgui_render_pass.hpp src/imgui_render_pass.cpp
  include/ovis/base/imgui_scene_controller.hpp src/imgui_scene_controller.cpp
  include/ovis/base/transform_component.hpp src/transform_component.cpp
)
add_library(ovis::base ALIAS ovis-base)

target_include_directories(
  ovis-base
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-base
  PUBLIC
    ovis::engine
)

target_add_assets(
  ovis-base

  assets/base.schema.json
  assets/ui.shader.vert
  assets/ui.shader.frag
)
