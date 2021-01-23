#include "texture_editor.hpp"

#include "../../editor_window.hpp"
#include <fstream>
#include <regex>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/file.hpp>
#include <ovis/core/range.hpp>
#include <ovis/engine/engine.hpp>
#include <ovis/engine/lua.hpp>

namespace ove {

TextureEditor::TextureEditor(const std::string& texture_id) : AssetEditor(texture_id), action_history_(this) {
  // texture_ = EditorWindow::instance()->resource_manager()->Load<ovis::Texture2D>()
  texture_ = ovis::LoadTexture2D(ovis::GetApplicationAssetLibrary(), texture_id, EditorWindow::instance()->context());
  description_ = texture_->description();

  SetFlags(ImGuiWindowFlags_HorizontalScrollbar);
}

void TextureEditor::DrawContent() {
  if (ImGui::GetIO().KeyCtrl) {
    scale_ += ImGui::GetIO().MouseWheel * 0.1f;
  }
  ImVec2 image_size = { description_.width * scale_, description_.height * scale_ };
  ImGui::Image(texture_.get(), image_size);
}

void TextureEditor::DrawInspectorContent() {
  int width = description_.width;
  ImGui::InputInt("Width", &width, 0, 0, ImGuiInputTextFlags_ReadOnly);

  int height = description_.height;
  ImGui::InputInt("Height", &height, 0, 0, ImGuiInputTextFlags_ReadOnly);

  float scale = scale_ * 100.0f;
  if (ImGui::InputFloat("Scaling", &scale, 0.0f, 0.0f, "%.0f%%")) {
    scale_ = scale / 100.0f;
  }
}

void TextureEditor::Save() {}

}  // namespace ove