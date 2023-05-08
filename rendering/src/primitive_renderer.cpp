#include "ovis/core/math_constants.hpp"
#include "ovis/rendering/primitive_renderer.hpp"

namespace ovis {
std::map<GraphicsContext*, std::weak_ptr<PrimitiveRenderer::Resources>> PrimitiveRenderer::resources;

PrimitiveRenderer::PrimitiveRenderer(std::string_view job_id, GraphicsContext* graphics_context)
    : RenderPass(job_id, graphics_context) {}

void PrimitiveRenderer::CreateResources() {
  assert(!is_drawing_);
  assert(!resources_);

  // REMARK: This is currently not thread safe, but it currently does not have to be and most likely never will be.
  auto resources_iterator = resources.find(context());
  if (resources_iterator != resources.end()) {
    resources_ = resources_iterator->second.lock();
    if (resources_) {
      return;
    }
  }

  LogV("Creating PrimitiveRenderer resources for context {}", static_cast<void*>(context()));
  resources_ = std::make_shared<Resources>();
  resources_->shader = LoadShaderProgram("primitive_rendering", context());

  VertexBufferDescription buffer_desc;
  buffer_desc.vertex_size_in_bytes = sizeof(Vertex);
  buffer_desc.size_in_bytes = sizeof(Vertex) * VERTEX_BUFFER_ELEMENT_COUNT;
  resources_->vertex_buffer = std::make_unique<ovis::VertexBuffer>(context(), buffer_desc);

  VertexInputDescription vertex_input_desc;
  vertex_input_desc.vertex_buffers = {resources_->vertex_buffer.get()};
  vertex_input_desc.vertex_attributes = {
      {*resources_->shader->GetAttributeLocation("Position"), 0, 0, VertexAttributeType::FLOAT32_VECTOR3},
      {*resources_->shader->GetAttributeLocation("Color"), 12, 0, VertexAttributeType::UINT8_NORM_VECTOR4}};
  resources_->vertex_input = std::make_unique<VertexInput>(context(), vertex_input_desc);

  resources_->vertices.reserve(VERTEX_BUFFER_ELEMENT_COUNT);

  // resources.insert(std::make_pair(context(), resources_));
}

void PrimitiveRenderer::ReleaseResources() {
  resources_.reset();
}

void PrimitiveRenderer::SetDrawSpace(DrawSpace space) {
  assert(!is_drawing_);
  draw_space_ = space;
}

void PrimitiveRenderer::BeginDraw(const SceneViewport& viewport) {
  assert(!is_drawing_);
  assert(resources_);

  is_drawing_ = true;
  resources_->vertices.clear();

  const Vector2 viewport_size = viewport.dimensions;
  screen_aabb_ = AxisAlignedBoundingBox2D::FromMinMax(Vector2::Zero(), viewport_size);

  const Matrix4 screen_to_clip_space =
      Matrix4::FromOrthographicProjection(0.0f, viewport_size.x, viewport_size.y, 0.0f, -1.0f, 1.0f);

  if (draw_space_ == DrawSpace::WORLD) {
    to_screen_space_ = Invert(screen_to_clip_space) * viewport.world_to_clip;
  } else {
    to_screen_space_ = Matrix4::Identity();
  }

  resources_->shader->SetUniform("ScreenToClipSpace", screen_to_clip_space);
}

void PrimitiveRenderer::EndDraw() {
  assert(is_drawing_);
  assert(resources_);

  Flush();
  is_drawing_ = false;
}

void PrimitiveRenderer::DrawPoint(const Vector3& position, float size, const Color& color) {
  std::array<Vertex, 6> vertices;
  const Vector2 extend = {size, size};

  const Vector3 p = TransformPosition(to_screen_space_, position);
  const Vector3 p00 = p + Vector3::FromVector2(extend * Vector2::UnitSquare()[0]);
  const Vector3 p01 = p + Vector3::FromVector2(extend * Vector2::UnitSquare()[1]);
  const Vector3 p10 = p + Vector3::FromVector2(extend * Vector2::UnitSquare()[2]);
  const Vector3 p11 = p + Vector3::FromVector2(extend * Vector2::UnitSquare()[3]);
  const uint32_t converted_color = ConvertToRGBA8(color);

  memcpy(vertices[0].position, p00.data, sizeof(float) * 3);
  vertices[0].color = converted_color;

  memcpy(vertices[1].position, p11.data, sizeof(float) * 3);
  vertices[1].color = converted_color;

  memcpy(vertices[2].position, p10.data, sizeof(float) * 3);
  vertices[2].color = converted_color;

  memcpy(vertices[3].position, p00.data, sizeof(float) * 3);
  vertices[3].color = converted_color;

  memcpy(vertices[4].position, p01.data, sizeof(float) * 3);
  vertices[4].color = converted_color;

  memcpy(vertices[5].position, p11.data, sizeof(float) * 3);
  vertices[5].color = converted_color;

  AddVertices(vertices);
}

void PrimitiveRenderer::DrawLine(const Vector3& start, const Vector3& end, const Color& color, float thickness) {
  const uint32_t converted_color = ConvertToRGBA8(color);
  const Vector3 p0 = TransformPosition(to_screen_space_, start);
  const Vector3 p1 = TransformPosition(to_screen_space_, end);
  const Vector3 orthogonal = Vector3::FromVector2(ConstructOrthogonalVectorCCW(Normalize<Vector2>(p1 - p0)));
  const float half_thickness = thickness * 0.5f;

  DrawLineInternal(p0, p1, orthogonal, converted_color, half_thickness);
}

void PrimitiveRenderer::DrawDashedLine(const Vector3& start, const Vector3& end, const Color& color, float thickness,
                                       float dash_length) {
  const uint32_t converted_color = ConvertToRGBA8(color);
  const float half_thickness = thickness * 0.5f;

  const LineSegment2D line_segment = {TransformPosition(to_screen_space_, start),
                                      TransformPosition(to_screen_space_, end)};

  const std::optional<LineSegment2D> clipped_line_segment = ClipLineSegment(screen_aabb_, line_segment);
  if (!clipped_line_segment.has_value()) {
    return;
  }

  const Vector3 p0 = Vector3::FromVector2(clipped_line_segment->start);
  const Vector3 p1 = Vector3::FromVector2(clipped_line_segment->end);
  const Vector3 orthogonal = Vector3::FromVector2(ConstructOrthogonalVectorCCW(Normalize<Vector2>(p1 - p0)));

  const float line_length = Distance(p0, p1);
  const Vector3 line_direction = (p1 - p0) / line_length;

  // Length of a dash plus the length of the space afterwards
  const float actual_dash_length = dash_length <= 0.0f ? 4 * thickness : dash_length;
  const float dash_interval = 2 * actual_dash_length;
  const float dash_count = line_length / dash_interval;
  const int complete_dashes = static_cast<int>(dash_count);

  for (int i = 0; i < complete_dashes; ++i) {
    const Vector3 dash_start = p0 + line_direction * dash_interval * i;
    const Vector3 dash_end = dash_start + line_direction * actual_dash_length;
    DrawLineInternal(dash_start, dash_end, orthogonal, converted_color, half_thickness);
  }

  {
    const float remaining_dash_fraction = std::min(1.0f, 2.0f * (dash_count - complete_dashes));
    const Vector3 final_dash_start = p0 + line_direction * dash_interval * complete_dashes;
    const Vector3 final_dash_end = final_dash_start + line_direction * actual_dash_length * remaining_dash_fraction;
    DrawLineInternal(final_dash_start, final_dash_end, orthogonal, converted_color, half_thickness);
  }
}

void PrimitiveRenderer::DrawLineStip(std::span<const Vector3> positions, const Color& color, float thickness) {
  assert(positions.size() >= 2);

  for (size_t i = 0; i < positions.size() - 1; ++i) {
    DrawLine(positions[i], positions[i + 1], color, thickness);
  }
}

void PrimitiveRenderer::DrawLoop(std::span<const Vector3> positions, const Color& color, float thickness) {
  assert(positions.size() > 2);
  DrawLineStip(positions, color, thickness);
  DrawLine(positions.back(), positions.front(), color, thickness);
}

void PrimitiveRenderer::DrawCircle(const Vector3& center, float radius, const Color& color, float thickness,
                                   size_t num_segments, const Vector3& support_vector0,
                                   const Vector3& support_vector1) {
  const size_t final_segment_count =
      num_segments < 2 ? CalculateSmoothCircleSegmentCount(center, radius, support_vector0, support_vector1)
                       : num_segments;

  const Vector3 base_position = center + radius * support_vector1;
  Vector3 previous_position = base_position;
  for (int i = 1; i < final_segment_count; ++i) {
    const float angle = i * 2.0f * Pi<float>() / final_segment_count;

    const Vector3 new_position =
        center + radius * (std::sin(angle) * support_vector0 + std::cos(angle) * support_vector1);
    DrawLine(previous_position, new_position, color, thickness);
    previous_position = new_position;
  }
  DrawLine(previous_position, base_position, color, thickness);
}

void PrimitiveRenderer::DrawArrow(const Vector3& start, const Vector3& end, const Color& color, float thickness,
                                  float arrow_width, float arrow_length) {
  const Vector3 start_to_end = end - start;
  const float start_to_end_length = Length(start_to_end);
  const Vector3 line_end =
      start + start_to_end * (start_to_end_length - thickness * arrow_length) / start_to_end_length;

  DrawLine(start, line_end, color, thickness);

  const Vector3 orthogonal = Vector3::FromVector2(Normalize(ConstructOrthogonalVectorCCW(start_to_end)));
  const float offset = 0.5f * thickness * arrow_width;

  // clang-format off
  DrawTriangle(
    line_end + orthogonal * offset,
    line_end - orthogonal * offset,
    end,
    color
  );
  // clang-format on
}

void PrimitiveRenderer::DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color) {
  uint32_t converted_color = ConvertToRGBA8(color);
  const Vector3 screen_space_v0 = TransformPosition(to_screen_space_, v0);
  const Vector3 screen_space_v1 = TransformPosition(to_screen_space_, v1);
  const Vector3 screen_space_v2 = TransformPosition(to_screen_space_, v2);
  Vertex vertices[3] = {{{screen_space_v0.x, screen_space_v0.y, screen_space_v0.z}, converted_color},
                        {{screen_space_v1.x, screen_space_v1.y, screen_space_v1.z}, converted_color},
                        {{screen_space_v2.x, screen_space_v2.y, screen_space_v2.z}, converted_color}};
  AddVertices(vertices);
}

void PrimitiveRenderer::DrawDisc(const Vector3& center, float radius, const Color& color, size_t num_segments,
                                 const Vector3& support_vector0, const Vector3& support_vector1) {
  const size_t final_segment_count =
      num_segments < 2 ? CalculateSmoothCircleSegmentCount(center, radius, support_vector0, support_vector1)
                       : num_segments;

  const Vector3 base_position = center + radius * support_vector1;
  Vector3 previous_position = base_position;
  for (int i = 1; i < final_segment_count; ++i) {
    const float angle = i * 2.0f * Pi<float>() / final_segment_count;

    const Vector3 new_position =
        center + radius * (std::sin(angle) * support_vector0 + std::cos(angle) * support_vector1);
    DrawTriangle(previous_position, new_position, center, color);
    previous_position = new_position;
  }
  DrawTriangle(previous_position, base_position, center, color);
}

void PrimitiveRenderer::DrawConvexPolygon(const Vector3* positions, size_t num_positions, const Color& color) {
  assert(num_positions > 2);

  for (size_t i = 0; i < num_positions - 1; ++i) {
    DrawTriangle(positions[0], positions[i], positions[i + 1], color);
  }
}

void PrimitiveRenderer::AddVertices(std::span<const Vertex> vertices) {
  assert(is_drawing_);
  assert(resources_);
  assert(vertices.size() % 3 == 0);

  const size_t remaining_space = VERTEX_BUFFER_ELEMENT_COUNT - resources_->vertices.size();
  const size_t vertices_to_copy = (std::min(vertices.size(), remaining_space) / 3) * 3;
  resources_->vertices.insert(resources_->vertices.end(), vertices.begin(), vertices.begin() + vertices_to_copy);

  if (vertices_to_copy < vertices.size()) {
    // We could fit all vertices, flush buffer and try again
    Flush();
    AddVertices(vertices.subspan(vertices_to_copy));
  }
}

void PrimitiveRenderer::Flush() {
  assert(is_drawing_);
  assert(resources_);

  const size_t vertex_count = resources_->vertices.size();
  assert(vertex_count < VERTEX_BUFFER_ELEMENT_COUNT);
  assert(vertex_count % 3 == 0);

  if (resources_->vertices.size() == 0) {
    return;
  }

  resources_->vertex_buffer->Write(0, vertex_count * sizeof(Vertex), resources_->vertices.data());

  DrawItem draw_item;
  draw_item.vertex_input = resources_->vertex_input.get();
  draw_item.shader_program = resources_->shader.get();
  draw_item.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
  // draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  draw_item.count = vertex_count;
  if (enable_alpha_blending_) {
    draw_item.blend_state.enabled = true;
    draw_item.blend_state.source_color_factor = SourceBlendFactor::SOURCE_ALPHA;
    draw_item.blend_state.destination_color_factor = DestinationBlendFactor::ONE_MINUS_SOURCE_ALPHA;
  }
  context()->Draw(draw_item);

  resources_->vertices.clear();
}

void PrimitiveRenderer::DrawLineInternal(const Vector3& p0, const Vector3& p1, const Vector3& orthogonal,
                                         uint32_t converted_color, float half_thickness) {
  const Vector3 p00 = p0 + half_thickness * orthogonal;
  const Vector3 p01 = p0 - half_thickness * orthogonal;
  const Vector3 p10 = p1 + half_thickness * orthogonal;
  const Vector3 p11 = p1 - half_thickness * orthogonal;

  std::array<Vertex, 6> vertices;

  memcpy(vertices[0].position, p00.data, sizeof(float) * 3);
  vertices[0].color = converted_color;

  memcpy(vertices[1].position, p11.data, sizeof(float) * 3);
  vertices[1].color = converted_color;

  memcpy(vertices[2].position, p10.data, sizeof(float) * 3);
  vertices[2].color = converted_color;

  memcpy(vertices[3].position, p00.data, sizeof(float) * 3);
  vertices[3].color = converted_color;

  memcpy(vertices[4].position, p01.data, sizeof(float) * 3);
  vertices[4].color = converted_color;

  memcpy(vertices[5].position, p11.data, sizeof(float) * 3);
  vertices[5].color = converted_color;

  AddVertices(vertices);
}

size_t PrimitiveRenderer::CalculateSmoothCircleSegmentCount(const Vector3& center, float radius,
                                                            const Vector3& support_vector0,
                                                            const Vector3& support_vector1) {
  // To create a smooth circle we need enough segments. The number of segments depends on the pixels the circle covers.
  // We approximately want one segment per pixel which can be approximated by the radius of the circle in screen space.
  // However, the radius definition given here can be in an arbitrary space and cannot be converted to screen space.
  // Thus, I approximate it by calculating the screen space distance along the two support vectors, which give me two
  // pontenial radii and I will take the maximum of those two as the "screen space radius".
  const Vector3& center_screen_space = TransformPosition(to_screen_space_, center);
  const Vector3& p0_screen_space = TransformPosition(to_screen_space_, center + support_vector0 * radius);
  const Vector3& p1_screen_space = TransformPosition(to_screen_space_, center + support_vector1 * radius);
  const float squared_screen_space_radius0 = SquaredDistance(center_screen_space, p0_screen_space);
  const float squared_screen_space_radius1 = SquaredDistance(center_screen_space, p1_screen_space);
  const float approximate_screen_space_radius =
      std::sqrt(std::max(squared_screen_space_radius0, squared_screen_space_radius1));

  // Minimum and maximum is a little arbitrary but why shouldn't that be?
  return std::clamp<size_t>(8, 512, std::ceil(TwoPi<float>() * approximate_screen_space_radius));
}

}  // namespace ovis
