#include <ovis/core/asset_library.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/render_target_configuration.hpp>
#include <ovis/rendering/graphics_loader.hpp>
#include <ovis/rendering/rendering_viewport.hpp>
#include <ovis/rendering2d/sprite.hpp>
#include <ovis/rendering2d/sprite_renderer.hpp>

namespace ovis {

SpriteRenderer::SpriteRenderer() : RenderPass("SpriteRenderer") {}

void SpriteRenderer::CreateResources() {
  shader_program_ = LoadShaderProgram(GetEngineAssetLibrary(), "sprite", context());

  struct Vertex {
    Vector2 position;
    Vector2 texture_coordinates;
  };

  Vertex vertices[] = {{{-0.5f, 0.5f}, {0.0f, 0.0f}},
                       {{-0.5f, -0.5f}, {0.0f, 1.0f}},
                       {{0.5f, 0.5f}, {1.0f, 0.0f}},
                       {{0.5f, -0.5f}, {1.0f, 1.0f}}};

  VertexBufferDescription vb_desc;
  vb_desc.vertex_size_in_bytes = 4 * sizeof(float);
  vb_desc.size_in_bytes = 4 * vb_desc.vertex_size_in_bytes;
  vertex_buffer_ = std::make_unique<VertexBuffer>(context(), vb_desc, vertices);

  VertexInputDescription vi_desc;
  vi_desc.vertex_buffers = {vertex_buffer_.get()};
  vi_desc.vertex_attributes = {
      {*shader_program_->GetAttributeLocation("Position"), 0, 0, VertexAttributeType::FLOAT32_VECTOR2},
      {*shader_program_->GetAttributeLocation("TextureCoordinates"), 8, 0, VertexAttributeType::FLOAT32_VECTOR2}};
  vertex_input_ = std::make_unique<VertexInput>(context(), vi_desc);
}

void SpriteRenderer::Render(const RenderContext& render_context) {
  DrawItem draw_item;
  draw_item.shader_program = shader_program_.get();
  draw_item.vertex_input = vertex_input_.get();
  draw_item.primitive_topology = PrimitiveTopology::TRIANGLE_STRIP;
  draw_item.count = 4;
  draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  draw_item.blend_state.enabled = true;
  draw_item.blend_state.source_color_factor = SourceBlendFactor::SOURCE_ALPHA;
  draw_item.blend_state.destination_color_factor = DestinationBlendFactor::ONE_MINUS_SOURCE_ALPHA;

  // TODO: "cache" vector (to avoid reallocation)
  auto objects_with_sprites = viewport()->scene()->GetSceneObjectsWithComponent("Sprite");

  std::sort(objects_with_sprites.begin(), objects_with_sprites.end(), [](SceneObject* lhs, SceneObject* rhs) {
    Vector3 lhs_position;
    Transform* lhs_transform = lhs->GetComponent<Transform>("Transform");
    if (lhs_transform != nullptr) {
      lhs_position = lhs_transform->position();
    }

    Vector3 rhs_position;
    Transform* rhs_transform = rhs->GetComponent<Transform>("Transform");
    if (rhs_transform != nullptr) {
      rhs_position = rhs_transform->position();
    }

    // TODO: project into camera view axis instead of using the z coordinates
    return lhs_position.z > rhs_position.z;
  });

  for (SceneObject* object : objects_with_sprites) {
    Sprite* sprite = object->GetComponent<Sprite>("Sprite");
    const Matrix3x4 size_matrix = Matrix3x4::FromScaling(Vector3::FromVector2(sprite->size(), 1.0f));
    const Color color = sprite->color();
    const std::string texture_asset = sprite->texture_asset();

    auto texture_iterator = textures_.find(texture_asset);
    if (texture_iterator == textures_.end()) {
      texture_iterator = textures_.insert(std::make_pair(texture_asset, LoadTexture2D(texture_asset, context()))).first;
    }
    const std::unique_ptr<Texture2D>& texture = texture_iterator->second;

    Transform* transform = object->GetComponent<Transform>("Transform");
    const Matrix4 world_view_projection =
        transform ? AffineCombine(render_context.world_to_clip_space,
                                  AffineCombine(transform->local_to_world_matrix(), size_matrix))
                  : AffineCombine(render_context.world_to_clip_space, size_matrix);

    Mat4x4 local_to_clip_space_matrix;
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        local_to_clip_space_matrix[i][j] = world_view_projection[i][j];
      }
    }
    Vec4 color2 = {color[0], color[1], color[2], color[3]};
    shader_program_->SetUniform("WorldViewProjection", local_to_clip_space_matrix);
    shader_program_->SetUniform("Color", color2);
    shader_program_->SetTexture("Texture", texture.get());
    context()->Draw(draw_item);
  }
}

}  // namespace ovis
