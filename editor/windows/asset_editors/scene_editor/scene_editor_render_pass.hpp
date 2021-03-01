#include <cstdint>
#include <memory>
#include <vector>

#include <box2d/b2_draw.h>

#include <ovis/math/basic_types.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/vertex_input.hpp>
#include <ovis/engine/render_pass.hpp>

namespace ovis {
namespace editor {

class SceneEditor;

class SceneEditorRenderPass : public RenderPass, public b2Draw {
 public:
  SceneEditorRenderPass(SceneEditor* scene_editor);

  void CreateResources() override;
  void ReleaseResources() override;

  void Render(const RenderContext& render_context) override;

  void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
  void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
  void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
  void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
  void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
  void DrawTransform(const b2Transform& xf) override;
  void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;

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

  bool is_rendering_ = false;

  const size_t vertex_buffer_size = 100;

  void RenderLine(vector3 start, vector3 end, vector4 color = vector4(1.0f));
  void RenderLine(vector3 start, vector4 start_color, vector3 end, vector4 end_color);
  void RenderLineLoop(std::vector<vector3> points, vector4 color = vector4(1.0f));
};

}  // namespace editor

}  // namespace ovis
