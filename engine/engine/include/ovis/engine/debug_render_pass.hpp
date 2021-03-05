#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>

#include <ovis/math/basic_types.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/vertex_input.hpp>
#include <ovis/engine/render_pass.hpp>

namespace ovis {

class DebugRenderPass : public RenderPass {
 public:
  enum class DrawSpace { WORLD, VIEWPORT };

  DebugRenderPass(const std::string& name);

  void CreateResources() override;
  void ReleaseResources() override;

  void SetDrawSpace(DrawSpace space);
  inline DrawSpace draw_space() const { return draw_space_; }

 protected:
  void BeginDraw(const RenderContext& render_context);
  void EndDraw();

  struct PointVertex {
    vector3 position;
    float size;
    glm::tvec4<std::uint8_t> color;
  };
  static_assert(sizeof(PointVertex) == 20);

  struct Vertex {
    vector3 position;
    glm::tvec4<std::uint8_t> color;
  };
  static_assert(sizeof(Vertex) == 16);

  // Point Drawing
  void DrawPoint(const vector3& v0, float size, const color& color);
  void DrawPoints(const PointVertex* vertices, size_t num_vertices);

  // Line Drawing
  void DrawLine(const vector3& v0, const vector3& v1, const color& color);
  void DrawLoop(const vector3* positions, size_t num_positions, const color& color);
  void DrawCircle(const vector3& center, float radius, const color& color, size_t num_segments = 20,
                  const vector3& support_vector0 = vector3(1.0f, 0.0f, 0.0f),
                  const vector3& support_vector1 = vector3(0.0f, 1.0f, 0.0f));
  void DrawLines(const Vertex* vertices, size_t num_vertices);

  // Solid Drawing
  void DrawTriangle(const vector3& v0, const vector3& v1, const vector3& v2, const color& color);
  void DrawDisc(const vector3& center, float radius, const color& color, size_t num_segments = 20,
                const vector3& support_vector0 = vector3(1.0f, 0.0f, 0.0f),
                const vector3& support_vector1 = vector3(0.0f, 1.0f, 0.0f));
  void DrawConvexPolygon(const vector3* positions, size_t num_positions, const color& color);
  void DrawTriangles(const Vertex* vertices, size_t num_vertices);

  bool enable_alpha_blending_ = false;

 private:
  DrawSpace draw_space_ = DrawSpace::WORLD;
  bool is_drawing_ = false;

  static constexpr size_t POINT_VERTEX_BUFFER_ELEMENT_COUNT = 1024 / sizeof(PointVertex);
  static constexpr size_t VERTEX_BUFFER_ELEMENT_COUNT = 1024 * 1024 / sizeof(Vertex);
  struct Resources {
    std::vector<PointVertex> point_vertices;
    std::vector<Vertex> line_vertices;
    std::vector<Vertex> triangle_vertices;
    std::unique_ptr<VertexBuffer> point_vertex_buffer;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<VertexInput> point_vertex_input;
    std::unique_ptr<VertexInput> vertex_input;
    std::unique_ptr<ShaderProgram> point_shader;
    std::unique_ptr<ShaderProgram> shader;
  };
  static std::map<GraphicsContext*, std::weak_ptr<Resources>> resources;
  std::shared_ptr<Resources> resources_;

  void FlushPoints();
  void FlushLines();
  void FlushTriangles();
  inline void Flush() {
    FlushPoints();
    FlushLines();
    FlushTriangles();
  }
};

}  // namespace ovis
