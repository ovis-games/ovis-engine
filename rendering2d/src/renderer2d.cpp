#include <ovis/core/asset_library.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/render_target_configuration.hpp>
#include <ovis/rendering/rendering_viewport.hpp>
#include <ovis/rendering/clear_pass.hpp>
#include <ovis/rendering2d/renderer2d.hpp>

namespace ovis {

Renderer2D::Renderer2D() {
  RenderAfter<ClearPass>();
}

void Renderer2D::CreateResources() {
  Texture2DDescription empty_texture_desc {
    .width = 1,
    .height = 1,
    .mip_map_count = 1,
    .format = TextureFormat::RGBA_UINT8,
    .filter = TextureFilter::POINT
  };

  shape_shader_ = LoadShaderProgram("shape2d", context());

  VertexBufferDescription buffer_desc;
  buffer_desc.vertex_size_in_bytes = sizeof(Shape2D::Vertex);
  buffer_desc.size_in_bytes = sizeof(Shape2D::Vertex) * VERTEX_BUFFER_ELEMENT_COUNT;
  vertex_buffer_ = std::make_unique<ovis::VertexBuffer>(context(), buffer_desc);

  VertexInputDescription vertex_input_desc;
  vertex_input_desc.vertex_buffers = {vertex_buffer_.get()};
  vertex_input_desc.vertex_attributes = {
      {*shape_shader_->GetAttributeLocation("Position"), 0, 0, VertexAttributeType::FLOAT32_VECTOR2},
      {*shape_shader_->GetAttributeLocation("Color"), 8, 0, VertexAttributeType::UINT8_NORM_VECTOR4}};
  vertex_input_ = std::make_unique<VertexInput>(context(), vertex_input_desc);

  const uint32_t white_pixel = 0xffffffff;
  empty_texture_ = std::make_unique<Texture2D>(context(), empty_texture_desc, &white_pixel);
}

void Renderer2D::ReleaseResources() {
  vertex_buffer_.reset();
  vertex_input_.reset();
  shape_shader_.reset();
  empty_texture_.reset();
}

void Renderer2D::Render(const RenderContext& render_context) {
  object_cache_.clear();
  object_cache_.insert(object_cache_.end(), viewport()->scene()->ObjectsWithComponent<Shape2D>().begin(),
                       viewport()->scene()->ObjectsWithComponent<Shape2D>().end());

  auto& objects_with_shape2d = object_cache_;
  std::sort(objects_with_shape2d.begin(), objects_with_shape2d.end(), [](SceneObject* lhs, SceneObject* rhs) {
    Vector3 lhs_position;
    Transform* lhs_transform = lhs->GetComponent<Transform>();
    if (lhs_transform != nullptr) {
      lhs_position = lhs_transform->world_position();
    }

    Vector3 rhs_position;
    Transform* rhs_transform = rhs->GetComponent<Transform>();
    if (rhs_transform != nullptr) {
      rhs_position = rhs_transform->world_position();
    }

    // TODO: project into camera view axis instead of using the z coordinates
    return lhs_position.z > rhs_position.z;
  });

  for (SceneObject* object : objects_with_shape2d) {
    Shape2D* shape = object->GetComponent<Shape2D>();
    const std::span<const Shape2D::Vertex> vertices = shape->vertices();

    if (shape_vertex_count_ + vertices.size() > VERTEX_BUFFER_ELEMENT_COUNT) {
      DrawShapeVertices();
    }

    const std::string texture_asset = shape->texture_asset();
    Texture2D* texture = empty_texture_.get();
    if (texture_asset.size() > 0) {
      auto texture_iterator = textures_.find(texture_asset);
      if (texture_iterator == textures_.end()) {
        texture_iterator = textures_.insert(std::make_pair(texture_asset, LoadTexture2D(texture_asset, context()))).first;
      }
      texture = texture_iterator->second.get();
    }

    Transform* transform = object->GetComponent<Transform>();
    const Matrix4 world_view_projection =
        transform ? AffineCombine(render_context.world_to_clip_space,
                                  transform->local_to_world_matrix())
                  : render_context.world_to_clip_space;
    for (size_t i = 0; i < vertices.size(); ++i) {
      const Vector2 transformed_position = TransformPosition(world_view_projection, Vector3(vertices[i].x, vertices[i].y, 0.0f));
      shape_vertices_[shape_vertex_count_ + i].x = transformed_position.x;
      shape_vertices_[shape_vertex_count_ + i].y = transformed_position.y;
      shape_vertices_[shape_vertex_count_ + i].color = vertices[i].color;
    }
    shape_vertex_count_ += vertices.size();
  }
  DrawShapeVertices();
}

void Renderer2D::DrawShapeVertices() {
  if (shape_vertex_count_ == 0) {
    return;
  }
  SDL_assert(shape_vertex_count_ % 3 == 0);

  vertex_buffer_->Write(0, shape_vertex_count_ * sizeof(Shape2D::Vertex), shape_vertices_.data());

  DrawItem draw_item;
  draw_item.vertex_input = vertex_input_.get();
  draw_item.shader_program = shape_shader_.get();
  draw_item.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
  draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  draw_item.count = shape_vertex_count_;
  draw_item.blend_state.enabled = true;
  draw_item.blend_state.source_color_factor = SourceBlendFactor::SOURCE_ALPHA;
  draw_item.blend_state.destination_color_factor = DestinationBlendFactor::ONE_MINUS_SOURCE_ALPHA;
  context()->Draw(draw_item);

  shape_vertex_count_ = 0;
}

}  // namespace ovis
