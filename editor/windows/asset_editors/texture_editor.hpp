#pragma once

#include <TextEditor.h>

#include "asset_editor.hpp"

#include <ovis/graphics/texture2d.hpp>

namespace ove {

class TextureEditor : public AssetEditor {
 public:
  TextureEditor(const std::string& script_id);

  void Draw() override;
  void Save() override;
  ActionHistoryBase* GetActionHistory() override { return &action_history_; }

 private:
  std::unique_ptr<ovis::Texture2D> texture_;
  ActionHistory<TextureEditor> action_history_;
};

}  // namespace ove
