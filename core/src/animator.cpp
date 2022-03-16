#include <ovis/core/animator.hpp>
#include <ovis/core/module.hpp>
#include <ovis/core/scene_object.hpp>

namespace ovis {

void Animator::PlayAnimation(std::string_view animation_name, bool loop) {
  current_.animation = scene_object()->GetAnimation(animation_name);
  current_.loop = loop;
  current_frame_ = 0.0f;
}

void Animator::AdvanceFrame(float delta_time_in_seconds) {
  if (current_.animation != nullptr) {
    current_frame_ += delta_time_in_seconds * current_.animation->speed();
    current_.animation->Animate(current_frame_, scene_object());

    if (current_frame_ >= current_.animation->end()) [[unlikely]] {
      if (current_.loop) {
        current_frame_ = current_.animation->start() + std::fmod(current_frame_ - current_.animation->start(), current_.animation->duration());
      } else  {
        current_frame_ = 0.0f;
        if (next_.animation != nullptr) {
          const float remaining_seconds = (current_frame_ - current_.animation->end()) / current_.animation->speed();
          current_.animation = next_.animation;
          current_.loop = next_.loop;
          next_.animation = nullptr;
          AdvanceFrame(remaining_seconds);
        } else {
          current_.animation = nullptr;
        }
      }
    }
    
  }
}

json Animator::Serialize() const {
  json animation = json::object();
  if (current_.animation != nullptr) {
    animation["animation"] = current_.animation->name();
    animation["loop"] = current_.loop;
  }
  animation["currentFrame"] = current_frame_;
  return animation;
}

bool Animator::Deserialize(const json& data) {
  if (data.contains("animation")) {
    PlayAnimation(static_cast<std::string>(data.at("animation")), data.at("loop"));
  } else {
    current_.animation = nullptr;
  }

  if (data.contains("currentFrame")) {
    current_frame_ = data.at("currentFrame");
  } else {
    current_frame_ =  0.0f;
  }
  return true;
}

void Animator::RegisterType(Module* module) {
  auto animator_description = TypeDescription::CreateForNativeType<Animator, SceneObjectComponent>("Animator");
  module->RegisterType(animator_description);
}

}
