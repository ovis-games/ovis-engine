#pragma once

#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/vertex_input.hpp>
#include <ovis/rendering/render_pass.hpp>
#include <ovis/rendering2d/shape2d.hpp>

namespace ovis {

class Renderer2D : public RenderPass {
 public:
  Renderer2D();

  void CreateResources() override;
  void ReleaseResources() override;

  void Render(const RenderContext& render_context) override;

 private:
  static constexpr size_t VERTEX_BUFFER_ELEMENT_COUNT = 64 * 1024;
  std::array<Shape2D::Vertex, VERTEX_BUFFER_ELEMENT_COUNT> shape_vertices_;
  size_t shape_vertex_count_ = 0;
  std::unique_ptr<VertexBuffer> vertex_buffer_;
  std::unique_ptr<VertexInput> vertex_input_;
  std::unique_ptr<ShaderProgram> shape_shader_;
  std::unique_ptr<Texture2D> empty_texture_;
  std::unordered_map<std::string, std::unique_ptr<Texture2D>> textures_;

  std::vector<SceneObject*> object_cache_;

  void DrawShapeVertices();
};

}  // namespace ovis
