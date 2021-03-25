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

namespace ovis {

class ShaderProgram;

struct UniformBufferDescription {
  ShaderProgram* shader_program;
};

template <typename T>
struct OpenGLTypeHelper;

#define OVIS_UNFORM_OPENGL_TYPE(cpp_type, opengl_type) \
  template <>                                          \
  struct OpenGLTypeHelper<cpp_type> {                  \
    static constexpr GLenum value = opengl_type;       \
  }

using Vec2 = std::array<float, 2>;
using Vec3 = std::array<float, 3>;
using Vec4 = std::array<float, 4>;

using IntVec2 = std::array<int, 2>;
using IntVec3 = std::array<int, 3>;
using IntVec4 = std::array<int, 4>;

using BoolVec2 = std::array<bool, 2>;
using BoolVec3 = std::array<bool, 3>;
using BoolVec4 = std::array<bool, 4>;

using Mat2x2 = std::array<std::array<float, 2>, 2>;
using Mat3x3 = std::array<std::array<float, 3>, 3>;
using Mat4x4 = std::array<std::array<float, 4>, 4>;

OVIS_UNFORM_OPENGL_TYPE(float, GL_FLOAT);
OVIS_UNFORM_OPENGL_TYPE(Vec2, GL_FLOAT_VEC2);
OVIS_UNFORM_OPENGL_TYPE(Vec3, GL_FLOAT_VEC3);
OVIS_UNFORM_OPENGL_TYPE(Vec4, GL_FLOAT_VEC4);

OVIS_UNFORM_OPENGL_TYPE(int, GL_INT);
OVIS_UNFORM_OPENGL_TYPE(IntVec2, GL_INT_VEC2);
OVIS_UNFORM_OPENGL_TYPE(IntVec3, GL_INT_VEC3);
OVIS_UNFORM_OPENGL_TYPE(IntVec4, GL_INT_VEC4);

OVIS_UNFORM_OPENGL_TYPE(bool, GL_BOOL);
OVIS_UNFORM_OPENGL_TYPE(BoolVec2, GL_BOOL_VEC2);
OVIS_UNFORM_OPENGL_TYPE(BoolVec3, GL_BOOL_VEC3);
OVIS_UNFORM_OPENGL_TYPE(BoolVec4, GL_BOOL_VEC4);

OVIS_UNFORM_OPENGL_TYPE(Mat2x2, GL_FLOAT_MAT2);
OVIS_UNFORM_OPENGL_TYPE(Mat3x3, GL_FLOAT_MAT3);
OVIS_UNFORM_OPENGL_TYPE(Mat4x4, GL_FLOAT_MAT4);

template <typename T>
inline constexpr GLenum OpenGLType = OpenGLTypeHelper<T>::value;

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
