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

using EntityIdVersionedIndex = VersionedIndex<uint32_t, 15, 16, 1>;

struct EntityId : EntityIdVersionedIndex {
  template <typename... T>
  requires(std::is_constructible_v<EntityIdVersionedIndex, T...>)
  constexpr EntityId(T&&... arguments) : VersionedIndex(std::forward<T>(arguments)...) {}

  bool is_active() const {
    return flags != 0;
  }

  constexpr static EntityId CreateInactive(uint32_t index) {
    return EntityId(index, 0, 0);
  }
};

struct Entity {
  // Iterators
  struct SiblingIterator;
  struct AscendingSiblingIterator;
  struct DescendantIterator;

  // Utility structs
  struct Siblings;
  struct Children;
  struct Descendants;

  // Fields
  EntityId id;
  EntityId parent_id;
  EntityId first_children_id;
  EntityId previous_sibling_id;
  EntityId next_sibling_id;
  std::string name;

  // Methods
  bool is_active() const {
    return id.is_active();
  }

  void activate() {
    assert(!is_active());
    id.flags = 1;
  }

  void deactivate() {
    assert(is_active());
    id.flags = 0;
  }

  bool has_parent() const {
    return parent_id != id;
  }
  Entity* parent() const;

  bool has_children() const {
    return first_children_id != id;
  }
  Children children(Scene* scene);

  bool has_siblings() const {
    return next_sibling_id != id;
  }
  Siblings siblings(Scene* scene);

  Descendants descendants(Scene* scene);

  static bool IsValidName(std::string_view name);
  static std::pair<std::string_view, std::optional<unsigned int>> ParseName(std::string_view name);

  OVIS_VM_DECLARE_TYPE_BINDING();
};

#if OVIS_EMSCRIPTEN
  static_assert(sizeof(Entity) == 32);
#endif

struct EntityIterator {
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = Entity;
  using pointer = Entity*;
  using reference = Entity&;
};

struct Entity::SiblingIterator : EntityIterator {
  Entity& operator*() const { return *entity; }
  Entity* operator->() { return entity; }
  SiblingIterator& operator++() {
    Increment();
    return *this;
  }
  SiblingIterator operator++(int) {
    auto current = *this;
    Increment();
    return current;
  }
  SiblingIterator& operator--() {
    Decrement();
    return *this;
  }
  SiblingIterator operator--(int) {
    auto current = *this;
    Decrement();
    return current;
  }

  Scene* scene;
  Entity* entity;

  void Increment();
  void Decrement();
};

inline bool operator==(const Entity::SiblingIterator& lhs, const Entity::SiblingIterator& rhs) {
  // We can assume that both are from the same scene as stated in C++11 standard (n3337):
  // § 24.2.1 — [iterator.requirements.general#6]
  // An iterator j is called reachable from an iterator i if and only if there is a finite
  // sequence of applications of the expression ++i that makes i == j. If j is reachable from i,
  // they refer to elements of the same sequence.

  // § 24.2.5 — [forward.iterators#2]
  // The domain of == for forward iterators is that of iterators over the same underlying sequence.
  assert(lhs.scene == rhs.scene);

  return lhs.entity == rhs.entity;
}

inline bool operator!=(const Entity::SiblingIterator& lhs, const Entity::SiblingIterator& rhs) {
  assert(lhs.scene == rhs.scene);  // See above
  return lhs.entity != rhs.entity;
}


Entity::SiblingIterator begin(const Entity::Siblings& siblings);
Entity::SiblingIterator end(const Entity::Siblings& siblings);

static_assert(std::bidirectional_iterator<Entity::SiblingIterator>);

struct Entity::AscendingSiblingIterator : EntityIterator {
  Entity& operator*() const { return *entity; }
  Entity* operator->() { return entity; }
  AscendingSiblingIterator& operator++() {
    Increment();
    return *this;
  }
  AscendingSiblingIterator operator++(int) {
    auto current = *this;
    Increment();
    return current;
  }

  Scene* scene;
  Entity* entity;

  void Increment();
};

inline bool operator==(const Entity::AscendingSiblingIterator& lhs, const Entity::AscendingSiblingIterator& rhs) {
  assert(lhs.scene == rhs.scene);  // See above
  return lhs.entity == rhs.entity;
}

inline bool operator!=(const Entity::AscendingSiblingIterator& lhs, const Entity::AscendingSiblingIterator& rhs) {
  assert(lhs.scene == rhs.scene);  // See above
  return lhs.entity != rhs.entity;
}

static_assert(std::forward_iterator<Entity::AscendingSiblingIterator>);

struct Entity::DescendantIterator : EntityIterator {
  Entity& operator*() const { return *current_descendant; }
  Entity* operator->() { return current_descendant; }
  DescendantIterator& operator++() {
    Increment();
    return *this;
  }
  DescendantIterator operator++(int) {
    auto current = *this;
    Increment();
    return current;
  }

  Scene* scene;
  Entity* root;
  Entity* current_descendant;

  void Increment();
};

inline bool operator==(const Entity::DescendantIterator& lhs, const Entity::DescendantIterator& rhs) {
  assert(lhs.scene == rhs.scene);  // See above
  assert(lhs.root == rhs.root);
  return lhs.current_descendant == rhs.current_descendant;
}

inline bool operator!=(const Entity::DescendantIterator& lhs, const Entity::DescendantIterator& rhs) {
  assert(lhs.scene == rhs.scene);  // See above
  assert(lhs.root == rhs.root);
  return lhs.current_descendant != rhs.current_descendant;
}

static_assert(std::forward_iterator<Entity::DescendantIterator>);

struct Entity::Siblings {
  Scene* scene;
  Entity* entity;

  Entity::SiblingIterator begin() const;
  Entity::SiblingIterator end() const;
};

struct Entity::Children {
  Scene* scene;
  Entity* entity;

  Entity::AscendingSiblingIterator begin() const;
  Entity::AscendingSiblingIterator end() const;
};

struct Entity::Descendants {
  Scene* scene;
  Entity* root;

  Entity::DescendantIterator begin() const;
  Entity::DescendantIterator end() const;
};

}  // namespace ovis
