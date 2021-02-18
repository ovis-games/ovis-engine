#pragma once

#include <imgui.h>

#include <ovis/graphics/texture2d.hpp>

namespace ImGui {

inline bool TextureButton(ovis::Texture2D* texture, const ImVec2& size = ImVec2(0,0), const ImVec2& uv0 = ImVec2(0, 0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1)) {
  return ImageButton(
      texture, size.x == 0 && size.y == 0 ? ImVec2(texture->description().width, texture->description().height) : size,
      uv0, uv1, frame_padding, bg_col, tint_col);
}

}  // namespace ImGui
