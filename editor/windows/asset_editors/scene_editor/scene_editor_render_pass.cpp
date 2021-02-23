#include "scene_editor_render_pass.hpp"

#include <ovis/graphics/graphics_context.hpp>
#include <ovis/engine/viewport.hpp>
#include <ovis/base/camera_component.hpp>

namespace ovis {
namespace editor {

SceneEditorRenderPass::SceneEditorRenderPass(SceneEditor* scene_editor) : RenderPass("SceneEditorRenderPass") {
  RenderAfter("SpriteRenderer");
}

void SceneEditorRenderPass::CreateResources() {
  line_shader_ = LoadShaderProgram("line", context());
  VertexBufferDescription line_buffer_desc;
  line_buffer_desc.vertex_size_in_bytes = sizeof(LineVertex);
  line_buffer_desc.size_in_bytes = sizeof(LineVertex) * vertex_buffer_size;
  line_vertex_buffer_ = std::make_unique<ovis::VertexBuffer>(context(), line_buffer_desc);

  VertexInputDescription vertex_input_desc;
  vertex_input_desc.vertex_buffers = {line_vertex_buffer_.get()};
  vertex_input_desc.vertex_attributes = {
      {*line_shader_->GetAttributeLocation("Position"), 0, 0, VertexAttributeType::FLOAT32_VECTOR3},
      {*line_shader_->GetAttributeLocation("Color"), 12, 0, VertexAttributeType::UINT8_NORM_VECTOR4}};
  line_vertex_input = std::make_unique<VertexInput>(context(), vertex_input_desc);
}

void SceneEditorRenderPass::ReleaseResources() {
  line_shader_.reset();
}

void SceneEditorRenderPass::Render(const RenderContext& render_context) {
  line_vertices_.clear();

  auto objects_with_cameras = render_context.scene->GetSceneObjectsWithComponent("Camera");

  for (SceneObject* object : objects_with_cameras) {
    CameraComponent* camera = object->GetComponent<CameraComponent>("Camera");
    vector3 near_top_left = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(-1.0f, 1.0f, -1.0f));
    vector3 near_bottom_left = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(-1.0f, -1.0f, -1.0f));
    vector3 near_top_right = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(1.0f, 1.0f, -1.0f));
    vector3 near_bottom_right = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(1.0f, -1.0f, -1.0f));
    vector3 far_top_left = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(-1.0f, 1.0f, 1.0f));
    vector3 far_bottom_left = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(-1.0f, -1.0f, 1.0f));
    vector3 far_top_right = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(1.0f, 1.0f, 1.0f));
    vector3 far_bottom_right = camera->NormalizedDeviceCoordinatesToWorldSpace(vector3(1.0f, -1.0f, 1.0f));

    // Near plane
    RenderLine(near_top_left, near_top_right);
    RenderLine(near_top_left, near_bottom_left);
    RenderLine(near_bottom_right, near_top_right);
    RenderLine(near_bottom_right, near_bottom_left);

    // Far plane
    RenderLine(far_top_left, far_top_right);
    RenderLine(far_top_left, far_bottom_left);
    RenderLine(far_bottom_right, far_top_right);
    RenderLine(far_bottom_right, far_bottom_left);

    // Connecting corners
    RenderLine(near_top_left, far_top_left);
    RenderLine(near_top_right, far_top_right);
    RenderLine(near_bottom_left, far_bottom_left);
    RenderLine(near_bottom_right, far_bottom_right);
  }

  line_shader_->SetUniform("WorldViewProjection", render_context.view_projection_matrix);

  DrawItem draw_item;
  draw_item.vertex_input = line_vertex_input.get();
  draw_item.shader_program = line_shader_.get();
  draw_item.primitive_topology = PrimitiveTopology::LINE_LIST;
  draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();

  size_t line_vertex_offset = 0;
  while (line_vertex_offset < line_vertices_.size()) {
    const size_t vertices_to_render = std::min(line_vertices_.size() - line_vertex_offset, vertex_buffer_size);
    line_vertex_buffer_->Write(0, vertices_to_render * sizeof(LineVertex), line_vertices_.data() + line_vertex_offset);
    draw_item.count = vertices_to_render;
    context()->Draw(draw_item);

    line_vertex_offset += vertices_to_render;
  }
}

void SceneEditorRenderPass::RenderLine(vector3 start, vector3 end, vector4 color) {
  RenderLine(start, color, end, color);
}

void SceneEditorRenderPass::RenderLine(vector3 start, vector4 start_color, vector3 end, vector4 end_color) {
  {
    LineVertex start_vertex;
    start_vertex.position = start;
    start_vertex.color = start_color * 255.0f;
    line_vertices_.push_back(start_vertex);
  }
  {
    LineVertex end_vertex;
    end_vertex.position = end;
    end_vertex.color = end_color * 255.0f;
    line_vertices_.push_back(end_vertex);
  }
}

void SceneEditorRenderPass::RenderLineLoop(std::vector<vector3> points, vector4 color) {
  
}

}  // namespace editor

}  // namespace ovis
