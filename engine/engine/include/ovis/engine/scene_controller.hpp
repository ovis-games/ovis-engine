#pragma once

#include <chrono>
#include <set>
#include <string>
#include <unordered_map>

#include <ovis/core/class.hpp>
#include <ovis/engine/event.hpp>

#if OVIS_ENABLE_BUILT_IN_PROFILING == 1
#include <ovis/core/profiling.hpp>
#endif

namespace ovis {

class Scene;

class SceneController {
  MAKE_NON_COPY_OR_MOVABLE(SceneController);
  friend class Scene;

 public:
  SceneController(const std::string& name);
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
  virtual void DrawImGui() {}

  static void RegisterToLua();
  static std::vector<std::string> GetRegisteredControllers();

 protected:
  void UpdateBefore(const std::string& controller_name);
  void UpdateAfter(const std::string& controller_name);

 private:
  Scene* scene_;
  std::string name_;
  std::set<std::string> update_before_list_;
  std::set<std::string> update_after_list_;
  std::set<std::string> subscribed_events_;

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
