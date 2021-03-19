#include <ovis/rendering/debug_render_pass.hpp>
#include <ovis/rendering/viewport.hpp>

namespace ovis {

DebugRenderPass::DebugRenderPass(const std::string& name) : RenderPass(name) {}

void DebugRenderPass::CreateResources() {
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

  LogV("Creating DebugRenderPass resources for context {}", static_cast<void*>(context()));
  resources_ = std::make_shared<Resources>();

  resources_->point_shader = LoadShaderProgram("debug_pass_point", context());

  VertexBufferDescription point_buffer_desc;
  point_buffer_desc.vertex_size_in_bytes = sizeof(PointVertex);
  point_buffer_desc.size_in_bytes = sizeof(PointVertex) * VERTEX_BUFFER_ELEMENT_COUNT;
  resources_->point_vertex_buffer = std::make_unique<ovis::VertexBuffer>(context(), point_buffer_desc);

  VertexInputDescription point_vertex_input_desc;
  point_vertex_input_desc.vertex_buffers = {resources_->point_vertex_buffer.get()};
  point_vertex_input_desc.vertex_attributes = {
      {*resources_->point_shader->GetAttributeLocation("Position"), 0, 0, VertexAttributeType::FLOAT32_VECTOR3},
      {*resources_->point_shader->GetAttributeLocation("Size"), 12, 0, VertexAttributeType::FLOAT32},
      {*resources_->point_shader->GetAttributeLocation("Color"), 16, 0, VertexAttributeType::UINT8_NORM_VECTOR4}};
  resources_->point_vertex_input = std::make_unique<VertexInput>(context(), point_vertex_input_desc);

  resources_->shader = LoadShaderProgram("debug_pass", context());

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

  resources_->point_vertices.reserve(POINT_VERTEX_BUFFER_ELEMENT_COUNT);
  resources_->line_vertices.reserve(VERTEX_BUFFER_ELEMENT_COUNT);
  resources_->triangle_vertices.reserve(VERTEX_BUFFER_ELEMENT_COUNT);

  // resources.insert(std::make_pair(context(), resources_));
}

void DebugRenderPass::ReleaseResources() {
  resources_.reset();
}

void DebugRenderPass::SetDrawSpace(DrawSpace space) {
  SDL_assert(!is_drawing_);
  draw_space_ = space;
}

void DebugRenderPass::BeginDraw(const RenderContext& render_context) {
  SDL_assert(!is_drawing_);
  SDL_assert(resources_);

  is_drawing_ = true;
  resources_->point_vertices.clear();
  resources_->line_vertices.clear();
  resources_->triangle_vertices.clear();

  Matrix4 view_projection;
  if (draw_space_ == DrawSpace::WORLD) {
    view_projection = render_context.view_projection_matrix;
  } else {
    const Vector2 viewport_size = viewport()->GetDimensionsAsVector2();
    view_projection = Matrix4::FromOrthographicProjection(0.0f, viewport_size.x, viewport_size.y, 0.0f, -1.0f, 1.0f);
  }

  resources_->point_shader->SetUniform("ViewProjection", view_projection);
  resources_->shader->SetUniform("ViewProjection", view_projection);
}

void DebugRenderPass::EndDraw() {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);

  FlushPoints();
  FlushLines();
  FlushTriangles();
  is_drawing_ = false;
}

void DebugRenderPass::DrawPoint(const Vector3& v0, float size, const Color& color) {
  PointVertex vertex{{v0.x, v0.y, v0.z}, size, ConvertToRGBA8(color)};
  DrawPoints(&vertex, 1);
}

void DebugRenderPass::DrawPoints(const PointVertex* vertices, size_t num_vertices) {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);

  const size_t remaining_space = POINT_VERTEX_BUFFER_ELEMENT_COUNT - resources_->point_vertices.size();
  const size_t vertices_to_copy = std::min(num_vertices, remaining_space);
  resources_->point_vertices.insert(resources_->point_vertices.end(), vertices, vertices + vertices_to_copy);

  if (vertices_to_copy < num_vertices) {
    // We could fit all vertices, flush buffer and try again
    FlushPoints();
    DrawPoints(vertices + vertices_to_copy, num_vertices - vertices_to_copy);
  }
}

void DebugRenderPass::DrawLine(const Vector3& v0, const Vector3& v1, const Color& color) {
  uint32_t converted_color = ConvertToRGBA8(color);
  Vertex vertices[2] = {{{v0.x, v0.y, v0.z}, converted_color}, {{v1.x, v1.y, v1.z}, converted_color}};
  DrawLines(vertices, 2);
}

void DebugRenderPass::DrawLoop(const Vector3* positions, size_t num_positions, const Color& color) {
  SDL_assert(num_positions > 2);

  for (size_t i = 0; i < num_positions - 1; ++i) {
    DrawLine(positions[i], positions[i + 1], color);
  }
  DrawLine(positions[num_positions - 1], positions[0], color);
}

void DebugRenderPass::DrawCircle(const Vector3& center, float radius, const Color& color, size_t num_segments,
                                 const Vector3& support_vector0, const Vector3& support_vector1) {
  SDL_assert(num_segments > 2);

  const Vector3 base_position = center + radius * support_vector1;
  Vector3 previous_position = base_position;
  for (int i = 1; i < num_segments; ++i) {
    const float angle = i * 2.0f * glm::pi<float>() / num_segments;

    const Vector3 new_position =
        center + radius * (std::sin(angle) * support_vector0 + std::cos(angle) * support_vector1);
    DrawLine(previous_position, new_position, color);
    previous_position = new_position;
  }
  DrawLine(previous_position, base_position, color);
}

void DebugRenderPass::DrawLines(const Vertex* vertices, size_t num_vertices) {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);
  SDL_assert(num_vertices % 2 == 0);

  const size_t remaining_space = VERTEX_BUFFER_ELEMENT_COUNT - resources_->line_vertices.size();
  const size_t vertices_to_copy = (std::min(num_vertices, remaining_space) / 2) * 2;
  resources_->line_vertices.insert(resources_->line_vertices.end(), vertices, vertices + vertices_to_copy);

  if (vertices_to_copy < num_vertices) {
    // We could fit all vertices, flush buffer and try again
    FlushLines();
    DrawLines(vertices + vertices_to_copy, num_vertices - vertices_to_copy);
  }
}

void DebugRenderPass::DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color) {
  uint32_t converted_color = ConvertToRGBA8(color);
  Vertex vertices[3] = {{{v0.x, v0.y, v0.z}, converted_color},
                        {{v1.x, v1.y, v1.z}, converted_color},
                        {{v2.x, v2.y, v2.z}, converted_color}};
  DrawTriangles(vertices, 3);
}

void DebugRenderPass::DrawDisc(const Vector3& center, float radius, const Color& color, size_t num_segments,
                               const Vector3& support_vector0, const Vector3& support_vector1) {
  SDL_assert(num_segments > 2);

  const Vector3 base_position = center + radius * support_vector1;
  Vector3 previous_position = base_position;
  for (int i = 1; i < num_segments; ++i) {
    const float angle = i * 2.0f * glm::pi<float>() / num_segments;

    const Vector3 new_position =
        center + radius * (std::sin(angle) * support_vector0 + std::cos(angle) * support_vector1);
    DrawTriangle(previous_position, new_position, center, color);
    previous_position = new_position;
  }
  DrawTriangle(previous_position, base_position, center, color);
}

void DebugRenderPass::DrawConvexPolygon(const Vector3* positions, size_t num_positions, const Color& color) {
  SDL_assert(num_positions > 2);

  for (size_t i = 0; i < num_positions - 1; ++i) {
    DrawTriangle(positions[0], positions[i], positions[i + 1], color);
  }
}

void DebugRenderPass::DrawTriangles(const Vertex* vertices, size_t num_vertices) {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);
  SDL_assert(num_vertices % 3 == 0);

  const size_t remaining_space = VERTEX_BUFFER_ELEMENT_COUNT - resources_->triangle_vertices.size();
  const size_t vertices_to_copy = (std::min(num_vertices, remaining_space) / 3) * 3;
  resources_->triangle_vertices.insert(resources_->triangle_vertices.end(), vertices, vertices + vertices_to_copy);

  if (vertices_to_copy < num_vertices) {
    // We could fit all vertices, flush buffer and try again
    FlushTriangles();
    DrawTriangles(vertices + vertices_to_copy, num_vertices - vertices_to_copy);
  }
}

void DebugRenderPass::FlushPoints() {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);

  const size_t vertex_count = resources_->point_vertices.size();
  SDL_assert(vertex_count < VERTEX_BUFFER_ELEMENT_COUNT);

  if (resources_->point_vertices.size() == 0) {
    return;
  }

  resources_->point_vertex_buffer->Write(0, vertex_count * sizeof(PointVertex), resources_->point_vertices.data());

  DrawItem draw_item;
  draw_item.vertex_input = resources_->point_vertex_input.get();
  draw_item.shader_program = resources_->point_shader.get();
  draw_item.primitive_topology = PrimitiveTopology::POINTS;
  draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  draw_item.count = vertex_count;
  if (enable_alpha_blending_) {
    draw_item.blend_state.enabled = true;
    draw_item.blend_state.source_color_factor = SourceBlendFactor::SOURCE_ALPHA;
    draw_item.blend_state.destination_color_factor = DestinationBlendFactor::ONE_MINUS_SOURCE_ALPHA;
  }
  context()->Draw(draw_item);

  resources_->point_vertices.clear();
}

void DebugRenderPass::FlushLines() {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);

  const size_t vertex_count = resources_->line_vertices.size();
  SDL_assert(vertex_count < VERTEX_BUFFER_ELEMENT_COUNT);
  SDL_assert(vertex_count % 2 == 0);

  if (vertex_count == 0) {
    return;
  }

  resources_->vertex_buffer->Write(0, vertex_count * sizeof(Vertex), resources_->line_vertices.data());

  DrawItem draw_item;
  draw_item.vertex_input = resources_->vertex_input.get();
  draw_item.shader_program = resources_->shader.get();
  draw_item.primitive_topology = PrimitiveTopology::LINE_LIST;
  draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  draw_item.count = vertex_count;
  if (enable_alpha_blending_) {
    draw_item.blend_state.enabled = true;
    draw_item.blend_state.source_color_factor = SourceBlendFactor::SOURCE_ALPHA;
    draw_item.blend_state.destination_color_factor = DestinationBlendFactor::ONE_MINUS_SOURCE_ALPHA;
  }
  context()->Draw(draw_item);

  resources_->line_vertices.clear();
}

void DebugRenderPass::FlushTriangles() {
  SDL_assert(is_drawing_);
  SDL_assert(resources_);

  const size_t vertex_count = resources_->triangle_vertices.size();
  SDL_assert(vertex_count < VERTEX_BUFFER_ELEMENT_COUNT);
  SDL_assert(vertex_count % 3 == 0);

  if (resources_->triangle_vertices.size() == 0) {
    return;
  }

  resources_->vertex_buffer->Write(0, vertex_count * sizeof(Vertex), resources_->triangle_vertices.data());

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

  resources_->triangle_vertices.clear();
}

}  // namespace ovis
