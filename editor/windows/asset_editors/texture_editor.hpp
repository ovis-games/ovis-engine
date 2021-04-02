#pragma once

#include "asset_editor.hpp"

#include <TextEditor.h>

#include <ovis/graphics/texture2d.hpp>

namespace ovis {
namespace editor {

class TextureEditor : public AssetEditor {
 public:
  TextureEditor(const std::string& script_id);

  void DrawContent() override;
  void DrawInspectorContent() override;

  void Save() override;

 private:
  std::shared_ptr<Texture2D> texture_;
  Texture2DDescription description_;
  float scale_ = 1.0f;
};

}  // namespace editor
}  // namespace ovis
