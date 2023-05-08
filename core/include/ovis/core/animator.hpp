#pragma once

#include <chrono>

#include "ovis/core/scene_object_animation.hpp"
#include "ovis/core/vm_bindings.hpp"

namespace ovis {

// class Animator {
//  public:
//   // Play animation immediately, active animations will be cancelled.
//   void PlayAnimation(std::string_view animation, bool loop = false);

//   // Queue animation. It will be played if the current animation has finished.
//   void QueueAnimation(std::string_view animation, bool loop = false);

//   void StopAnimation();

//   OVIS_VM_DECLARE_TYPE_BINDING();

//  private:
//   struct PlayInfo {
//     safe_ptr<SceneObjectAnimation> animation;
//     bool loop;
//   };
//   PlayInfo current_;
//   PlayInfo next_;
//   float current_frame_ = 0.0f;

//   void AdvanceFrame(float delta_time_in_seconds);
// };

}  // namespace ovis

