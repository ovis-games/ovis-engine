#pragma once

#include <ovis/imgui/imgui_window.hpp>

#include <imgui.h>

#include <ovis/graphics/texture2d.hpp>

namespace ovis {
namespace editor {

class Toolbar : public ImGuiWindow {
 public:
  Toolbar();

  void ProcessEvent(Event* event) override;

 protected:
  void BeforeBegin() override;
  void DrawContent() override;

 private:
  void Save();
  void Undo();
  void Redo();
};

}  // namespace editor
}  // namespace ovis
