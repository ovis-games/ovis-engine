#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/script_scene_controller.hpp>

namespace ovis {

namespace {

ScriptSceneController* GetController(const sol::table& scene_controller) {
  ScriptSceneController* controller = scene_controller["_script_controller_instance"];
  if (controller == nullptr) {
    sol::error(
        "Internal variable _script_controller_instance not set. If you instantiated this scene controller "
        "yourself via SceneController:new() you cannot call this function in initialize(). If this is not the case, "
        "please file a bug report!");
  }
  return controller;
}

sol::table GetSubscribedEventsTable(sol::table& scene_controller) {
  sol::object subscribed_events = scene_controller["_subscribed_events"];
  if (subscribed_events.get_type() != sol::type::table) {
    return scene_controller["_subscribed_events"] = sol::table::create(scene_controller.lua_state());
  } else {
    return subscribed_events;
  }
}

}  // namespace

ScriptSceneController::ScriptSceneController(const std::string& name, sol::table class_table) : SceneController(name) {
  sol::protected_function new_function = class_table["new"];
  if (new_function != sol::lua_nil) {
    class_table["_script_controller_instance"] = this;

    sol::protected_function_result class_instance = new_function.call(class_table);
    if (class_instance.valid() && class_instance.get_type() == sol::type::table) {
      sol::table instance = class_instance;

      class_table["_script_controller_instance"] = nullptr;
      instance["_script_controller_instance"] = this;

      instance_ = instance;
      play_function_ = instance_["play"];
      stop_function_ = instance_["stop"];
      update_function_ = instance_["update"];
      process_event_function_ = instance_["process_event"];
    }
  } else {
    LogE("Failed to find constructor for '{}'", name);
  }
}

ScriptSceneController::~ScriptSceneController() {
  if (is_valid()) {
    instance_["_script_controller_instance"] = nullptr;
  }
}

void ScriptSceneController::Play() {
  instance_["scene"] = scene();

  if (play_function_ != sol::lua_nil) {
    sol::protected_function_result result = play_function_.call(instance_);
  }
}

void ScriptSceneController::Stop() {
  instance_["scene"] = scene();

  if (stop_function_ != sol::lua_nil) {
    sol::protected_function_result result = stop_function_.call(instance_);
  }
}

void ScriptSceneController::Update(std::chrono::microseconds delta_time) {
  instance_["scene"] = scene();

  if (update_function_ != sol::lua_nil) {
    sol::protected_function_result result = update_function_.call(instance_, delta_time.count() / 1000000.0);
  }
}

void ScriptSceneController::ProcessEvent(Event* event) {
  instance_["scene"] = scene();

  sol::protected_function event_callback;

  auto event_callback_entry = GetSubscribedEventsTable(instance_)[event->type()];
  if (event_callback_entry.get_type() == sol::type::string) {
    LogD("Call {}", event_callback_entry.get<std::string>());
    event_callback = instance_[event_callback_entry];
  } else if (event_callback_entry.get_type() == sol::type::function) {
    LogD("Call function");
    event_callback = event_callback_entry;
  } else {
    LogD("Call process_event");
    event_callback = process_event_function_;
  }

  if (event_callback != sol::lua_nil) {
    sol::protected_function_result result = event_callback.call(instance_, event->GetValue());
  }
}

void ScriptSceneController::RegisterType(sol::table* module) {
  sol::table scene_controller_type = (*module)["class"]("ScriptSceneController");
  scene_controller_type["subscribe_to_event"] = sol::overload(
      [](sol::table scene_controller, std::string event_type) {
        GetSubscribedEventsTable(scene_controller)[event_type] = true;
        GetController(scene_controller)->SubscribeToEvent(event_type);
        LogI("Subscribed to event '{}'", event_type);
      },
      [](sol::table scene_controller, std::string event_type, std::string member_function_name) {
        GetSubscribedEventsTable(scene_controller)[event_type] = member_function_name;
        GetController(scene_controller)->SubscribeToEvent(event_type);
        LogI("Subscribed to event '{}' via member function name", event_type);
      },
      [](sol::table scene_controller, std::string event_type, sol::function callback) {
        GetSubscribedEventsTable(scene_controller)[event_type] = callback;
        GetController(scene_controller)->SubscribeToEvent(event_type);
        LogI("Subscribed to event '{}' via callback", event_type);
      });

  scene_controller_type["unsubscribe_from_event"] = [](sol::table scene_controller, std::string event_type) {
    GetController(scene_controller)->UnsubscribeFromEvent(event_type);
    GetSubscribedEventsTable(scene_controller)[event_type] = sol::nil;
    LogI("Unsubscribed from event: {}", event_type);
  };

  scene_controller_type["is_subscribed_to_event"] = [](sol::table scene_controller, std::string event_type) {
    return GetController(scene_controller)->IsSubscribedToEvent(event_type);
  };

  scene_controller_type["initialize"] = [](sol::table scene_controller) { GetSubscribedEventsTable(scene_controller); };
  (*module)["ScriptSceneController"] = scene_controller_type;


  sol::usertype<ScriptSceneController> script_scene_controller_type =
      module->new_usertype<ScriptSceneController>("ScriptSceneControllerWrapper");
  script_scene_controller_type["subscribe_to_event"] = &ScriptSceneController::SubscribeToEvent;
  script_scene_controller_type["unsubscribe_from_event"] = &ScriptSceneController::UnsubscribeFromEvent;
  script_scene_controller_type["is_subscribed_to_event"] = &ScriptSceneController::IsSubscribedToEvent;
}

std::unique_ptr<ScriptSceneController> LoadScriptSceneController(const std::string& asset_id, sol::state* lua_state) {
  SDL_assert(lua_state != nullptr);

  if (GetApplicationAssetLibrary() != nullptr && GetApplicationAssetLibrary()->Contains(asset_id) &&
      GetApplicationAssetLibrary()->GetAssetType(asset_id) == "scene_controller") {
    std::optional<std::string> code = GetApplicationAssetLibrary()->LoadAssetTextFile(asset_id, "lua");

    if (code) {
      sol::protected_function_result result = lua_state->do_string(*code, "=" + asset_id);
      if (result.valid() && result.get_type() == sol::type::table) {
        sol::table class_table = result.get<sol::table>();
        auto controller = std::make_unique<ScriptSceneController>(asset_id, class_table);
        if (controller && controller->is_valid()) {
          return controller;
        } else {
          LogE("Failed to create class instance for script controller '{}'", asset_id);
          return nullptr;
        }
      } else {
        LogE("Failed to execute scene controller script '{}'", asset_id);
        return nullptr;
      }
    } else {
      LogE("Failed to load scene controller script '{}'", asset_id);
      return nullptr;
    }
  } else {
    LogE("Cannot find scene controller script for '{}'", asset_id);
    return nullptr;
  }
}

}  // namespace ovis