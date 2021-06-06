#include <iterator>
#include <map>
#include <string>

#include <SDL2/SDL_assert.h>

#include <ovis/utils/log.hpp>
#include <ovis/utils/utf8.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/core/scene.hpp>
#include <ovis/core/scene_controller.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_viewport.hpp>
#include <ovis/core/script_scene_controller.hpp>

namespace ovis {

const json Scene::schema_ = {{"$ref", "engine#/$defs/scene"}};

Scene::Scene() {}

Scene::~Scene() {
  // Explicitly clear owning containers because the destructor of their objects may need to still access the scene.
  ClearObjects();
  controllers_.clear();
  removed_controllers_.clear();
}

SceneController* Scene::AddController(std::unique_ptr<SceneController> scene_controller) {
  if (!scene_controller) {
    LogE("Scene controller is null!");
    return nullptr;
  }

  const std::string scene_controller_id = scene_controller->name();
  if (controllers_.count(scene_controller_id) != 0) {
    LogE("Scene controller '{}' already added", scene_controller_id);
    return nullptr;
  }

  auto insert_return_value = controllers_.insert(std::make_pair(scene_controller_id, std::move(scene_controller)));
  SDL_assert(insert_return_value.second);
  SceneController* inserted_scene_controller = insert_return_value.first->second.get();
  inserted_scene_controller->scene_ = this;
  InvalidateControllerOrder();
  if (is_playing_) {
    inserted_scene_controller->Play();
  }
  return inserted_scene_controller;
}

SceneController* Scene::AddController(const std::string& scene_controller_id) {
  if (controllers_.count(scene_controller_id) != 0) {
    LogE("Scene controller '{}' already added", scene_controller_id);
    return nullptr;
  }

  std::unique_ptr<SceneController> controller;
  if (SceneController::IsRegistered(scene_controller_id)) {
    controller = *SceneController::Create(scene_controller_id);
  } else {
    controller = LoadScriptSceneController(scene_controller_id, &lua);
  }

  if (controller == nullptr) {
    LogE("Failed to create scene controller '{}'", scene_controller_id);
    return nullptr;
  }
  return AddController(std::move(controller));
}

SceneController* Scene::AddController(const std::string& id, const json& serialized_controller) {
  SceneController* controller = AddController(id);
  if (controller) {
    controller->Deserialize(serialized_controller);
  }
  return controller;
}

// SceneController* Scene::AddController(const std::string& id, const sol::table& properties) {
//   SceneController* controller = AddController(id);

//   if (controller != nullptr) {
//     sol::table lua_controller = controller->GetValue().as<sol::table>();
//     for (const auto& [key, value] : properties) {
//       lua_controller[key] = value;
//     }
//   }

//   return controller;
// }

std::string Scene::CreateControllerName(std::string_view base_name) {
  std::string name(base_name);

  uint64_t number = 2;
  while (controllers_.contains(name)) {
    name = std::string(base_name) + std::to_string(number);
    number++;
  }

  return name;
}

void Scene::RemoveController(const std::string& id) {
  const auto scene_controller = controllers_.find(id);
  if (scene_controller == controllers_.end()) {
    LogE("The scene does not contain the controller '{}'", id);
  } else {
    if (is_playing_) {
      scene_controller->second->Stop();
    }
    removed_controllers_.push_back(std::move(scene_controller->second));
    controllers_.erase(scene_controller);
    InvalidateControllerOrder();
  }
}

void Scene::ClearControllers() {
  removed_controllers_.reserve(removed_controllers_.size() + controllers_.size());
  for (auto& controller : controllers_) {
    removed_controllers_.push_back(std::move(controller.second));
  }
  controllers_.clear();
  InvalidateControllerOrder();
}

bool Scene::HasController(const std::string& id) const {
  return controllers_.contains(id);
}

SceneObject* Scene::CreateObject(std::string_view base_name, SceneObject* parent) {
  if (!SceneObject::IsValidName(base_name)) {
    LogE("Invalid object name: {}", base_name);
    return nullptr;
  }

  std::string object_name(base_name);
  std::string object_path = SceneObject::BuildPath(object_name, parent);
  if (ContainsObject(object_path)) {
    // If the name already exists append a number to the end. If there is already a number
    // at the end, increment it.
    auto parse_result = SceneObject::ParseName(object_name);
    unsigned int number = parse_result.second.has_value() ? *parse_result.second : 1;
    do {
      ++number;
      object_name = fmt::format("{}{}", parse_result.first, number);
      object_path = SceneObject::BuildPath(object_name, parent);
    } while (ContainsObject(object_path));
  }

  auto insert_result = objects_.insert(std::make_pair(object_path, std::make_unique<SceneObject>(this, object_name, parent)));
  SDL_assert(insert_result.second);
  SDL_assert(object_path == insert_result.first->second->path());
  return insert_result.first->second.get();
}

SceneObject* Scene::CreateObject(std::string_view object_name, const json& serialized_object, SceneObject* parent) {
  auto object = CreateObject(object_name, parent);
  object->Deserialize(serialized_object);
  return object;
}

SceneObject* Scene::CreateObject(std::string_view object_name, const sol::table& component_properties, SceneObject* parent) {
  auto object = CreateObject(object_name, parent);

  for (const auto& [key, value] : component_properties) {
    object->AddComponent(key.as<std::string>(), value.as<sol::table>());
  }

  return object;
}

void Scene::DeleteObject(std::string_view object_path) {
  SDL_assert(objects_.count(std::string(object_path)) == 1);
  objects_.erase(std::string(object_path));
}

void Scene::DeleteObject(SceneObject* object) {
  if (object != nullptr) {
    SDL_assert(object->scene() == this);
    DeleteObject(object->path());
  }
}

void Scene::ClearObjects() {
  while (objects_.size() > 0) {
    // Destroy one at a time because parent objects will explicitly destory their children
    // TODO: maybe set a special flag in the scene that we are already destructing?
    objects_.erase(objects_.begin());
  }
}

SceneObject* Scene::GetObject(std::string_view object_path) {
  auto object = objects_.find(std::string(object_path));
  if (object == objects_.end()) {
    return nullptr;
  } {
    return object->second.get();
  }
}

bool Scene::ContainsObject(std::string_view object_reference) {
  return objects_.count(std::string(object_reference));
}

void Scene::GetSceneObjectsWithComponent(const std::string& component_id,
                                         std::vector<SceneObject*>* scene_objects) const {
  SDL_assert(scene_objects != nullptr);
  scene_objects->clear();

  for (auto& name_object_pair : objects_) {
    SceneObject* scene_object = name_object_pair.second.get();
    if (scene_object->HasComponent(component_id)) {
      scene_objects->push_back(scene_object);
    }
  }
}

void Scene::Play() {
  SDL_assert(!is_playing());
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->Play();
  }
  is_playing_ = true;
}

void Scene::Stop() {
  SDL_assert(is_playing());
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->Stop();
  }
  DeleteRemovedControllers();
  is_playing_ = false;
}

void Scene::BeforeUpdate() {
  SDL_assert(is_playing() && "Call Play() before calling Update().");
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->BeforeUpdate();
  }
}

void Scene::AfterUpdate() {
  SDL_assert(is_playing() && "Call Play() before calling Update().");
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->AfterUpdate();
  }
  DeleteRemovedControllers();
}

void Scene::Update(std::chrono::microseconds delta_time) {
  SDL_assert(is_playing() && "Call Play() before calling Update().");
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->UpdateWrapper(delta_time);
  }
}

void Scene::ProcessEvent(Event* event) {
  const std::string event_type(event->type());

  for (const auto& controller : controller_order_) {
    if (controller->IsSubscribedToEvent(event_type)) {
      controller->ProcessEvent(event);
      if (!event->is_propagating()) {
        break;
      }
    }
  }
}

json Scene::Serialize() const {
  json serialized_object = {{"version", "0.1"}};
  auto& controllers = serialized_object["controllers"] = json::object();
  for (const auto& controller : controllers_) {
    controllers[controller.first] = controller.second->Serialize();
  }

  auto& objects = serialized_object["objects"] = json::object();
  for (const auto& object : objects_) {
    if (object.second->parent() == nullptr) {
      objects[object.first] = object.second->Serialize();
    }
  }

  Camera* camera = main_viewport() ? main_viewport()->camera() : nullptr;
  if (camera != nullptr) {
    SDL_assert(camera->scene_object() != nullptr);
    serialized_object["camera"] = camera->scene_object()->name();
  }
  // if (camera_object_.size() > 0) {
  //   serialized_object["camera"] = camera_object_;
  // }

  return serialized_object;
}

bool Scene::Deserialize(const json& serialized_object) {
  if (!serialized_object.contains("version") || serialized_object["version"] != "0.1") {
    LogE("Invalid scene object. Version must be 0.1!");
    return false;
  }

  ClearObjects();
  ClearControllers();

  if (serialized_object.contains("controllers") && serialized_object["controllers"].is_object()) {
    for (const auto& controller : serialized_object["controllers"].items()) {
      AddController(controller.key(), controller.value());
    }
  }

  if (serialized_object.contains("objects") && serialized_object["objects"].is_object()) {
    for (const auto& object : serialized_object["objects"].items()) {
      CreateObject(object.key(), object.value());
    }
  }

  if (serialized_object.contains("camera")) {
    // camera_object_ = serialized_object.at("camera");
    if (main_viewport() != nullptr) {
      // Need to copy out variable here as nlohmann json currently does not support direct
      // conversion to std::string_view
      const std::string camera_object_reference = serialized_object.at("camera");
      SceneObject* object = GetObject(camera_object_reference);
      if (object != nullptr && object->HasComponent("Camera")) {
        main_viewport()->SetCamera(object->GetComponent<Camera>("Camera"));
        LogD("Setting Camera");
      } else {
        main_viewport()->SetCamera(nullptr);
        LogD("Setting Camera to null");
      }
    }
  }

  return true;
}

void Scene::InvalidateControllerOrder() {
  controllers_sorted_ = false;
}

void Scene::DeleteRemovedControllers() {
  if (removed_controllers_.size() > 0) {
    LogD("Delete {} removed controller(s)", removed_controllers_.size());
    removed_controllers_.clear();
  }
}

void Scene::SortControllers() {
  // First depends on second beeing already rendered
  std::multimap<std::string, std::string> dependencies;
  std::set<std::string> controllers_left;

  for (const auto& name_controller_pair : controllers_) {
    controllers_left.insert(name_controller_pair.first);

    for (auto update_before : name_controller_pair.second->update_before_list_) {
      if (controllers_.count(update_before) == 0) {
        LogW("Cannot update {0} before {1}, {1} not found!", name_controller_pair.first, update_before);
      } else {
        dependencies.insert(std::make_pair(update_before, name_controller_pair.first));
      }
    }

    for (auto update_after : name_controller_pair.second->update_after_list_) {
      if (controllers_.count(update_after) == 0) {
        LogW("Cannot update {0} after {1}, {1} not found!", name_controller_pair.first, update_after);
      } else {
        dependencies.insert(std::make_pair(name_controller_pair.first, update_after));
      }
    }
  }

  LogV("Sorting controllers:");
#ifndef NDEBUG
  for (const auto& [controller, dependency] : dependencies) {
    LogV("{} depends on {}", controller, dependency);
  }
#endif

  controller_order_.clear();
  controller_order_.reserve(controllers_.size());
  while (controllers_left.size() > 0) {
    auto next = std::find_if(controllers_left.begin(), controllers_left.end(),
                             [&dependencies](const std::string& value) { return dependencies.count(value) == 0; });

    SDL_assert(next != controllers_left.end());
    LogV(" {}", *next);

    SDL_assert(controllers_.find(*next) != controllers_.end());
    controller_order_.push_back(controllers_[*next].get());
    for (auto i = dependencies.begin(), e = dependencies.end(); i != e;) {
      if (i->second == *next) {
        i = dependencies.erase(i);
      } else {
        ++i;
      }
    }
    controllers_left.erase(next);
  }

  LogV("Controllers sorted!");
  controllers_sorted_ = true;
}

SceneController* Scene::GetControllerInternal(std::string_view controller_name) const {
  auto controller = controllers_.find(controller_name);
  if (controller == controllers_.end()) {
    return nullptr;
  } else {
    return controller->second.get();
  }
}

void Scene::RegisterType(sol::table* module) {
  // clang-format off

  /// This class represents a scene.
  // @classmod ovis.core.Scene
  // @testinginclude <ovis/core/scene.hpp>
  // @cppsetup ovis::Scene scene;
  // @cppsetup ovis::lua["scene"] = &scene;
  sol::usertype<Scene> scene_type = module->new_usertype<Scene>("Scene", sol::no_constructor);

  /// Creates a new object within the scene.
  // @function Scene:add_object
  // @string basename The name of the new object. If it does already exist an increasing number will be added to the
  // end.
  // @treturn SceneObject
  // @usage local first_object = scene:add_object("object")
  // assert(first_object.name == "object")
  // local second_object = scene:add_object("object")
  // assert(second_object.name == "object2")
  scene_type["add_object"] = sol::overload(
    [](Scene* scene, const std::string& name) {
      return safe_ptr(scene->CreateObject(name));
    },
    [](Scene* scene, const std::string& name, const sol::table& component_properties) {
      return safe_ptr(scene->CreateObject(name, component_properties));
    });

  /// Returns an object of the scene.
  // @function Scene:get_object
  // @string name The name of the object.
  // @treturn SceneObject
  // @usage local object = scene:add_object("object")
  // assert(object.name == "object")
  // local same_object = scene:get_object("object")
  // assert(object == same_object)
  scene_type["get_object"] = [](Scene* scene, const std::string& name) {
    return safe_ptr(scene->GetObject(name));
  };

  /// Removes an object from the scene.
  // @function Scene:remove_object
  // @string name The name of the object
  // @usage scene:add_object("object")
  // assert(scene:contains_object("object"))
  // scene:remove_object("object")
  // assert(not scene:contains_object("object"))

  /// Removes an object from the scene.
  // @function Scene:remove_object
  // @tparam SceneObject object The object that should be removed from the scene
  // @usage local obj = scene:add_object("object")
  // assert(scene:contains_object("object"))
  // scene:remove_object(obj)
  // assert(not scene:contains_object("object"))
  // assert(obj == nil)
  scene_type["remove_object"] =
  sol::overload(
    sol::resolve<void(std::string_view)>(&Scene::DeleteObject),
    sol::resolve<void(SceneObject*)>(&Scene::DeleteObject)
  );

  /// Checks whether an object exists.
  // @function Scene:contains_object
  // @string name The name of the object to check
  // @treturn bool
  // @usage local obj = scene:add_object("object")
  // assert(scene:contains_object("object"))
  // scene:remove_object(obj)
  // assert(not scene:contains_object("object"))
  scene_type["contains_object"] = &Scene::ContainsObject;

  /// Returns an iterator to all objects in the scene
  // @function objects
  // @usage for obj in scene:objects() do
  //   core.log(obj)
  // end
  scene_type["objects"] = [](Scene& scene) {
    return [&scene, object_index = std::make_shared<size_t>(0)]() -> safe_ptr<SceneObject> {
      if (*object_index < scene.objects_.size()) {
        SceneObject* object = std::next(scene.objects_.begin(), *object_index)->second.get();
        (*object_index)++;
        return safe_ptr(object);
      } else {
        return nullptr;
      }
    };
  };

  /// Returns an iterator to all controllers in the scene
  // The iterator returns the controllers in the @{02-scene-structure|update order}.
  // @function Scene:controllers
  // @usage for obj in scene:objects() do
  //   core.log(obj)
  // end
  // scene_type["controllers"] = [](Scene& scene) {
  //   return [&scene, object_index = std::make_shared<size_t>(0)]() -> SceneObject* {
  //     if (*object_index < scene.objects_.size()) {
  //       SceneObject* object = std::next(scene.objects_.begin(), *object_index)->second.get();
  //       (*object_index)++;
  //       return object;
  //     } else {
  //       return nullptr;
  //     }
  //   };
  // };

  // clang-format on
}

}  // namespace ovis
