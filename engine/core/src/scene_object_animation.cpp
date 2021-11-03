#include <ovis/core/scene_object_animation.hpp>

namespace ovis {

namespace {

SceneObjectAnimationKeyframe ParseKeyframe(vm::Type* type, const json& data) {
  return {
    .frame = data.at("frame"),
    .value = vm::Value()
  };
}

SceneObjectAnimationChannel ParseChannel(const json& data) {
  SceneObjectAnimationChannel channel;
  channel.object_path = data.at("object");
  channel.component = data.at("component");
  channel.property = data.at("property");
  channel.type = vm::Type::Deserialize(data.at("type"));
  channel.keyframes.reserve(data.at("keyframes").size());
  for (const auto& keyframe : data.at("keyframes")) {
    channel.keyframes.push_back(ParseKeyframe(channel.type.get(), keyframe));
  }
  // Sort keyframes by frame
  std::sort(channel.keyframes.begin(), channel.keyframes.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.frame < rhs.frame;
  });
  return channel;
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

}

