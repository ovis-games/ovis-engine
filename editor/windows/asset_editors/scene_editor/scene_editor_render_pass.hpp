#include <memory>
#include <cstdint>
#include <vector>

#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/engine/render_pass.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <ovis/graphics/vertex_input.hpp>

namespace ovis {
namespace editor {

class SceneEditor;

class SceneEditorRenderPass : public RenderPass {
 public:
  SceneEditorRenderPass(SceneEditor* scene_editor);

  void CreateResources() override;
  void ReleaseResources() override;

  void Render(Scene* scene) override;

 private:
  struct LineVertex {
    glm::vec3 position;
    glm::tvec4<std::uint8_t> color;
  };
  static_assert(sizeof(LineVertex) == 16);

  std::vector<LineVertex> line_vertices_;
  std::unique_ptr<VertexBuffer> line_vertex_buffer_;
  std::unique_ptr<ShaderProgram> line_shader_;
  std::unique_ptr<VertexInput> line_vertex_input;
};

}  // namespace editor

}  // namespace ovis
