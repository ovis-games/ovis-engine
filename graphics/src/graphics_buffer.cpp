#include "ovis/graphics/graphics_buffer.hpp"

namespace ovis {

GraphicsBuffer::GraphicsBuffer(GraphicsContext* context, Type type) : GraphicsResource(context, type), name_(0) {
  glGenBuffers(1, &name_);
  assert(name_ != 0);
}

GraphicsBuffer::~GraphicsBuffer() {
  // Bound check is done in derived classes
  glDeleteBuffers(1, &name_);
}

}  // namespace ovis
