#pragma once

#include <string>
#include <string_view>

#include "ovis/utils/safe_pointer.hpp"
#include "ovis/utils/serialize.hpp"

namespace ovis {

// class SceneObject;

// struct SceneObjectAnimationKeyframe {
//   std::uint32_t frame;
//   // vm::Value value;
// };

// struct SceneObjectAnimationChannel {
//   std::string object_path;
//   // std::weak_ptr<Type> component_type;
//   std::string property;
//   // std::weak_ptr<Function> interpolation_function;
//   std::vector<SceneObjectAnimationKeyframe> keyframes;
// };

// class SceneObjectAnimation : public Serializable, public SafelyReferenceable {
//  public:
//   SceneObjectAnimation(std::string_view name);

//   std::string_view name() const { return name_; }
//   std::uint32_t start() const { return start_; }
//   std::uint32_t end() const { return end_; }
//   std::uint32_t duration() const { return end_ - start_; }
//   float speed() const { return speed_; }

//   void Animate(float frame, SceneObject* object);

//   json Serialize() const override;
//   bool Deserialize(const json& data) override;

//  private:
//   std::string name_;
//   std::uint32_t start_;
//   std::uint32_t end_;
//   float speed_;
//   std::vector<SceneObjectAnimationChannel> channels_;
// };

}

