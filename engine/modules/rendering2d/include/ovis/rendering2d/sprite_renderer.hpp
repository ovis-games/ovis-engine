#pragma once

#include <ovis/core/resource.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/static_mesh.hpp>
#include <ovis/engine/render_pass.hpp>

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
  std::unordered_map<std::string, std::unique_ptr<Texture2D>> textures_;
};

}  // namespace ovis