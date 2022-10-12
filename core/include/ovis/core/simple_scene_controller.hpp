#pragma once

#include <type_traits>
#include <utility>

#include "ovis/utils/log.hpp"
#include "ovis/utils/parameter_pack.hpp"
#include "ovis/utils/reflection.hpp"
#include "ovis/utils/type_list.hpp"
#include "ovis/vm/virtual_machine.hpp"
#include "ovis/core/main_vm.hpp"
#include "ovis/core/scene_controller.hpp"

namespace ovis {

template <auto FUNCTION>
class SimpleSceneController : public SceneController {
  using ArgumentTypes = typename reflection::Invocable<FUNCTION>::ArgumentTypes;

 public:
  SimpleSceneController(std::string_view name) : SceneController(name) {
    ParseAccess(ArgumentTypes{}, std::make_index_sequence<ArgumentTypes::size>());
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
};

}  // namespace ovis
