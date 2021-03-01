add_library(
  ovis-physics2d
  
  include/ovis/physics2d/physics2d_module.hpp src/physics2d_module.cpp
  include/ovis/physics2d/physics_world2d.hpp src/physics_world2d.cpp
  include/ovis/physics2d/rigid_body2d.hpp src/rigid_body2d.cpp
)
add_library(ovis::physics2d ALIAS ovis-physics2d)

target_include_directories(
  ovis-physics2d
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-physics2d
  PUBLIC
    ovis::engine
    ovis::base
    box2d
)

target_add_assets(
  ovis-physics2d

  assets/physics2d.schema.json
)
