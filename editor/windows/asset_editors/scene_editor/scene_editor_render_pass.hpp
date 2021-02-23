#include <memory>
#include <cstdint>
#include <vector>

#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/engine/render_pass.hpp>
#include <ovis/math/basic_types.hpp>
#include <ovis/graphics/vertex_input.hpp>

namespace ovis {
namespace editor {

class SceneEditor;

class SceneEditorRenderPass : public RenderPass {
 public:
  SceneEditorRenderPass(SceneEditor* scene_editor);

  void CreateResources() override;
  void ReleaseResources() override;

  void Render(const RenderContext& render_context) override;

 private:
  struct LineVertex {
    vector3 position;
    glm::tvec4<std::uint8_t> color;
  };
  static_assert(sizeof(LineVertex) == 16);

  std::vector<LineVertex> line_vertices_;
  std::unique_ptr<VertexBuffer> line_vertex_buffer_;
  std::unique_ptr<ShaderProgram> line_shader_;
  std::unique_ptr<VertexInput> line_vertex_input;

  const size_t vertex_buffer_size = 100;

  void RenderLine(vector3 start, vector3 end, vector4 color = vector4(1.0f));
  void RenderLine(vector3 start, vector4 start_color, vector3 end, vector4 end_color);
  void RenderLineLoop(std::vector<vector3> points, vector4 color = vector4(1.0f));
};

}  // namespace editor

}  // namespace ovis
