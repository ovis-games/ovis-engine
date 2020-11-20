add_library(
  ovis-graphics
  
  include/ovis/graphics/blend_state.hpp
  include/ovis/graphics/cubemap.hpp src/cubemap.cpp
  include/ovis/graphics/depth_buffer_state.hpp
  include/ovis/graphics/gl.hpp
  include/ovis/graphics/gpu_time_profiler.hpp src/gpu_time_profiler.cpp
  include/ovis/graphics/graphics_buffer.hpp src/graphics_buffer.cpp
  include/ovis/graphics/graphics_context.hpp src/graphics_context.cpp
  include/ovis/graphics/graphics_resource.hpp src/graphics_resource.cpp
  include/ovis/graphics/index_buffer.hpp src/index_buffer.cpp
  include/ovis/graphics/render_target_configuration.hpp src/render_target_configuration.cpp
  include/ovis/graphics/render_target_texture2d.hpp src/render_target_texture2d.cpp
  include/ovis/graphics/render_target.hpp
  include/ovis/graphics/shader_program.hpp src/shader_program.cpp
  include/ovis/graphics/static_mesh.hpp
  include/ovis/graphics/stencil_state.hpp
  include/ovis/graphics/texture.hpp src/texture.cpp
  include/ovis/graphics/texture2d.hpp src/texture2d.cpp
  include/ovis/graphics/uniform_buffer.hpp src/uniform_buffer.cpp
  include/ovis/graphics/vertex_buffer.hpp src/vertex_buffer.cpp
  include/ovis/graphics/vertex_input.hpp src/vertex_input.cpp
)
add_library(ovis::graphics ALIAS ovis-graphics)

target_include_directories(
  ovis-graphics
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (APPLE)
  target_compile_definitions(
    ovis-graphics
    PUBLIC
      -DGL_SILENCE_DEPRECATION
  )
endif ()

target_include_directories(
  ovis-graphics
  PUBLIC
    ${OVIS_ROOT}/extern/glm/include
)

target_link_libraries(
  ovis-graphics
  PUBLIC
    ovis::core
    ovis::math
)

if (NOT OVIS_EMSCRIPTEN)
  find_package(OpenGL REQUIRED)
  target_link_libraries(
    ovis-graphics
    PUBLIC
      OpenGL::GL
  )
endif (NOT OVIS_EMSCRIPTEN)

if (WIN32)
  find_package(GLEW REQUIRED)
  target_include_directories(
    ovis-graphics
    PUBLIC
      ${GLEW_INCLUDE_DIRS}
  )
  target_link_libraries(
    ovis-graphics
    PUBLIC
      ${GLEW_LIBRARIES}
  )
endif ()
