#include "editor_window.hpp"

#include "windows/log_window.hpp"
#include "windows/asset_viewer_window.hpp"
#include "windows/inspector_window.hpp"
#include "windows/dockspace_window.hpp"

#include <emscripten/html5.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <ovis/core/log.hpp>
#include <ovis/engine/scene.hpp>
#include <ovis/engine/window.hpp>

namespace ove {

namespace {

ovis::WindowDescription CreateWindowDescription() {
  ovis::WindowDescription window_description;

  window_description.title = "Ovis Editor";
  window_description.resource_search_paths = {"/resources/", "/assets/"};
  window_description.scene_controllers = {"ImGui"};
  window_description.render_passes = {"ImGui"};

  double canvas_css_width;
  double canvas_css_height;
  if (emscripten_get_element_css_size("canvas", &canvas_css_width, &canvas_css_height) == EMSCRIPTEN_RESULT_SUCCESS) {
    window_description.width = static_cast<int>(canvas_css_width);
    window_description.height = static_cast<int>(canvas_css_height);
  }

  return window_description;
}

}  // namespace

EditorWindow* EditorWindow::instance_ = nullptr;

EditorWindow::EditorWindow() : ovis::Window(CreateWindowDescription()) {
  SDL_assert(instance_ == nullptr);
  instance_ = this;

  // Add them here, so the instance variable is set
  scene()->AddController("EditorWindowController");
  scene()->AddController("LoadingWindow");
  scene()->AddController(std::make_unique<DockspaceWindow>());
  scene()->AddController(std::make_unique<LogWindow>());
  scene()->AddController(std::make_unique<AssetViewerWindow>());
  scene()->AddController(std::make_unique<InspectorWindow>());

  SetUIStyle();
}

void EditorWindow::Update(std::chrono::microseconds delta_time) {
  double canvas_css_width;
  double canvas_css_height;
  int canvas_width;
  int canvas_height;
  if (emscripten_get_element_css_size("canvas", &canvas_css_width, &canvas_css_height) == EMSCRIPTEN_RESULT_SUCCESS &&
      emscripten_get_canvas_element_size("canvas", &canvas_width, &canvas_height) == EMSCRIPTEN_RESULT_SUCCESS &&
      (canvas_width != static_cast<int>(canvas_css_width) || canvas_height != static_cast<int>(canvas_css_height))) {
    ovis::LogV("css: {}x{} vs real: {}x{}", canvas_css_width, canvas_css_height, canvas_width, canvas_height);
    // emscripten_set_canvas_element_size("canvas",
    // static_cast<int>(canvas_css_width),
    // static_cast<int>(canvas_css_height));
    Resize(static_cast<int>(canvas_css_width), static_cast<int>(canvas_css_height));
  }

  Window::Update(delta_time);
}

void EditorWindow::SetUIStyle() {
  ImGuiStyle* style = &ImGui::GetStyle();
  ImVec4* colors = style->Colors;

  const auto uint32_to_imvec4 = [](uint32_t rgba) {
    return ImVec4(((rgba & 0xff000000) >> 24) / 255.0f, ((rgba & 0x00ff0000) >> 16) / 255.0f,
                  ((rgba & 0x0000ff00) >> 8) / 255.0f, ((rgba & 0x000000ff) >> 0) / 255.0f);
  };

  const auto rgba_to_imvec4 = [](uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  };

  const ImVec4 COLOR_TEXT = rgba_to_imvec4(255, 255, 255, 255);
  const ImVec4 COLOR_TEXT_DISABLED = rgba_to_imvec4(127, 127, 127, 255);
  const ImVec4 COLOR_SELECTED_TEXT_BACKGROUND = rgba_to_imvec4(72, 72, 72, 255);
  const ImVec4 COLOR_WINDOW_BACKGROUND = rgba_to_imvec4(48, 48, 48, 255);
  const ImVec4 COLOR_FRAME_WINDOW_BACKGROUND = rgba_to_imvec4(12, 12, 12, 255);
  const ImVec4 COLOR_HIGHLIGHT = rgba_to_imvec4(128, 128, 128, 255);
  const ImVec4 COLOR_BORDER = rgba_to_imvec4(128, 128, 128, 255);
  const ImVec4 COLOR_BORDER_SHADOW = rgba_to_imvec4(0, 0, 0, 255);
  const ImVec4 COLOR_ACTIVE_HEADER = rgba_to_imvec4(128, 128, 128, 255);
  const ImVec4 COLOR_BUTTON = rgba_to_imvec4(80, 80, 80, 255);
  const ImVec4 COLOR_TITLE_BACKGROUND = rgba_to_imvec4(72, 72, 72, 255);

  const ImVec4 COLOR_SEPARATOR_HOVERED = rgba_to_imvec4(255, 255, 255, 200);
  const ImVec4 COLOR_SEPARATOR_ACTIVE = rgba_to_imvec4(255, 255, 255, 255);

  colors[ImGuiCol_Text] = COLOR_TEXT;
  colors[ImGuiCol_TextDisabled] = COLOR_TEXT_DISABLED;
  colors[ImGuiCol_WindowBg] = COLOR_WINDOW_BACKGROUND;
  colors[ImGuiCol_ChildBg] = COLOR_WINDOW_BACKGROUND;
  colors[ImGuiCol_PopupBg] = COLOR_WINDOW_BACKGROUND;
  colors[ImGuiCol_Border] = COLOR_BORDER;
  colors[ImGuiCol_BorderShadow] = COLOR_BORDER_SHADOW;
  colors[ImGuiCol_FrameBg] = COLOR_FRAME_WINDOW_BACKGROUND;
  colors[ImGuiCol_FrameBgHovered] = COLOR_ACTIVE_HEADER;
  colors[ImGuiCol_FrameBgActive] = COLOR_HIGHLIGHT;
  colors[ImGuiCol_TitleBg] = COLOR_TITLE_BACKGROUND;
  colors[ImGuiCol_TitleBgActive] = COLOR_TITLE_BACKGROUND;
  colors[ImGuiCol_TitleBgCollapsed] = COLOR_TITLE_BACKGROUND;
  colors[ImGuiCol_MenuBarBg] = COLOR_WINDOW_BACKGROUND;
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark] = COLOR_TEXT;
  colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = COLOR_ACTIVE_HEADER;
  colors[ImGuiCol_Button] = COLOR_BUTTON;
  colors[ImGuiCol_ButtonHovered] = COLOR_ACTIVE_HEADER;
  colors[ImGuiCol_ButtonActive] = COLOR_HIGHLIGHT;
  colors[ImGuiCol_Header] = COLOR_WINDOW_BACKGROUND;
  colors[ImGuiCol_HeaderHovered] = COLOR_HIGHLIGHT;
  colors[ImGuiCol_HeaderActive] = COLOR_ACTIVE_HEADER;
  colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
  colors[ImGuiCol_SeparatorHovered] = COLOR_SEPARATOR_HOVERED;
  colors[ImGuiCol_SeparatorActive] = COLOR_SEPARATOR_ACTIVE;
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
  colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
  colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
  colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
  colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
  colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_HeaderActive];  // * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
  colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = COLOR_SELECTED_TEXT_BACKGROUND;
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight] = COLOR_ACTIVE_HEADER;
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

}  // namespace ove