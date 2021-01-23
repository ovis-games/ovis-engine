#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <SDL2/SDL.h>
#include <imgui.h>

#include "../../action_history.hpp"
#include "../ui_window.hpp"

#include <ovis/core/file.hpp>
#include <ovis/core/log.hpp>

namespace ove {

class AssetEditor : public UiWindow {
 public:
  AssetEditor(const std::string& asset_id);
  virtual ~AssetEditor();

  inline std::string asset_id() const { return asset_id_; }

  void SaveFile(const std::string& type, const std::variant<std::string, ovis::Blob>& content);
  std::optional<std::string> LoadTextFile(const std::string& file_type);
  std::optional<ovis::Blob> LoadBinaryFile(const std::string& file_type);

  virtual void DrawInspectorContent() {}
  virtual void Save() = 0;
  virtual ActionHistoryBase* GetActionHistory() = 0;

  void DrawImGui() override;
  static AssetEditor* last_focused_document_window;
  
  static const std::string GetAssetEditorId(const std::string& asset_id);

 private:
  std::string asset_id_;
  bool is_focused_ = false;
  bool first_frame_ = true;
  bool keep_open_ = true;
};

}  // namespace ove