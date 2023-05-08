#include "ovis/core/asset_library.hpp"
#include "ovis/core/transform.hpp"
#include "ovis/graphics/graphics_context.hpp"
#include "ovis/graphics/render_target_configuration.hpp"
#include "ovis/rendering/clear_pass.hpp"
#include "ovis/rendering2d/renderer2d.hpp"
#include "ovis/rendering2d/shape2d.hpp"
#include "ovis/rendering2d/text.hpp"

namespace ovis {

Renderer2D::Renderer2D(GraphicsContext* graphics_context) : RenderPass("Renderer2D", graphics_context) {
  ExecuteAfter("ClearPass");
  RequireReadAccess<Shape2D>();
  RequireReadAccess<Text>();
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
      {*shape_shader_->GetAttributeLocation("TextureCoordinates"), 8, 0, VertexAttributeType::FLOAT32_VECTOR2},
      {*shape_shader_->GetAttributeLocation("Color"), 16, 0, VertexAttributeType::UINT8_NORM_VECTOR4}};
  vertex_input_ = std::make_unique<VertexInput>(context(), vertex_input_desc);

  const uint32_t white_pixel = 0xffffffff;
  empty_texture_ = std::make_unique<Texture2D>(context(), empty_texture_desc, &white_pixel);

  font_atlases_.push_back(FontAtlas(context(), "NotoSans-Regular", 32.0f));
}

void Renderer2D::ReleaseResources() {
  vertex_buffer_.reset();
  vertex_input_.reset();
  shape_shader_.reset();
  empty_texture_.reset();
}

void Renderer2D::Render(const SceneUpdate& update, const SceneViewport& viewport) {
  // // auto& objects_with_shape2d = object_cache_;
  // std::sort(object_cache_.begin(), object_cache_.end(), [](SceneObject* lhs, SceneObject* rhs) {
  //   Vector3 lhs_position;
  //   Transform* lhs_transform = lhs->GetComponent<Transform>();
  //   if (lhs_transform != nullptr) {
  //     lhs_position = lhs_transform->world_position();
  //   }

  //   Vector3 rhs_position;
  //   Transform* rhs_transform = rhs->GetComponent<Transform>();
  //   if (rhs_transform != nullptr) {
  //     rhs_position = rhs_transform->world_position();
  //   }

  //   // TODO: project into camera view axis instead of using the z coordinates
  //   return lhs_position.z > rhs_position.z;
  // });
  //
  // auto trnsform_storage = 

  auto shape_storage = update.scene->GetComponentStorage<Shape2D>();
  auto text_storage = update.scene->GetComponentStorage<Text>();

  Texture2D* bound_texture = nullptr;
  for (Entity& entity : *update.scene) {
    // Transform* transform = object->GetComponent<Transform>();
    // const Matrix4 world_to_clip_space =
    //     transform ? AffineCombine(render_context.world_to_clip_space, transform->local_to_world_matrix())
    //               : render_context.world_to_clip_space;
    const Matrix4 world_to_clip_space = Matrix4::Identity();

    if (shape_storage.EntityHasComponent(entity.id)) {
      const Shape2D& shape = shape_storage[entity.id];
      const std::span<const Shape2D::Vertex> vertices = shape.vertices();

      const std::string texture_asset = shape.texture_asset();
      Texture2D* texture = empty_texture_.get();
      if (texture_asset.size() > 0) {
        auto texture_iterator = textures_.find(texture_asset);
        if (texture_iterator == textures_.end()) {
          texture_iterator = textures_.insert(std::make_pair(texture_asset, LoadTexture2D(texture_asset, context()))).first;
        }
        texture = texture_iterator->second.get();
      }

      if (texture != bound_texture) {
        DrawShapeVertices();
        shape_shader_->SetTexture("Texture", texture);
        bound_texture = texture;
      } else if (shape_vertex_count_ + vertices.size() > VERTEX_BUFFER_ELEMENT_COUNT) {
        DrawShapeVertices();
      }

      for (size_t i = 0; i < vertices.size(); ++i) {
        const Vector2 transformed_position = TransformPosition(world_to_clip_space, Vector3(vertices[i].x, vertices[i].y, 0.0f));
        shape_vertices_[shape_vertex_count_ + i].x = transformed_position.x;
        shape_vertices_[shape_vertex_count_ + i].y = transformed_position.y;
        shape_vertices_[shape_vertex_count_ + i].color = vertices[i].color;
      }
      shape_vertex_count_ += vertices.size();
    }

    if (text_storage.EntityHasComponent(entity.id)) {
      const Text& text = text_storage[entity.id];

      DrawShapeVertices(); // TODO: check if texture different
      Vector2 position = Vector2::Zero();

      if (Texture2D* texture = font_atlases_[0].texture(); texture != bound_texture) {
        DrawShapeVertices();
        shape_shader_->SetTexture("Texture", texture);
        bound_texture = texture;
      }
      for (char character : text.text) {
        auto vertices = font_atlases_[0].GetCharacterVertices(character, &position, text.color);
        if (shape_vertex_count_ + vertices.size() > VERTEX_BUFFER_ELEMENT_COUNT) {
          DrawShapeVertices();
        }
        for (auto& vertex : vertices) {
          const Vector2 transformed_position = TransformPosition(world_to_clip_space, Vector3(vertex.x, vertex.y, 0.0f));
          vertex.x = transformed_position.x;
          vertex.y = transformed_position.y;
        }

        std::memcpy(&shape_vertices_[shape_vertex_count_], vertices.data(), sizeof(Shape2D::Vertex) * vertices.size());
        shape_vertex_count_ += vertices.size();
      }
    }
  }

  DrawShapeVertices();
}

void Renderer2D::DrawShapeVertices() {
  if (shape_vertex_count_ == 0) {
    return;
  }
  assert(shape_vertex_count_ % 3 == 0);

  vertex_buffer_->Write(0, shape_vertex_count_ * sizeof(Shape2D::Vertex), shape_vertices_.data());

  DrawItem draw_item;
  draw_item.vertex_input = vertex_input_.get();
  draw_item.shader_program = shape_shader_.get();
  draw_item.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
  // draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  draw_item.count = shape_vertex_count_;
  draw_item.blend_state.enabled = true;
  draw_item.blend_state.source_color_factor = SourceBlendFactor::SOURCE_ALPHA;
  draw_item.blend_state.destination_color_factor = DestinationBlendFactor::ONE_MINUS_SOURCE_ALPHA;
  context()->Draw(draw_item);

  shape_vertex_count_ = 0;
}

}  // namespace ovis
