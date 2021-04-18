#pragma once

#include <chrono>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include <sol/sol.hpp>

#include <ovis/utils/class.hpp>
#include <ovis/utils/serialize.hpp>
#include <ovis/utils/static_factory.hpp>
#include <ovis/core/event.hpp>

#if OVIS_ENABLE_BUILT_IN_PROFILING == 1
#include <ovis/utils/profiling.hpp>
#endif

namespace ovis {

class Scene;

class SceneController : public Serializable, public StaticFactory<SceneController, std::unique_ptr<SceneController>()> {
  MAKE_NON_COPY_OR_MOVABLE(SceneController);
  friend class Scene;

 public:
  explicit SceneController(std::string_view name);
  virtual ~SceneController();

  inline Scene* scene() const { return scene_; }
  inline std::string name() const { return name_; }

  void Remove();

  bool IsSubscribedToEvent(const std::string& event_type) const;
  void SubscribeToEvent(std::string event_type);
  void UnsubscribeFromEvent(const std::string& event_type);

  virtual void Play() {}
  virtual void Stop() {}
  virtual void BeforeUpdate() {}
  virtual void AfterUpdate() {}
  virtual void Update(std::chrono::microseconds delta_time);

  virtual void ProcessEvent(Event* event);

  // Default implementation that does nothing
  const json* GetSchema() const override { return &DEFAULT_SCHEMA; }
  json Serialize() const override;
  bool Deserialize(const json& data) override;

  static void RegisterType(sol::table* module);

 protected:
  void UpdateBefore(std::string_view controller_name);
  template <typename T>
  inline void UpdateBefore() {
    UpdateBefore(T::Name());
  }

  void UpdateAfter(std::string_view controller_name);
  template <typename T>
  inline void UpdateAfter() {
    UpdateAfter(T::Name());
  }

 private:
  Scene* scene_;
  std::string name_;
  std::set<std::string> update_before_list_;
  std::set<std::string> update_after_list_;
  std::set<std::string> subscribed_events_;

  static const json DEFAULT_SCHEMA;

#if OVIS_ENABLE_BUILT_IN_PROFILING
  CPUTimeProfiler update_profiler_;
#endif

  inline void UpdateWrapper(std::chrono::microseconds delta_time) {
#if OVIS_ENABLE_BUILT_IN_PROFILING
    update_profiler_.BeginMeasurement();
#endif
    Update(delta_time);
#if OVIS_ENABLE_BUILT_IN_PROFILING
    update_profiler_.EndMeasurement();
#endif
  }
};

}  // namespace ovis
