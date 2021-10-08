add_library(
  ovis-rendering2d
  
  include/ovis/rendering2d/rendering2d_module.hpp src/rendering2d_module.cpp
  include/ovis/rendering2d/renderer2d.hpp src/renderer2d.cpp
  include/ovis/rendering2d/shape2d.hpp src/shape2d.cpp
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
    ovis::rendering
)

target_add_assets(
  ovis-rendering2d

  assets/rendering2d.schema.json
  assets/shape2d.shader.vert
  assets/shape2d.shader.frag
)
