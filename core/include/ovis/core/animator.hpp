#pragma once

#include <chrono>

#include <ovis/core/scene_object_animation.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/vm_bindings.hpp>

namespace ovis {

class Animator : public SceneObjectComponent {
  OVIS_MAKE_DYNAMICALLY_LUA_REFERENCABLE();
  friend class AnimatorController;

 public:
  // Play animation immediately, active animations will be cancelled.
  void PlayAnimation(std::string_view animation, bool loop = false);

  // Queue animation. It will be played if the current animation has finished.
  void QueueAnimation(std::string_view animation, bool loop = false);

  void StopAnimation();

  json Serialize() const override;
  bool Deserialize(const json& data) override;

  OVIS_VM_DECLARE_TYPE_BINDING();

 private:
  struct PlayInfo {
    safe_ptr<SceneObjectAnimation> animation;
    bool loop;
  };
  PlayInfo current_;
  PlayInfo next_;
  float current_frame_ = 0.0f;

  void AdvanceFrame(float delta_time_in_seconds);
};

}  // namespace ovis

