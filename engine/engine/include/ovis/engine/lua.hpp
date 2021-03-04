#pragma once

#include <string>

#include <sol/sol.hpp>

#include <ovis/core/event.hpp>
#include <ovis/core/json.hpp>
#include <ovis/core/resource_manager.hpp>

namespace ovis {

class Lua {
 public:
  static sol::state state;

  static void SetupEnvironment();
  static sol::protected_function_result Execute(const std::string& code, const std::string& chunk_name);

  static EventHandler<void(const std::string&)> on_error;
};

// sol::state& lua();

// void SetupLuaEnvironment();
// bool LoadScript(ResourceManager* resource_manager, const json& parameters, const std::string& id,
//                 const std::string& directory);

}  // namespace ovis