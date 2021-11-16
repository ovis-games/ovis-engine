#include <ovis/core/animator.hpp>
#include <ovis/core/animator_controller.hpp>
#include <ovis/core/scene.hpp>

namespace ovis {

void AnimatorController::Update(std::chrono::microseconds delta_time) {
  const float delta_time_in_seconds = delta_time.count() / 1000000.0f;
  for (auto& animation : scene()->ObjectComponentsOfType<Animator>()) {
    animation->AdvanceFrame(delta_time_in_seconds);
  }
}

}
