#pragma once

#include <TextEditor.h>

#include "asset_editor.hpp"

#include <ovis/graphics/texture2d.hpp>

namespace ove {

class TextureEditor : public AssetEditor {
 public:
  TextureEditor(const std::string& script_id);

  void DrawContent() override;
  void DrawInspectorContent() override;

  void Save() override;
  ActionHistoryBase* GetActionHistory() override { return &action_history_; }

 private:
  std::unique_ptr<ovis::Texture2D> texture_;
  ovis::Texture2DDescription description_;
  ActionHistory<TextureEditor> action_history_;
  float scale_ = 1.0f;
};

}  // namespace ove
