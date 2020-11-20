add_library(
  ovis-base
  
  include/ovis/base/base_module.hpp src/base_module.cpp
  include/ovis/base/imgui_render_pass.hpp src/imgui_render_pass.cpp
  include/ovis/base/imgui_scene_controller.hpp src/imgui_scene_controller.cpp
  include/ovis/base/transform2d_component.hpp src/transform2d_component.cpp
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

add_custom_command(
  TARGET ovis-base
  PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
)

target_link_options(
 ovis-base
 PUBLIC
  --preload-file ${CMAKE_BINARY_DIR}/assets@/ovis_assets
)
