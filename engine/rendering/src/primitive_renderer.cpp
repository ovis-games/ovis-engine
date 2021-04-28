#include <ovis/core/math_constants.hpp>
#include <ovis/rendering/primitive_renderer.hpp>
#include <ovis/rendering/rendering_viewport.hpp>

namespace ovis {

PrimitiveRenderer::PrimitiveRenderer(std::string_view name) : RenderPass(name) {}

void PrimitiveRenderer::CreateResources() {
  SDL_assert(!is_drawing_);
  SDL_assert(!resources_);

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
  SDL_assert(!is_drawing_);
  draw_space_ = space;
}

void PrimitiveRenderer::BeginDraw(const RenderContext& render_context) {
  SDL_assert(!is_drawing_);
  SDL_assert(resources_);

  is_drawing_ = true;
  resources_->vertices.clear();

  const Vector2 viewport_size = viewport()->GetDimensions();
  const Matrix4 screen_to_clip_space =
      Matrix4::FromOrthographicProjection(0.0f, viewport_size.x, viewport_size.y, 0.0f, -1.0f, 1.0f);

  if (draw_space_ == DrawSpace::WORLD) {
    to_screen_space_ = Invert(screen_to_clip_space) * render_context.world_to_clip_space;
  } else {
    to_screen_space_ = Matrix4::Identity();
  }

  resources_->shader->SetUniform("ScreenToClipSpace", screen_to_clip_space);
}

void PrimitiveRenderer::EndDraw() {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);

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
  std::array<Vertex, 6> vertices;

  const uint32_t converted_color = ConvertToRGBA8(color);
  const Vector3 p0 = TransformPosition(to_screen_space_, start);
  const Vector3 p1 = TransformPosition(to_screen_space_, end);
  const Vector3 orthogonal = Vector3::FromVector2(ConstructOrthogonalVectorCCW(Normalize<Vector2>(p1 - p0)));
  const float half_thickness = thickness * 0.5f;

  const Vector3 p00 = p0 + half_thickness * orthogonal;
  const Vector3 p01 = p0 - half_thickness * orthogonal;
  const Vector3 p10 = p1 + half_thickness * orthogonal;
  const Vector3 p11 = p1 - half_thickness * orthogonal;

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

void PrimitiveRenderer::DrawLineStip(std::span<const Vector3> positions, const Color& color, float thickness) {
  SDL_assert(positions.size() >= 2);

  for (size_t i = 0; i < positions.size() - 1; ++i) {
    DrawLine(positions[i], positions[i + 1], color, thickness);
  }
}

void PrimitiveRenderer::DrawLoop(std::span<const Vector3> positions, const Color& color, float thickness) {
  SDL_assert(positions.size() > 2);
  DrawLineStip(positions, color, thickness);
  DrawLine(positions.back(), positions.front(), color, thickness);
}

void PrimitiveRenderer::DrawCircle(const Vector3& center, float radius, const Color& color, float thickness,
                                   size_t num_segments, const Vector3& support_vector0,
                                   const Vector3& support_vector1) {
  SDL_assert(num_segments > 2);

  const Vector3 base_position = center + radius * support_vector1;
  Vector3 previous_position = base_position;
  for (int i = 1; i < num_segments; ++i) {
    const float angle = i * 2.0f * Pi<float>() / num_segments;

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
  SDL_assert(num_segments > 2);

  const Vector3 base_position = center + radius * support_vector1;
  Vector3 previous_position = base_position;
  for (int i = 1; i < num_segments; ++i) {
    const float angle = i * 2.0f * Pi<float>() / num_segments;

    const Vector3 new_position =
        center + radius * (std::sin(angle) * support_vector0 + std::cos(angle) * support_vector1);
    DrawTriangle(previous_position, new_position, center, color);
    previous_position = new_position;
  }
  DrawTriangle(previous_position, base_position, center, color);
}

void PrimitiveRenderer::DrawConvexPolygon(const Vector3* positions, size_t num_positions, const Color& color) {
  SDL_assert(num_positions > 2);

  for (size_t i = 0; i < num_positions - 1; ++i) {
    DrawTriangle(positions[0], positions[i], positions[i + 1], color);
  }
}

void PrimitiveRenderer::AddVertices(std::span<const Vertex> vertices) {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);
  SDL_assert(vertices.size() % 3 == 0);

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
  SDL_assert(is_drawing_);
  SDL_assert(resources_);

  const size_t vertex_count = resources_->vertices.size();
  SDL_assert(vertex_count < VERTEX_BUFFER_ELEMENT_COUNT);
  SDL_assert(vertex_count % 3 == 0);

  if (resources_->vertices.size() == 0) {
    return;
  }

  resources_->vertex_buffer->Write(0, vertex_count * sizeof(Vertex), resources_->vertices.data());

  DrawItem draw_item;
  draw_item.vertex_input = resources_->vertex_input.get();
  draw_item.shader_program = resources_->shader.get();
  draw_item.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
  draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  draw_item.count = vertex_count;
  if (enable_alpha_blending_) {
    draw_item.blend_state.enabled = true;
    draw_item.blend_state.source_color_factor = SourceBlendFactor::SOURCE_ALPHA;
    draw_item.blend_state.destination_color_factor = DestinationBlendFactor::ONE_MINUS_SOURCE_ALPHA;
  }
  context()->Draw(draw_item);

  resources_->vertices.clear();
}

}  // namespace ovis
