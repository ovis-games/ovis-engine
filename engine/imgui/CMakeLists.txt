add_library(
  ovis-imgui
  
  include/ovis/imgui/imgui_module.hpp src/imgui_module.cpp
  include/ovis/imgui/imgui_render_pass.hpp src/imgui_render_pass.cpp
  include/ovis/imgui/imgui_start_frame_controller.hpp src/imgui_start_frame_controller.cpp
  include/ovis/imgui/imgui_end_frame_controller.hpp src/imgui_end_frame_controller.cpp
)
add_library(ovis::imgui ALIAS ovis-imgui)

target_include_directories(
  ovis-imgui
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-imgui
  PUBLIC
    ovis::core
    ovis::rendering
    ovis::input
    DearImGui
)

target_add_assets(
  ovis-imgui

  assets/imgui.schema.json
)
