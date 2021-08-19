#pragma once

#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/vertex_input.hpp>
#include <ovis/rendering/render_pass.hpp>

namespace ovis {

class SpriteRenderer : public RenderPass {
 public:
  SpriteRenderer();

  void CreateResources() override;

  void Render(const RenderContext& render_context) override;

 private:
  std::unique_ptr<ShaderProgram> shader_program_;
  std::unique_ptr<VertexBuffer> vertex_buffer_;
  std::unique_ptr<VertexInput> vertex_input_;
  std::unique_ptr<Texture2D> empty_texture_;
  std::unordered_map<std::string, std::unique_ptr<Texture2D>> textures_;
};

}  // namespace ovis
