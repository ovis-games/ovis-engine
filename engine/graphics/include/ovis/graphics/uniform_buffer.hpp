#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>

#include <ovis/utils/log.hpp>
#include <ovis/graphics/cubemap.hpp>
#include <ovis/graphics/graphics_resource.hpp>
#include <ovis/graphics/texture2d.hpp>
#include <ovis/graphics/graphics_types.hpp>

namespace ovis {

class ShaderProgram;

struct UniformBufferDescription {
  ShaderProgram* shader_program;
};

class UniformBuffer : public GraphicsResource {
  friend class GraphicsDevice;
  friend class ShaderProgram;

  struct UniformDesciption {
    std::size_t offset;
    GLsizei size;
    GLint location;
    GLint base_texture_unit;
    GLenum type;
    std::string name;
  };

 public:
  UniformBuffer(GraphicsContext* context, const UniformBufferDescription& description);
  virtual ~UniformBuffer() override;

  template <typename T>
  inline void SetUniform(const std::string& name, T&& value) {
    const auto it_uniform = m_uniform_indices.find(name);
    if (it_uniform == m_uniform_indices.end()) {
      LogW("Trying to set unknown uniform: '{}'", name);
      return;
    }
    SDL_assert(m_uniform_descriptions[it_uniform->second].type == OpenGLType<std::remove_reference_t<T>>);
    memcpy(GetUniformPointer(it_uniform->second), &value, sizeof(value));
  }

  inline void SetTexture(const std::string& sampler_name, Texture2D* texture) {
    SDL_assert(m_uniform_indices.find(sampler_name) != m_uniform_indices.end());
    SDL_assert(m_uniform_indices[sampler_name] < m_uniform_descriptions.size());
    SDL_assert(m_uniform_descriptions[m_uniform_indices[sampler_name]].type == GL_SAMPLER_2D);
    SetTexture(m_uniform_descriptions[m_uniform_indices[sampler_name]].base_texture_unit, texture);
  }

  inline void SetTexture(const std::string& sampler_name, Cubemap* texture) {
    SDL_assert(m_uniform_indices.find(sampler_name) != m_uniform_indices.end());
    SDL_assert(m_uniform_indices[sampler_name] < m_uniform_descriptions.size());
    SDL_assert(m_uniform_descriptions[m_uniform_indices[sampler_name]].type == GL_SAMPLER_CUBE);
    SetTexture(m_uniform_descriptions[m_uniform_indices[sampler_name]].base_texture_unit, texture);
  }

  inline void SetTexture(const std::size_t sampler_index, Texture* texture) {
    SDL_assert(sampler_index < m_textures.size());
    m_textures[sampler_index] = texture;
  }

 private:
  std::unordered_map<std::string, std::size_t> m_uniform_indices;
  std::vector<UniformDesciption> m_uniform_descriptions;
  std::vector<GLbyte> m_uniform_buffer;
  std::vector<Texture*> m_textures;

  inline void* GetUniformBufferPointer(std::size_t offset) { return m_uniform_buffer.data() + offset; }

  inline void* GetUniformPointer(std::size_t uniform_index) {
    SDL_assert(uniform_index < m_uniform_descriptions.size());
    return GetUniformBufferPointer(m_uniform_descriptions[uniform_index].offset);
  }

  void Bind();
};

}  // namespace ovis
