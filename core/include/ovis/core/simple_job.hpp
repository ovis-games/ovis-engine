#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "ovis/core/event_storage.hpp"
#include "ovis/utils/log.hpp"
#include "ovis/utils/parameter_pack.hpp"
#include "ovis/utils/reflection.hpp"
#include "ovis/utils/result.hpp"
#include "ovis/utils/type_list.hpp"
#include "ovis/vm/virtual_machine.hpp"
#include "ovis/core/component_storage.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/job.hpp"
#include "ovis/core/main_vm.hpp"
#include "ovis/core/scene.hpp"

namespace ovis {

template <auto FUNCTION>
class SimpleJob : public Job<Scene*, SceneUpdate> {
  using ArgumentTypes = typename reflection::Invocable<FUNCTION>::ArgumentTypes;

 public:
  SimpleJob(std::string_view name) : Job(name) {
    ParseAccess(ArgumentTypes{});
  }

  Result<> Prepare(Scene* const& scene) override {
    GetSources(scene, ArgumentTypes{});
    return Success;
  }

  Result<> Execute(const SceneUpdate& parameters) override {
    if constexpr (needs_iteration_) {
      for (Entity& entity : parameters.scene->entities()) {
        if (ShouldExecute(entity, ArgumentTypes{}, std::make_index_sequence<ArgumentTypes::size>())) {
          OVIS_CHECK_RESULT(Call(&entity, ArgumentTypes{}, std::make_index_sequence<ArgumentTypes::size>()));
        }
      }
      return Success;
    } else {
      OVIS_CHECK_RESULT(Call(nullptr, ArgumentTypes{}, std::make_index_sequence<ArgumentTypes::size>()));
      return Success;
    }
  }

 private:
  template <typename T> struct ParameterSource;

  template <>
  struct ParameterSource<Entity*> {
    using type = Entity*;
    static constexpr bool needs_iteration = true;
    static void ParseAccess(SimpleJob* job) { }
    static type GetSource(Scene* scene) { return nullptr; }
    static bool ShouldExecute(Entity* entity, type source) { return true; }
    static Entity* GetParameter(Entity* entity, type source) { return entity; }
  };
  template <>
  struct ParameterSource<Scene*> {
    using type = Scene*;
    static constexpr bool needs_iteration = false;
    static void ParseAccess(SimpleJob* job) { }
    static type GetSource(Scene* scene) { return scene; }
    static bool ShouldExecute(Entity* entity, type source) { return true; }
    static Scene* GetParameter(Entity* entity, Scene* scene) { return scene; }
  };
  template <typename T>
  struct ParameterSource<ComponentStorageView<T>> {
    using type = ComponentStorageView<T>;
    static constexpr bool needs_iteration = false;
    static void ParseAccess(SimpleJob* job) { ParameterSource<T>::ParseAccess(job); }
    static type GetSource(Scene* scene) { return ParameterSource<T>::GetSource(scene);  }
    static bool ShouldExecute(Entity* entity, type source) { return true; }
    static T GetParameter(Scene* scene) { return scene; }
  };
  template <typename T>
  struct ParameterSource<const T&> {
    using type = ComponentStorageView<const T>;
    static constexpr bool needs_iteration = true;
    static void ParseAccess(SimpleJob* job) {
      // assert(main_vm->GetType<T>()
      job->RequireReadAccess(main_vm->GetTypeId<T>());
    }
    static type GetSource(Scene* scene) { return scene->GetComponentStorage<const T>(); }
    static bool ShouldExecute(Entity* entity, type source) { return source.EntityHasComponent(entity->id); }
    static auto GetParameter(Entity* entity, type source) { return source.GetComponent(entity->id); }
  };
  template <typename T>
  struct ParameterSource<EventEmitter<T>> {
    using type = EventEmitter<T>;
    static constexpr bool is_event_emitter = true;
    static constexpr bool needs_iteration = false;
    static void ParseAccess(SimpleJob* job) {
      assert(main_vm->GetType<T>()->attributes().contains("Core.Event"));
      job->RequireWriteAccess(main_vm->GetTypeId<T>()); 
    }
    static type GetSource(Scene* scene) { return scene->GetEventEmitter<T>();  }
    static bool ShouldExecute(Entity* entity, type source) { return true; }
    static auto GetParameter(Entity* entity, type source) { return source; }
  };
  template <typename T>
  struct ParameterSource<T&> {
    using type = ComponentStorageView<const T>;
    static constexpr bool needs_iteration = true;
    static void ParseAccess(SimpleJob* job) { job->RequireWriteAccess(main_vm->GetTypeId<T>()); }
    static type GetSource(Scene* scene) { return scene->GetComponentStorage<T>(); }
    static bool ShouldExecute(Entity* entity, type source) { return source.EntityHasComponent(entity->id); }
    static auto GetParameter(Entity* entity, type source) { return source.GetComponent(entity->id); }
  };
  template <typename T>
  struct ParameterSource<const T*> {
    using type = ComponentStorageView<T>;
    static constexpr bool needs_iteration = true;
    static void ParseAccess(SimpleJob* job) { job->RequireReadAccess(main_vm->GetTypeId<T>()); }
    static type GetSource(Scene* scene) { return scene->GetComponentStorage<const T>(); }
    static bool ShouldExecute(Entity* entity, type source) { return true; }
    static auto GetParameter(Entity* entity, type source) {
      assert(entity != nullptr);
      return source.EntityHasComponent(entity->id) ? &source.GetComponent(entity->id) : nullptr;
    }
  };
  template <typename T>
  struct ParameterSource<T*> {
    using type = ComponentStorageView<T>;
    static constexpr bool needs_iteration = true;
    static void ParseAccess(SimpleJob* job) { job->RequireWriteAccess(main_vm->GetTypeId<T>()); }
    static type GetSource(Scene* scene) { return scene->GetComponentStorage<T>(); }
    static bool ShouldExecute(Entity* entity, type source) { return true; }
    static auto GetParameter(Entity* entity, type source) {
      assert(entity != nullptr);
      return source.EntityHasComponent(entity->id) ? &source.GetComponent(entity->id) : nullptr;
    }
  };

  template <typename T> struct ParameterSourceList;
  template <typename... T> struct ParameterSourceList<TypeList<T...>> {
    using type = std::tuple<typename ParameterSource<T>::type...>;
    static constexpr bool needs_iteration = (... || ParameterSource<T>::needs_iteration);
  };

  typename ParameterSourceList<ArgumentTypes>::type parameter_sources_;
  constexpr static bool needs_iteration_ = ParameterSourceList<ArgumentTypes>::needs_iteration;

  template <typename... T>
  void ParseAccess(TypeList<T...>) {
    (ParameterSource<T>::ParseAccess(this), ...);
  }

  template <typename... T>
  void GetSources(Scene* scene, TypeList<T...>) {
    parameter_sources_ = std::make_tuple(ParameterSource<T>::GetSource(scene)...);
  }

  template <typename... T, std::size_t... I>
  bool ShouldExecute(Entity& entity, TypeList<T...>, std::index_sequence<I...>) {
    return (... && ParameterSource<T>::ShouldExecute(&entity, std::get<I>(parameter_sources_)));
  }

  template <typename... T, std::size_t... I>
  Result<> Call(Entity* entity, TypeList<T...>, std::index_sequence<I...>) {
    if constexpr (is_result_v<typename reflection::Invocable<FUNCTION>::ReturnType>) {
      OVIS_CHECK_RESULT(FUNCTION(ParameterSource<T>::GetParameter(entity, std::get<I>(parameter_sources_))...));
    } else {
      FUNCTION(ParameterSource<T>::GetParameter(entity, std::get<I>(parameter_sources_))...);
    }
    return Success;
  }
};

#define OVIS_CREATE_SIMPLE_JOB(function)              \
  class function##Job : public SimpleJob<&function> { \
   public:                                            \
    function##Job() : SimpleJob(#function) {}         \
  };

}  // namespace ovis
