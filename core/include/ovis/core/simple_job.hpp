#pragma once

#include <type_traits>
#include <utility>

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
    ParseAccess(ArgumentTypes{}, std::make_index_sequence<ArgumentTypes::size>());
  }

  Result<> Prepare(Scene* const& parameters) override {
    return Success;
  }

  Result<> Execute(const SceneUpdate& parameters) override {
    auto storages = GetStorages(parameters.scene, ArgumentTypes{});
    for (auto id : parameters.scene->GetEntityIds()) {
      OVIS_CHECK_RESULT(Call(ArgumentTypes{}, std::make_index_sequence<ArgumentTypes::size>(), storages, id));
    }
    return Success;
  }

 private:
  template <typename... T, std::size_t... I>
  void ParseAccess(TypeList<T...>, std::index_sequence<I...>) {
    (ParseParameterAccess<nth_parameter_t<I, T...>>(I), ...);
  }

  template <typename T>
  void ParseParameterAccess(std::size_t index) {
    if constexpr (std::is_pointer_v<T>) {
      auto type = main_vm->GetTypeId<std::remove_pointer_t<T>>();
      RequireWriteAccess(type);
    } else if constexpr (std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>) {
      auto type = main_vm->GetTypeId<std::remove_reference_t<T>>();
      RequireReadAccess(type);
    } else {
      static_assert(true, "Invalid parameter type");
    }
  }

  template <typename... T>
  std::vector<ComponentStorage*> GetStorages(Scene* scene, TypeList<T...>) {
    return { scene->template GetComponentStorage<std::remove_pointer_t<T>>()... };
  }

  template <typename... T, std::size_t... I>
  Result<> Call(TypeList<T...>, std::index_sequence<I...>, const std::vector<ComponentStorage*>& storages, EntityId id) {
    if constexpr (is_result_v<typename reflection::Invocable<FUNCTION>::ReturnType>) {
      OVIS_CHECK_RESULT(FUNCTION(Get<T>(storages, I, id)...));
    } else {
      FUNCTION(Get<T>(storages, I, id)...);
    }
    return Success;
  }

  template <typename T>
  auto Get(const std::vector<ComponentStorage*>& storages, std::size_t index, EntityId id) {
    if constexpr (std::is_pointer_v<T>) {
      return &storages[index]->GetComponent<std::remove_pointer_t<T>>(id);
    } else if constexpr (std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>) {
      return storages[index]->GetComponent<std::remove_cvref_t<T>>(id);
    } else {
      static_assert(true, "Invalid parameter type");
    }
  }
};

#define OVIS_CREATE_SIMPLE_JOB(function)              \
  class function##Job : public SimpleJob<&function> { \
   public:                                            \
    function##Job() : SimpleJob(#function) {}         \
  };

}  // namespace ovis
