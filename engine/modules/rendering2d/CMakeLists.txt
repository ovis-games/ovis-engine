add_library(
  ovis-rendering2d
  
  include/ovis/rendering2d/rendering2d_module.hpp src/rendering2d_module.cpp
  include/ovis/rendering2d/sprite_renderer.hpp src/sprite_renderer.cpp
  include/ovis/rendering2d/sprite_component.hpp src/sprite_component.cpp
)
add_library(ovis::rendering2d ALIAS ovis-rendering2d)

target_include_directories(
  ovis-rendering2d
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-rendering2d
  PUBLIC
    ovis::engine
    ovis::base
)

target_add_assets(
  ovis-rendering2d

  assets/rendering2d.schema.json
  assets/sprite.shader.vert
  assets/sprite.shader.frag
)
