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

add_custom_command(
  TARGET ovis-rendering2d
  PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
)

target_link_options(
 ovis-rendering2d
 PUBLIC
  --preload-file ${CMAKE_BINARY_DIR}/assets@/ovis_assets
)
