#include <iterator>
#include <map>
#include <string>

#include <SDL2/SDL_assert.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/core/log.hpp>
#include <ovis/core/utf8.hpp>
#include <ovis/scene/scene.hpp>
#include <ovis/scene/scene_controller.hpp>
#include <ovis/scene/scene_object.hpp>
#include <ovis/scene/script_scene_controller.hpp>
#include <ovis/core/lua.hpp>

namespace ovis {

const json Scene::schema_ = {{"$ref", "engine#/$defs/scene"}};

Scene::Scene() {}

Scene::~Scene() {}

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

SceneObject* Scene::CreateObject(const std::string& object_name) {
  std::string candidate_name = object_name;
  int number = 1;
  while (ContainsObject(candidate_name)) {
    ++number;
    candidate_name = object_name + std::to_string(number);
  }
  auto result = objects_.insert(std::make_pair(candidate_name, std::make_unique<SceneObject>(this, candidate_name)));
  SDL_assert(result.second);
  return result.first->second.get();
}

SceneObject* Scene::CreateObject(const std::string& object_name, const json& serialized_object) {
  auto object = CreateObject(object_name);
  object->Deserialize(serialized_object);
  return object;
}

void Scene::DeleteObject(const std::string& object_name) {
  SDL_assert(objects_.count(object_name) == 1);
  objects_.erase(object_name);
}

void Scene::ClearObjects() {
  objects_.clear();
}

SceneObject* Scene::GetObject(const std::string& object_name) {
  auto object = objects_.find(object_name);
  return object->second.get();
}

bool Scene::ContainsObject(const std::string& object_name) {
  return objects_.count(object_name) == 1;
}

void Scene::GetObjects(std::vector<SceneObject*>* scene_objects, bool sort_by_name) const {
  scene_objects->clear();
  scene_objects->reserve(objects_.size());
  for (auto& name_object_pair : objects_) {
    scene_objects->push_back(name_object_pair.second.get());
  }
  if (sort_by_name) {
    std::sort(scene_objects->begin(), scene_objects->end(),
              [](SceneObject* left, SceneObject* right) { return to_lower(left->name()) < to_lower(right->name()); });
  }
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

void Scene::DrawImGui() {
  for (const auto& controller : controller_order_) {
    controller->DrawImGui();
  }
}

json Scene::Serialize() const {
  json serialized_object = {{"version", "0.1"}};
  auto& controllers = serialized_object["controllers"] = json::object();
  for (const auto& controller : controllers_) {
    controllers[controller.first] = json::object();
  }

  auto& objects = serialized_object["objects"] = json::object();
  for (const auto& object : objects_) {
    objects[object.first] = object.second->Serialize();
  }

  return serialized_object;
}

bool Scene::Deserialize(const json& serialized_object) {
  if (!serialized_object.contains("version") || serialized_object["version"] != "0.1") {
    LogE("Invalid scene object. Version must be 0.1!");
    return false;
  }

  ClearControllers();
  ClearObjects();

  if (serialized_object.contains("controllers") && serialized_object["controllers"].is_object()) {
    for (const auto& controller : serialized_object["controllers"].items()) {
      // SDL_assert(std::find(SceneController::GetRegisteredControllers().begin(),
      //                      SceneController::GetRegisteredControllers().end(),
      //                      controller.key()) != SceneController::GetRegisteredControllers().end());
      LogV("Adding controller '{}'", controller.key());
      AddController(controller.key());
    }
  }

  if (serialized_object.contains("objects") && serialized_object["objects"].is_object()) {
    for (const auto& object : serialized_object["objects"].items()) {
      CreateObject(object.key(), object.value());
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
  std::set<std::string> controllers_left_;

  for (const auto& name_controller_pair : controllers_) {
    controllers_left_.insert(name_controller_pair.first);

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
  controller_order_.clear();
  controller_order_.reserve(controllers_.size());
  while (controllers_left_.size() > 0) {
    auto next = std::find_if(controllers_left_.begin(), controllers_left_.end(),
                             [&dependencies](const std::string& value) { return dependencies.count(value) == 0; });

    SDL_assert(next != controllers_left_.end());
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
    controllers_left_.erase(next);
  }

  LogV("Controllers sorted!");
  controllers_sorted_ = true;
}

SceneController* Scene::GetControllerInternal(const std::string& controller_name) const {
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
  // @classmod ovis.engine.Scene
  // @testinginclude <ovis/scene/scene.hpp>
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
  scene_type["add_object"] = sol::resolve<SceneObject*(const std::string&)>(&Scene::CreateObject);

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
  // -- do not use obj anymore!
  scene_type["remove_object"] =
  sol::overload(
    &Scene::DeleteObject,
    [](Scene* scene, SceneObject* object){
      if (scene != nullptr && object != nullptr) {
        scene->DeleteObject(object->name());
      }
  });

  /// Checks whether an object exists.
  // @function Scene:contains_object
  // @string name The name of the object to check
  // @treturn bool
  // @usage local obj = scene:add_object("object")
  // assert(scene:contains_object("object"))
  // scene:remove_object(obj)
  // assert(not scene:contains_object("object"))
  // -- do not use obj anymore!
  scene_type["contains_object"] = &Scene::ContainsObject;

  /// Returns an iterator to all objects in the scene
  // @field Scene.objects
  // @usage for obj in scene:objects() do
  //   core.log(obj)
  // end
  scene_type["objects"] = [](Scene& scene) {
    return [&scene, object_index = std::make_shared<size_t>(0)]() -> SceneObject* {
      if (*object_index < scene.objects_.size()) {
        SceneObject* object = std::next(scene.objects_.begin(), *object_index)->second.get();
        (*object_index)++;
        return object;
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
  scene_type["controllers"] = [](Scene& scene) {
    return [&scene, object_index = std::make_shared<size_t>(0)]() -> SceneObject* {
      if (*object_index < scene.objects_.size()) {
        SceneObject* object = std::next(scene.objects_.begin(), *object_index)->second.get();
        (*object_index)++;
        return object;
      } else {
        return nullptr;
      }
    };
  };

  // clang-format on
}

}  // namespace ovis
