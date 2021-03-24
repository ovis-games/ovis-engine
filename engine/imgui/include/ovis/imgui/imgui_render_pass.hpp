#include <memory>

#include <imgui.h>

#include <ovis/core/vector.hpp>
#include <ovis/graphics/index_buffer.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/vertex_input.hpp>
#include <ovis/rendering/render_pass.hpp>

namespace ovis {

class ImGuiRenderPass : public RenderPass {
 public:
  static inline constexpr std::string_view Name() { return "ImGui"; }

  ImGuiRenderPass();
  virtual ~ImGuiRenderPass();

  void CreateResources() override;

  void Render(const RenderContext& render_context) override;

 private:
  std::unique_ptr<Texture2D> font_texture_;

  std::unique_ptr<ShaderProgram> shader_program_;
  std::unique_ptr<VertexBuffer> vertex_buffer_;
  std::unique_ptr<VertexInput> vertex_input_;
  std::unique_ptr<IndexBuffer> index_buffer_;
};

}  // namespace ovis
