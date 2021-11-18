#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>

#include <SDL2/SDL_assert.h>

namespace ovis {

class GraphicsContext;

class GraphicsResource {
  friend class GraphicsContext;

 public:
  enum class Type : std::uint8_t {
    NONE,
    VERTEX_BUFFER,
    VERTEX_INPUT,
    INDEX_BUFFER,
    SHADER_PROGRAM,
    UNIFORM_BUFFER,
    TEXTURE_2D,
    TEXTURE_CUBE,
    RENDER_TARGET,
    RENDER_TARGET_CONFIGURATION,
    QUERY,
  };
  using Id = std::uint16_t;

  inline GraphicsContext* context() const { return context_; }
  virtual ~GraphicsResource();

  Type type() const { return type_; }
  Id id() const { return id_; }

 protected:
  GraphicsResource(GraphicsContext* context, Type type);

 private:
  GraphicsContext* context_;
  Id id_;
  Type type_;

  // Create dummy resource with type = NONE
  GraphicsResource(GraphicsContext* context, Id id);

  // ID functionality, ids are stored with the version in the high and the index in the low bits:
  // [VERSION INDEX]
  static constexpr unsigned int ID_BITS = std::numeric_limits<Id>::digits;
  static constexpr unsigned int VERSION_BITS = 2;
  static constexpr unsigned int INDEX_BITS = ID_BITS - VERSION_BITS;
  static constexpr std::uint16_t INDEX_MASK = (1 << INDEX_BITS) - 1;
  static constexpr std::uint16_t VERSION_MASK = ((1 << VERSION_BITS) - 1) << INDEX_BITS;
  static_assert((INDEX_MASK & VERSION_MASK) == 0, "Index and version mask are overlapping");
  static_assert((INDEX_MASK | VERSION_MASK) == std::numeric_limits<Id>::max(),
                "Index and version mask should fill the whole id");
  static inline size_t ExtractIndex(Id id) { return id & INDEX_MASK; }
  static inline size_t ExtractVersion(Id id) { return (id & VERSION_MASK) >> INDEX_BITS; }
  static inline Id CreateId(size_t index, size_t old_version) {
    SDL_assert(index <= INDEX_MASK);
    SDL_assert(old_version < (1 << VERSION_BITS));
    const size_t new_version = (old_version + 1) & ((1 << VERSION_BITS) - 1);
    return (new_version << INDEX_BITS) | index;
  }
  static inline Id CreateId(size_t index) {
    SDL_assert(index <= INDEX_MASK);
    return index;
  }
};

}  // namespace ovis
