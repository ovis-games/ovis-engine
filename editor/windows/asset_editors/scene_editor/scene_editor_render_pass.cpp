#include "scene_editor_render_pass.hpp"

#include <ovis/graphics/graphics_context.hpp>
#include <ovis/engine/viewport.hpp>

namespace ovis {
namespace editor {

SceneEditorRenderPass::SceneEditorRenderPass(SceneEditor* scene_editor) : RenderPass("SceneEditorRenderPass") {
  RenderAfter("SpriteRenderer");
}

void SceneEditorRenderPass::CreateResources() {
  line_shader_ = LoadShaderProgram("line", context());

  line_vertices_.resize(2);
  line_vertices_[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
  line_vertices_[0].color = glm::tvec4<std::uint8_t>(255, 0, 0, 255);
  line_vertices_[1].position = glm::vec3(1.0f, 1.0f, 0.0f);
  line_vertices_[1].color = glm::tvec4<std::uint8_t>(255, 0, 0, 255);

  VertexBufferDescription line_buffer_desc;
  line_buffer_desc.vertex_size_in_bytes = sizeof(LineVertex);
  line_buffer_desc.size_in_bytes = sizeof(LineVertex) * 2;
  line_vertex_buffer_ = std::make_unique<ovis::VertexBuffer>(context(), line_buffer_desc, line_vertices_.data());

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

void SceneEditorRenderPass::Render(Scene* scene) {
  line_shader_->SetUniform("WorldViewProjection", glm::mat4());

  DrawItem draw_item;
  draw_item.vertex_input = line_vertex_input.get();
  draw_item.shader_program = line_shader_.get();
  draw_item.count = 2;
  draw_item.primitive_topology = PrimitiveTopology::LINE_LIST;
  draw_item.render_target_configuration = viewport()->GetDefaultRenderTargetConfiguration();
  context()->Draw(draw_item);
}

}  // namespace editor

}  // namespace ovis
