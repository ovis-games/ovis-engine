#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include "ovis/utils/versioned_index.hpp"
#include "ovis/core/vm_bindings.hpp"

namespace ovis {

class Scene;

class Entity {
  friend class Scene;

 public:
  using Id = VersionedIndex<uint32_t, 16>;

  Entity(Scene* scene, Id id);

  Scene* scene() const { return scene_; }
  Id id() const { return id_; }
  bool is_alive() const { return is_alive_; }
  std::string_view name() const { return name_; }
  std::string_view path() const { return path_; }
  std::optional<Id> parent_id() const { return parent_id_; }
  Entity* parent() const;

  static bool IsValidName(std::string_view name);
  static std::pair<std::string_view, std::optional<unsigned int>> ParseName(std::string_view name);

 private:
  Id id_;
  bool is_alive_;
  Scene* scene_;
  std::optional<Id> parent_id_;
  std::set<Id> children_ids_;  // TODO: should use a vector here but I am too lazy right now
  std::string name_;
  std::string path_;

  void Wake(std::string_view name, std::optional<Entity::Id> parent = std::nullopt);
  void Kill();

  OVIS_VM_DECLARE_TYPE_BINDING();
};

}  // namespace ovis
