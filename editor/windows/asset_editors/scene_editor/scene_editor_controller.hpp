#include <ovis/engine/scene_controller.hpp>

namespace ovis {
namespace editor {

class SceneEditorController : public SceneController {
 public:
  bool ProcessEvent(const SDL_Event& event) override;

 private:
};

}  // namespace editor

}  // namespace ovis
