#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>

#include <ovis/math/color.hpp>
#include <ovis/math/vector.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/vertex_input.hpp>
#include <ovis/rendering/render_pass.hpp>

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
    float position[3];
    float size;
    uint32_t color;
  };
  static_assert(sizeof(PointVertex) == 20);

  struct Vertex {
    float position[3];
    uint32_t color;
  };
  static_assert(sizeof(Vertex) == 16);

  // Point Drawing
  void DrawPoint(const Vector3& v0, float size, const Color& color);
  void DrawPoints(const PointVertex* vertices, size_t num_vertices);

  // Line Drawing
  void DrawLine(const Vector3& v0, const Vector3& v1, const Color& color);
  void DrawLoop(const Vector3* positions, size_t num_positions, const Color& color);
  void DrawCircle(const Vector3& center, float radius, const Color& color, size_t num_segments = 20,
                  const Vector3& support_vector0 = Vector3::PositiveX(),
                  const Vector3& support_vector1 = Vector3::PositiveY());
  void DrawLines(const Vertex* vertices, size_t num_vertices);

  // Solid Drawing
  void DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color);
  void DrawDisc(const Vector3& center, float radius, const Color& color, size_t num_segments = 20,
                const Vector3& support_vector0 = Vector3::PositiveX(),
                const Vector3& support_vector1 = Vector3::PositiveY());
  void DrawConvexPolygon(const Vector3* positions, size_t num_positions, const Color& color);
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
