#pragma once

#include <cassert>
#include <limits>
#include <concepts>

// A VersionedIndex is meant to be used as an id for resources that are stored in a vector-like structure. It consists
// of an index that specifies the offset in the vector and a version that indicates whether the resources is the one
// that is meant to be referenced or if the resource slot has been reused. E.g., one could have a vector of loaded
// textures and the textures are referenced at other places via an index. If now the texture at index 2 gets deleted and
// a new texture is loaded, it may resuse the slot at index 2. However, when only storing the index at other places you
// are not able to detect that the texture has changed. Thus, additional bits are added that store the 'version' of the
// resource, which will be increased whenever a slot is reused. By comparing the versions you can then detect references
// to no longer existing resources.
//
// T specifies the underlying type of the index (must be an unsigned integer) and VERSION_BITS specifies the number of
// bits that should be reserved for versioning. E.g., a VersionedIndex<uint32_t, 2> would have a 30 bit index with 2
// bits of versioning. Thus, the version can range from 0 to 3 and would wrap around afterwards. So, if you reuse the
// same slot by a multiple of 4 times you cannot detect resource changes. So you should carefully choose the number of
// versioning bits based on how often resources are replaced, how often you validate the ids and what impact a false
// negative would have.

namespace ovis {

template <typename T>
concept IndexType = std::is_unsigned_v<T> && !std::is_same_v<T, bool>;

template <IndexType T, std::size_t VERSION_BITS, std::size_t INDEX_BITS = std::numeric_limits<T>::digits - VERSION_BITS>
struct VersionedIndex {
  static_assert(VERSION_BITS <= std::numeric_limits<T>::digits);
  static_assert(INDEX_BITS <= std::numeric_limits<T>::digits);
  static_assert(VERSION_BITS + INDEX_BITS <= std::numeric_limits<T>::digits);

  static constexpr T NextVersion(T current_version) {
    return (current_version + 1) & ((1 << VERSION_BITS) - 1);
  }

  VersionedIndex() = default;

  constexpr VersionedIndex(T index) : index(index), version(0) {
    assert(index < (1 << INDEX_BITS));
  }
  constexpr VersionedIndex(T index, T version) : index(index), version(version) {
    assert(index < (1 << INDEX_BITS));
    assert(version < (1 << VERSION_BITS));
  }

  constexpr VersionedIndex<T, VERSION_BITS, INDEX_BITS> next() const {
    return VersionedIndex<T, VERSION_BITS, INDEX_BITS>(index, NextVersion(version));
  }

  T index : INDEX_BITS;
  T version : VERSION_BITS;
};

template <IndexType T, std::size_t VERSION_BITS, std::size_t INDEX_BITS>
inline bool operator==(VersionedIndex<T, VERSION_BITS, INDEX_BITS> lhs, VersionedIndex<T, VERSION_BITS, INDEX_BITS> rhs) {
  return lhs.index == rhs.index && lhs.version == rhs.version;
}

template <IndexType T, std::size_t VERSION_BITS, std::size_t INDEX_BITS>
inline bool operator!=(VersionedIndex<T, VERSION_BITS, INDEX_BITS> lhs, VersionedIndex<T, VERSION_BITS, INDEX_BITS> rhs) {
  return lhs.index != rhs.index || lhs.version != rhs.version;
}

}
