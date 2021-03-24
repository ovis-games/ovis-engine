// #include "scene_editor_render_pass.hpp"

// #include <cmath>

// #include <glm/gtc/constants.hpp>
// #include <ovis/base/camera_component.hpp>

// #include <ovis/graphics/graphics_context.hpp>
// #include <ovis/rendering/rendering_viewport.hpp>
// #include <ovis/physics2d/physics_world2d.hpp>
// #include <ovis/utils/log.hpp>

// namespace ovis {
// namespace editor {

// SceneEditorRenderPass::SceneEditorRenderPass(SceneEditor* scene_editor) : RenderPass("SceneEditorRenderPass") {
//   RenderAfter("SpriteRenderer");
//   RenderBefore("Physics2DDebugLayer");
// }

// void SceneEditorRenderPass::CreateResources() {
//   line_shader_ = LoadShaderProgram("line", context());
//   VertexBufferDescription line_buffer_desc;
//   line_buffer_desc.vertex_size_in_bytes = sizeof(LineVertex);
//   line_buffer_desc.size_in_bytes = sizeof(LineVertex) * vertex_buffer_size;
//   line_vertex_buffer_ = std::make_unique<ovis::VertexBuffer>(context(), line_buffer_desc);

//   VertexInputDescription vertex_input_desc;
//   vertex_input_desc.vertex_buffers = {line_vertex_buffer_.get()};
//   vertex_input_desc.vertex_attributes = {
//       {*line_shader_->GetAttributeLocation("Position"), 0, 0, VertexAttributeType::FLOAT32_VECTOR3},
//       {*line_shader_->GetAttributeLocation("Color"), 12, 0, VertexAttributeType::UINT8_NORM_VECTOR4}};
//   line_vertex_input = std::make_unique<VertexInput>(context(), vertex_input_desc);
  
  
//   SetFlags(e_shapeBit | e_jointBit | e_jointBit | e_aabbBit | e_pairBit | e_centerOfMassBit);
// }

// void SceneEditorRenderPass::ReleaseResources() {
//   line_shader_.reset();
// }

// void SceneEditorRenderPass::Render(const RenderContext& render_context) {
//   is_rendering_ = true;
//   line_vertices_.clear();


//   auto objects_with_cameras = render_context.scene->GetSceneObjectsWithComponent("Camera");

//   for (SceneObject* object : objects_with_cameras) {
//     CameraComponent* camera = object->GetComponent<CameraComponent>("Camera");
//     Vector3 near_top_left = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(-1.0f, 1.0f, -1.0f));
//     Vector3 near_bottom_left = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(-1.0f, -1.0f, -1.0f));
//     Vector3 near_top_right = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(1.0f, 1.0f, -1.0f));
//     Vector3 near_bottom_right = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(1.0f, -1.0f, -1.0f));
//     Vector3 far_top_left = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(-1.0f, 1.0f, 1.0f));
//     Vector3 far_bottom_left = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(-1.0f, -1.0f, 1.0f));
//     Vector3 far_top_right = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(1.0f, 1.0f, 1.0f));
//     Vector3 far_bottom_right = camera->NormalizedDeviceCoordinatesToWorldSpace(Vector3(1.0f, -1.0f, 1.0f));

//     // Near plane
//     RenderLine(near_top_left, near_top_right);
//     RenderLine(near_top_left, near_bottom_left);
//     RenderLine(near_bottom_right, near_top_right);
//     RenderLine(near_bottom_right, near_bottom_left);

//     // Far plane
//     RenderLine(far_top_left, far_top_right);
//     RenderLine(far_top_left, far_bottom_left);
//     RenderLine(far_bottom_right, far_top_right);
//     RenderLine(far_bottom_right, far_bottom_left);

//     // Connecting corners
//     RenderLine(near_top_left, far_top_left);
//     RenderLine(near_top_right, far_top_right);
//     RenderLine(near_bottom_left, far_bottom_left);
//     RenderLine(near_bottom_right, far_bottom_right);
//   }

//   line_shader_->SetUniform("WorldViewProjection", render_context.view_projection_matrix);

//   DrawItem draw_item;
//   draw_item.vertex_input = line_vertex_input.get();
//   draw_item.shader_program = line_shader_.get();
//   draw_item.primitive_topology = PrimitiveTopology::LINE_LIST;
//   draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();

//   size_t line_vertex_offset = 0;
//   while (line_vertex_offset < line_vertices_.size()) {
//     const size_t vertices_to_render = std::min(line_vertices_.size() - line_vertex_offset, vertex_buffer_size);
//     line_vertex_buffer_->Write(0, vertices_to_render * sizeof(LineVertex), line_vertices_.data() + line_vertex_offset);
//     draw_item.count = vertices_to_render;
//     context()->Draw(draw_item);

//     line_vertex_offset += vertices_to_render;
//   }

//   is_rendering_ = false;
// }

// void SceneEditorRenderPass::DrawPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color) {
//   SDL_assert(is_rendering_);

//   std::vector<Vector3> points(vertex_count);
//   for (int i = 0; i < vertex_count; ++i) {
//     points[i].x = vertices[i].x;
//     points[i].y = vertices[i].y;
//   }
//   RenderLineLoop(points, {color.r, color.g, color.b, color.a});
// }

// void SceneEditorRenderPass::DrawSolidPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color) {
//   DrawPolygon(vertices, vertex_count, color);
// }

// void SceneEditorRenderPass::DrawCircle(const b2Vec2& center, float radius, const b2Color& color) {
//   SDL_assert(is_rendering_);

//   constexpr int CIRCLE_VERTEX_COUNT = 20;

//   std::vector<Vector3> circle_vertices(CIRCLE_VERTEX_COUNT);
//   for (int i = 0; i < CIRCLE_VERTEX_COUNT; ++i) {
//     const float angle = i * 2.0f * glm::pi<float>() / CIRCLE_VERTEX_COUNT;

//     circle_vertices[i].x = center.x + std::sin(angle) * radius;
//     circle_vertices[i].y = center.y + std::cos(angle) * radius;
//   }
//   RenderLineLoop(circle_vertices, {color.r, color.g, color.b, color.a});
// }

// void SceneEditorRenderPass::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis,
//                                             const b2Color& color) {
//   DrawCircle(center, radius, color);
// }

// void SceneEditorRenderPass::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
//   RenderLine({p1.x, p1.y, 0.0f}, {p2.x, p2.y, 0.0f}, {color.r, color.g, color.b, color.a});
// }

// void SceneEditorRenderPass::DrawTransform(const b2Transform& xf) {
//   DrawSegment(xf.p, xf.p + b2Vec2(xf.q.c, xf.q.s), b2Color(1.0f, 0.0f, 0.0f));
//   DrawSegment(xf.p, xf.p + b2Vec2(xf.q.s, xf.q.c), b2Color(0.0f, 1.0f, 0.0f));
// }

// void SceneEditorRenderPass::DrawPoint(const b2Vec2& p, float size, const b2Color& color) {
//   DrawSolidCircle(p, size, b2Vec2(), color);
// }

// void SceneEditorRenderPass::RenderLine(Vector3 start, Vector3 end, vector4 color) {
//   RenderLine(start, color, end, color);
// }

// void SceneEditorRenderPass::RenderLine(Vector3 start, vector4 start_color, Vector3 end, vector4 end_color) {
//   {
//     LineVertex start_vertex;
//     start_vertex.position = start;
//     start_vertex.color = start_color * 255.0f;
//     line_vertices_.push_back(start_vertex);
//   }
//   {
//     LineVertex end_vertex;
//     end_vertex.position = end;
//     end_vertex.color = end_color * 255.0f;
//     line_vertices_.push_back(end_vertex);
//   }
// }

// void SceneEditorRenderPass::RenderLineLoop(std::vector<Vector3> points, vector4 color) {
//   if (points.size() < 2) {
//     return;
//   }

//   for (size_t i = 0; i < points.size() - 1; ++i) {
//     RenderLine(points[i], points[i + 1], color);
//   }
//   if (points.size() > 2) {
//     RenderLine(points.back(), points.front(), color);
//   }
// }

// }  // namespace editor

// }  // namespace ovis
