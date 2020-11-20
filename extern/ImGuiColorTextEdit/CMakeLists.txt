add_library(
  ImGuiColorTextEdit
  TextEditor.h TextEditor.cpp
)

target_include_directories(
  ImGuiColorTextEdit
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  ImGuiColorTextEdit
  PUBLIC
    DearImGui
)