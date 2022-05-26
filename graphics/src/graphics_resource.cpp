#include <SDL2/SDL_assert.h>

#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/graphics_resource.hpp>

namespace ovis {

GraphicsResource::~GraphicsResource() {
  const size_t index = id().index;
  SDL_assert(context_->resources_[index] == this);
  context_->resources_[index] = new GraphicsResource(context_, id());
}

GraphicsResource::GraphicsResource(GraphicsContext* context, Type type) : context_(context), type_(type) {
  SDL_assert(context != nullptr);
  SDL_assert(type != Type::NONE);

  for (size_t index = 0; index < context->resources_.size(); ++index) {
    GraphicsResource* resource = context->resources_[index];
    SDL_assert(resource != nullptr);

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
  SDL_assert(context != nullptr);
}

}  // namespace ovis
