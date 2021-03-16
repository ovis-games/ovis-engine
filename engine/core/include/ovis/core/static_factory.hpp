#pragma once

#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <ovis/core/range.hpp>

namespace ovis {

// Usage: class SomeFactory : StaticFactory<SomeFactory, float(int)> {}; for a
// class that produces floats with an int as an input
template <typename FactoryClass, typename Function>
class StaticFactory {
 public:
  static bool Register(std::string id, std::function<Function> factory_function) {
    return factory_functions_.insert(std::make_pair(std::move(id), std::move(factory_function))).second;
  }

  static bool Deregister(const std::string& id) { return factory_functions_.erase(id) == 1; }

  static bool IsRegistered(const std::string& id) { return factory_functions_.count(id) == 1; }

  static size_t GetRegisteredFactoriesCount() { return factory_functions_.size(); }
  static auto registered_ids() { return Keys(factory_functions_); }

  template <typename... FactoryArguments>
  static std::optional<typename std::function<Function>::result_type> Create(const std::string& id,
                                                                             FactoryArguments&&... arguments) {
    static_assert(std::is_invocable_v<std::function<Function>, FactoryArguments...>,
                  "Cannot call factory function with these arguments");
    const auto it = factory_functions_.find(id);
    if (it == factory_functions_.end()) {
      return {};
    } else {
      return it->second(std::forward<FactoryArguments>(arguments)...);
    }
  }

 private:
  static std::unordered_map<std::string, std::function<Function>> factory_functions_;
};

template <typename FactoryClass, typename Function>
std::unordered_map<std::string, std::function<Function>> StaticFactory<FactoryClass, Function>::factory_functions_;

}  // namespace ovis
