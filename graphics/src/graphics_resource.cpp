#include "ovis/graphics/graphics_resource.hpp"

#include "ovis/graphics/graphics_context.hpp"

namespace ovis {

GraphicsResource::~GraphicsResource() {
  const size_t index = id().index;
  assert(context_->resources_[index] == this);
  context_->resources_[index] = new GraphicsResource(context_, id());
}

GraphicsResource::GraphicsResource(GraphicsContext* context, Type type) : context_(context), type_(type) {
  assert(context != nullptr);
  assert(type != Type::NONE);

  for (size_t index = 0; index < context->resources_.size(); ++index) {
    GraphicsResource* resource = context->resources_[index];
    assert(resource != nullptr);

    if (resource->type() == Type::NONE) {
      id_ = resource->id().next();
      delete resource;
      context->resources_[index] = this;
      return;
    }
  }

  id_ = Id(context->resources_.size());
  context->resources_.push_back(this);
}

GraphicsResource::GraphicsResource(GraphicsContext* context, Id id) : context_(context), id_(id), type_(Type::NONE) {
  assert(context != nullptr);
}

}  // namespace ovis
