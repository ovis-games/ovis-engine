#pragma once

#include <ovis/engine/window.hpp>

namespace ovis {
namespace editor {

class EditorWindow : public Window {
 public:
  EditorWindow();

  void LoadGameWithId(const std::string& game_id);

  bool SendEvent(const SDL_Event& event) override;
  void Update(std::chrono::microseconds delta_time) override;

  static inline EditorWindow* instance() { return instance_; }

 private:
  static EditorWindow* instance_;

  void SetUIStyle();
};

}  // namespace editor
}  // namespace ovis