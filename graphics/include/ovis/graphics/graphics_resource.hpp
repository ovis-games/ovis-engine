#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>

#include <ovis/utils/versioned_index.hpp>

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
  using Id = VersionedIndex<std::uint16_t, 2>;

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
};

}  // namespace ovis
