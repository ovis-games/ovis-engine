#pragma once

#include <string_view>

#include <imgui.h>

#include <ovis/core/scene_controller.hpp>

namespace ovis {

class ImGuiStartFrameController : public SceneController {
  friend class ImGuiEndFrameController;
  friend class ImGuiRenderPass;
  friend class ImGuiWindow;

 public:
  static inline constexpr std::string_view Name() { return "ImGuiStartFrame"; }

  ImGuiStartFrameController();

  void Update(std::chrono::microseconds delta_time) override;
  void ProcessEvent(Event* event) override;

  ImFont* LoadFont(std::string_view asset, float size = 20.0f, std::vector<std::pair<ImWchar, ImWchar>> ranges = {});
  ImFont* GetFont(std::string_view name, float size = 20.0f);

 private:
  std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)> imgui_context_;

  // Indicates whether a mouse button was just pressed
  bool mouse_button_pressed_[5] = {false, false, false, false, false};
  bool frame_started_ = false;

  struct FontData {
    std::vector<std::byte> font_file;
    std::vector<ImWchar> glyph_ranges;
    ImFont* font;
  };
  std::map<std::string, FontData, std::less<>> fonts_;
  bool font_added_ = true; // Defaults to true, to add default font
  bool reload_font_atlas_ = false;
  unsigned char* font_atlas_pixels_ = nullptr;
  int font_atlas_width_ = 0;
  int font_atlas_height_ = 0;
};

}  // namespace ovis
