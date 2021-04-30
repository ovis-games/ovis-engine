#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <span>

#include <ovis/core/color.hpp>
#include <ovis/core/matrix.hpp>
#include <ovis/core/vector.hpp>
#include <ovis/graphics/graphics_context.hpp>
#include <ovis/graphics/shader_program.hpp>
#include <ovis/graphics/vertex_buffer.hpp>
#include <ovis/graphics/vertex_input.hpp>
#include <ovis/rendering/render_pass.hpp>

namespace ovis {

class PrimitiveRenderer : public RenderPass {
 public:
  enum class DrawSpace { WORLD, SCREEN };

  PrimitiveRenderer(std::string_view name);

  void CreateResources() override;
  void ReleaseResources() override;

  void SetDrawSpace(DrawSpace space);
  inline DrawSpace draw_space() const { return draw_space_; }

 protected:
  void BeginDraw(const RenderContext& render_context);
  void EndDraw();

  struct Vertex {
    float position[3];
    uint32_t color;
  };
  static_assert(sizeof(Vertex) == 16);

  // Point Drawing
  void DrawPoint(const Vector3& position, float size, const Color& color);

  // Line Drawing
  void DrawLine(const Vector3& start, const Vector3& end, const Color& color, float thickness = 3.0f);
  void DrawLineStip(std::span<const Vector3> positions, const Color& color, float thickness = 3.0f);
  void DrawLoop(std::span<const Vector3> positions, const Color& color, float thickness = 3.0f);
  void DrawCircle(const Vector3& center, float radius, const Color& color, float thickness = 3.0f,
                  size_t num_segments = 0, const Vector3& support_vector0 = Vector3::PositiveX(),
                  const Vector3& support_vector1 = Vector3::PositiveY());

  // 2D Arrow
  void DrawArrow(const Vector3& start, const Vector3& end, const Color& color, float thickness = 3.0f,
                 float arrow_width = 2.0f, float arrow_length = 2.0f);

  // Solid Drawing
  void DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color);
  void DrawDisc(const Vector3& center, float radius, const Color& color, size_t num_segments = 0,
                const Vector3& support_vector0 = Vector3::PositiveX(),
                const Vector3& support_vector1 = Vector3::PositiveY());
  void DrawConvexPolygon(const Vector3* positions, size_t num_positions, const Color& color);

  //
  void AddVertices(std::span<const Vertex> vertices);
  void AddLineVerticesWithControlPoints(const Vector3& previous, const Vector3& start, const Vector3& end,
                                        const Vector3& next, uint32_t color, float half_thickness);

  bool enable_alpha_blending_ = false;

 private:
  DrawSpace draw_space_ = DrawSpace::WORLD;
  Matrix4 to_screen_space_;
  bool is_drawing_ = false;

  static constexpr size_t VERTEX_BUFFER_ELEMENT_COUNT = 1024 * 1024 / sizeof(Vertex);
  struct Resources {
    std::vector<Vertex> vertices;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<VertexInput> vertex_input;
    std::unique_ptr<ShaderProgram> shader;
  };
  static std::map<GraphicsContext*, std::weak_ptr<Resources>> resources;
  std::shared_ptr<Resources> resources_;

  void Flush();
  size_t CalculateSmoothCircleSegmentCount(const Vector3& center, float radius, const Vector3& support_vector0,
                                           const Vector3& support_vector1);
};

}  // namespace ovis
