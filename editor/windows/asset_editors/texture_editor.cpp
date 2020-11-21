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
}

void TextureEditor::Draw() {
  ImGui::Image(texture_.get(), {512, 512});
}

void TextureEditor::Save() {}

}  // namespace ove