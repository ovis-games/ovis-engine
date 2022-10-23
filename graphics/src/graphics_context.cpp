#include "ovis/graphics/graphics_context.hpp"

#include "ovis/utils/log.hpp"
#include "ovis/graphics/index_buffer.hpp"
#include "ovis/graphics/render_target_configuration.hpp"
#include "ovis/graphics/shader_program.hpp"
#include "ovis/graphics/vertex_input.hpp"

namespace ovis {

GraphicsContext::GraphicsContext(Vector2 framebuffer_dimensions)
    : m_bound_array_buffer(0),
      m_bound_element_array_buffer(0),
      m_bound_program(0),
      m_active_texture_unit(0),
      scissoring_enabled_(false) {

#if _WIN32
  glewInit();
#endif

  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_caps.max_vertex_attribs);
  assert(m_caps.max_vertex_attribs >= 8);
  m_vertex_attrib_array_states.resize(m_caps.max_vertex_attribs, false);

  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_caps.num_texture_units);
  assert(m_caps.num_texture_units >= 8);
  m_bound_textures.resize(m_caps.num_texture_units, 0);

  glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_caps.num_vertex_texture_units);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  viewport_width_ = framebuffer_dimensions.x;
  viewport_height_ = framebuffer_dimensions.y;

  m_default_render_target_configuration.reset(new RenderTargetConfiguration(this, framebuffer_dimensions.x, framebuffer_dimensions.y));

#if !defined(__IPHONEOS__) && !defined(__EMSCRIPTEN__)
  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
#endif
}

GraphicsContext::~GraphicsContext() {
  m_default_render_target_configuration.reset();
  for (auto& resource : resources_) {
    assert(resource->type() == GraphicsResource::Type::NONE);
  }
}

void GraphicsContext::SetFramebufferSize(int width, int height) {
  m_default_render_target_configuration->width_ = width;
  m_default_render_target_configuration->height_ = height;
}

void GraphicsContext::Draw(const DrawItem& draw_item) {
  assert(draw_item.shader_program != nullptr);
  // assert(draw_item.vertex_input != nullptr);

  ApplyBlendState(&blend_state_, draw_item.blend_state);
  ApplyDepthBufferState(&depth_buffer_state_, draw_item.depth_buffer_state);

  auto targets = draw_item.render_target_configuration != nullptr ? draw_item.render_target_configuration
                                                                  : default_render_target_configuration();
  targets->Bind();
  if (targets->width() != viewport_width_ || targets->height() != viewport_height_) {
    glViewport(0, 0, targets->width(), targets->height());
    viewport_width_ = targets->width();
    viewport_height_ = targets->height();
  }

  if (draw_item.scissor_rect.has_value() != scissoring_enabled_) {
    if (draw_item.scissor_rect) {
      glEnable(GL_SCISSOR_TEST);
      scissoring_enabled_ = true;
    } else {
      glDisable(GL_SCISSOR_TEST);
      scissoring_enabled_ = false;
    }
  }
  if (draw_item.scissor_rect.has_value() && *draw_item.scissor_rect != current_scissor_rect_) {
    const int bottom = targets->height() - draw_item.scissor_rect->top - draw_item.scissor_rect->height;
    glScissor(draw_item.scissor_rect->left, bottom, draw_item.scissor_rect->width, draw_item.scissor_rect->height);
    current_scissor_rect_ = *draw_item.scissor_rect;
  }

  if (culling_enabled_ != draw_item.enable_culling) {
    if (draw_item.enable_culling) {
      glEnable(GL_CULL_FACE);
    } else {
      glDisable(GL_CULL_FACE);
    }
    culling_enabled_ = draw_item.enable_culling;
  }

  draw_item.shader_program->Bind();
  if (draw_item.vertex_input != nullptr) {
    draw_item.vertex_input->Bind();
  }
  const GLenum primitive_topology = static_cast<GLenum>(draw_item.primitive_topology);

  if (draw_item.index_buffer == nullptr) {
#ifdef DEBUG
    glValidateProgram(draw_item.shader_program->m_program_name);
    GLint validation_status = 0;
    glGetProgramiv(draw_item.shader_program->m_program_name, GL_VALIDATE_STATUS, &validation_status);
    assert(validation_status == GL_TRUE);
#endif

    glDrawArrays(primitive_topology, draw_item.start, draw_item.count);
  } else {
    draw_item.index_buffer->Bind();

#ifdef DEBUG
    glValidateProgram(draw_item.shader_program->m_program_name);
    GLint validation_status = 0;
    glGetProgramiv(draw_item.shader_program->m_program_name, GL_VALIDATE_STATUS, &validation_status);
    assert(validation_status == GL_TRUE);
#endif

    const GLenum index_type = static_cast<GLenum>(draw_item.index_buffer->description().index_format);
    const auto index_offset_in_bytes = draw_item.start * draw_item.index_buffer->bytes_per_index();
#if !OVIS_EMSCRIPTEN
    glDrawElementsBaseVertex(primitive_topology, draw_item.count, index_type,
                             reinterpret_cast<GLvoid*>(index_offset_in_bytes), draw_item.base_vertex);
#else
    assert(draw_item.base_vertex == 0);
    glDrawElements(primitive_topology, draw_item.count, index_type, reinterpret_cast<GLvoid*>(index_offset_in_bytes));
#endif
  }
}

}  // namespace ovis
