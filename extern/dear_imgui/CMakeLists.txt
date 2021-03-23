add_library(
  DearImGui

  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_widgets.cpp
  imgui_tables.cpp
  misc/cpp/imgui_stdlib.cpp
)

target_include_directories(
  DearImGui
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
)

target_link_libraries(
  DearImGui
  PUBLIC
    ovis::core
)
