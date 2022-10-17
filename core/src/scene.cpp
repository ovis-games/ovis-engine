#include "ovis/core/scene.hpp"

#include <iterator>
#include <map>
#include <string>

#include "ovis/utils/log.hpp"
#include "ovis/utils/utf8.hpp"
#include "ovis/core/asset_library.hpp"
#include "ovis/core/entity.hpp"
#include "ovis/core/lua.hpp"
#include "ovis/core/scene_controller.hpp"
#include "ovis/core/scene_viewport.hpp"

namespace ovis {

const json Scene::schema_ = {{"$ref", "engine#/$defs/scene"}};

Scene::Scene(std::size_t initial_object_capacity) {
  // TODO: what is this? do we need this?
  event_handler_index_ = RegisterGlobalEventHandler([this](Event* event) { ProcessEvent(event); });

  entities_.reserve(initial_object_capacity);
  for (std::size_t i = 0; i < initial_object_capacity; ++i) {
    entities_.emplace_back(this, Entity::Id(i));
  }
}

Scene::~Scene() {
  controllers_.clear();
  removed_controllers_.clear();
  DeregisterGlobalEventHandler(event_handler_index_);
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
  assert(insert_return_value.second);
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


Entity* Scene::CreateEntity(std::string_view object_name, std::optional<Entity::Id> parent) {
  assert(!parent || IsEntityIdValid(*parent));

  for (std::size_t i = parent ? parent->index + 1 : 0; i < entities_.size(); ++i) {
    if (!entities_[i].is_alive()) {
      entities_[i].Wake(object_name, parent);
      return &entities_[i];
    }
  }
  // TODO: check if an object with that name already exists

  return nullptr;
}

Entity* Scene::GetEntity(Entity::Id id) {
  return IsEntityIdValid(id) ? &entities_[id.index] : nullptr;
}

void Scene::DeleteEntity(Entity::Id id) {
  assert(IsEntityIdValid(id));
  assert(entities_[id.index].is_alive());
  entities_[id.index].Kill();
}

void Scene::ClearEntities() {
  for (auto& object : entities_) {
    if (object.is_alive()) {
      object.Kill();
    }
  }
}

std::set<TypeId> Scene::GetUsedEntityComponentTypes() const {
  std::set<TypeId> types;
  for (const auto& [_, controller] : controllers_) {
    for (const auto type : controller->read_access_components()) {
      types.insert(type);
    }
    for (const auto type : controller->write_access_components()) {
      types.insert(type);
    }
  }
  return types;
}

ComponentStorage* Scene::GetComponentStorage(TypeId component_type) {
  for (auto& storage : component_storages_) {
    if (storage.component_type_id() == component_type) {
      return &storage;
    }
  }
  return nullptr;
}

// void Scene::ClearEntities() {
//   while (entities_.size() > 0) {
//     // Destroy one at a time because parent objects will explicitly destory their children
//     // TODO: maybe set a special flag in the scene that we are already destructing?
//     entities_.erase(entities_.begin());
//   }
// }

// Entity* Scene::GetEntity(std::string_view object_path) {
//   auto object = entities_.find(std::string(object_path));
//   if (object == entities_.end()) {
//     return nullptr;
//   }
//   { return object->second.get(); }
// }

// bool Scene::ContainsEntity(std::string_view object_reference) {
//   return entities_.count(std::string(object_reference));
// }
//

void Scene::Prepare() {
  component_storages_.clear();
  {
    const auto object_component_types = GetUsedEntityComponentTypes();
    component_storages_.reserve(object_component_types.size());
    for (const auto component_type : object_component_types) {
      component_storages_.emplace_back(this, component_type, entities_.size());
    }
  }
}

void Scene::Play() {
  assert(!is_playing());
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->Play();
  }
  is_playing_ = true;
}

void Scene::Stop() {
  assert(is_playing());
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
  assert(is_playing() && "Call Play() before calling Update().");
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->BeforeUpdate();
  }
}

void Scene::AfterUpdate() {
  assert(is_playing() && "Call Play() before calling Update().");
  if (!controllers_sorted_) {
    SortControllers();
  }
  for (const auto& controller : controller_order_) {
    controller->AfterUpdate();
  }
  DeleteRemovedControllers();
}

void Scene::Update(std::chrono::microseconds delta_time) {
  assert(is_playing() && "Call Play() before calling Update().");
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
//   auto& controllers = serialized_object["controllers"] = json::object();
//   for (const auto& controller : controllers_) {
//     controllers[controller.first] = controller.second->Serialize();
//   }

//   auto& objects = serialized_object["objects"] = json::object();
//   for (const auto& object : entities_) {
//     if (object.second->parent() == nullptr) {
//       objects[object.first] = object.second->Serialize();
//     }
//   }

//   Camera* camera = main_viewport() ? main_viewport()->camera() : nullptr;
//   if (camera != nullptr) {
//     SDL_assert(camera->scene_object() != nullptr);
//     serialized_object["camera"] = camera->scene_object()->name();
//   }
//   // if (camera_object_.size() > 0) {
//   //   serialized_object["camera"] = camera_object_;
//   // }

  return serialized_object;
}

bool Scene::Deserialize(const json& serialized_object) {
//   if (!serialized_object.contains("version") || serialized_object["version"] != "0.1") {
//     LogE("Invalid scene object. Version must be 0.1!");
//     return false;
//   }

//   ClearEntities();
//   ClearControllers();

//   if (serialized_object.contains("controllers") && serialized_object["controllers"].is_object()) {
//     for (const auto& controller : serialized_object["controllers"].items()) {
//       AddController(controller.key(), controller.value());
//     }
//   }

//   if (serialized_object.contains("objects") && serialized_object["objects"].is_object()) {
//     for (const auto& object : serialized_object["objects"].items()) {
//       CreateEntity(object.key(), object.value());
//     }
//   }

//   if (serialized_object.contains("camera")) {
//     // camera_object_ = serialized_object.at("camera");
//     if (main_viewport() != nullptr) {
//       // Need to copy out variable here as nlohmann json currently does not support direct
//       // conversion to std::string_view
//       const std::string camera_object_reference = serialized_object.at("camera");
//       Entity* object = GetEntity(camera_object_reference);
//       if (object != nullptr && object->HasComponent<Camera>()) {
//         main_viewport()->SetCamera(object->GetComponent<Camera>());
//         LogD("Setting Camera");
//       } else {
//         main_viewport()->SetCamera(nullptr);
//         LogD("Setting Camera to null");
//       }
//     }
//   }

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

    assert(next != controllers_left.end());
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

}  // namespace ovis
