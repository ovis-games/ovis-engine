#include <ovis/core/scene_object_animation.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_object_component.hpp>
#include <ovis/core/transform.hpp> // TODO: delete this line

namespace ovis {

namespace {

SceneObjectAnimationKeyframe ParseKeyframe(const vm::Type& type, const json& data) {
  return {
    .frame = data.at("frame"),
    .value = type.CreateValue(data.at("value")),
  };
}

SceneObjectAnimationChannel ParseChannel(const json& data) {
  SceneObjectAnimationChannel channel;
  channel.object_path = data.at("object");
  channel.component_type = vm::Type::Deserialize(data.at("component"));
  assert(channel.component_type != nullptr);
  channel.property = data.at("property");
  auto property = channel.component_type->GetProperty(channel.property);
  assert(property != nullptr);
  channel.interpolation_function = vm::Function::Deserialize(data.at("interpolationFunction"));
  assert(channel.interpolation_function != nullptr);
  channel.keyframes.reserve(data.at("keyframes").size());

  const auto property_type = property->type;
  for (const auto& keyframe : data.at("keyframes")) {
    channel.keyframes.push_back(ParseKeyframe(*property_type.get(), keyframe));
  }
  // Sort keyframes by frame
  std::sort(channel.keyframes.begin(), channel.keyframes.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.frame < rhs.frame;
  });
  return channel;
}

}

SceneObjectAnimation::SceneObjectAnimation(std::string_view name) : name_(name) {}

void SceneObjectAnimation::Animate(float frame, SceneObject* object) {
  assert(object != nullptr);
  assert(frame >= start_);
  assert(frame <= end_);

  for (const auto& channel : channels_) {
    SceneObject* sub_object = object; // TODO: change this
    assert(sub_object);

    vm::Value component = sub_object->GetComponent(channel.component_type);
    if (channel.keyframes.size() == 1) {
      component.SetProperty(channel.property, channel.keyframes[0].value);
    } else if (channel.keyframes.size() > 1) {
      auto value_a_it = channel.keyframes.cbegin();
      auto value_b_it = std::next(value_a_it);
      while (value_b_it->frame < frame && std::next(value_b_it) != channel.keyframes.end()) {
        value_a_it = value_b_it;
        value_b_it = std::next(value_b_it);
      }

      const float t =  (frame - value_a_it->frame) / (value_b_it->frame - value_a_it->frame);
      const vm::Value result = channel.interpolation_function->Call<vm::Value>(value_a_it->value, value_b_it->value, t);
      component.SetProperty(channel.property, result);
    }
  }
}

bool SceneObjectAnimation::Deserialize(const json& data) {
  if (data.contains("start")) {
    start_ = data.at("start");
  } else {
    start_ = 0;
  }

  if (data.contains("end")) {
    end_ = data.at("end");
  } else {
    end_ = 100;
  }

  if (data.contains("speed")) {
    speed_ = data.at("speed");
  } else {
    speed_ = 1.0f;
  }

  if (data.contains("channels")) {
    channels_.clear();
    channels_.reserve(data.at("channels").size());
    for (const auto& channel : data.at("channels")) {
      channels_.push_back(ParseChannel(channel));
    }
  } else {
    channels_ = {};
  }

  return true;
}

json SceneObjectAnimation::Serialize() const {
  return json();
}

}

