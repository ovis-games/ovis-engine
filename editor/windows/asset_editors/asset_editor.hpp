#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <SDL2/SDL.h>
#include <imgui.h>

#include "../../action_history.hpp"

#include <ovis/core/file.hpp>
#include <ovis/core/log.hpp>

namespace ove {

class AssetEditor {
 public:
  AssetEditor(const std::string& asset_id);
  virtual ~AssetEditor();

  inline std::string asset_id() const { return asset_id_; }
  inline bool is_focused() const { return is_focused_; }
  inline void Focus() { should_focus_ = true; }
  inline bool should_close() const { return !keep_open_; }

  void SaveFile(const std::string& type, const std::variant<std::string, ovis::Blob>& content);
  std::optional<std::string> LoadTextFile(const std::string& file_type);
  std::optional<ovis::Blob> LoadBinaryFile(const std::string& file_type);

  void Draw(ImGuiID dockspace_id);
  void CheckForFocus();

  virtual void Update(std::chrono::microseconds delta_time) {}
  virtual bool ProcessEvent(const SDL_Event& event) { return false; }
  virtual void Draw();
  virtual void DrawPropertyWindows();
  virtual void Save() = 0;
  virtual ActionHistoryBase* GetActionHistory() = 0;

  static AssetEditor* last_focused_document_window;

 private:
  std::string asset_id_;
  bool is_focused_ = false;
  bool first_frame_ = true;
  bool keep_open_ = true;
  bool should_focus_ = false;
};

using AssetEditors = std::vector<std::unique_ptr<AssetEditor>>;

}  // namespace ove