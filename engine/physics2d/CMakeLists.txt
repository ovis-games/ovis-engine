add_library(
  ovis-physics2d
  
  include/ovis/physics2d/physics2d_module.hpp src/physics2d_module.cpp
  include/ovis/physics2d/physics_world2d.hpp src/physics_world2d.cpp
  include/ovis/physics2d/rigid_body2d.hpp src/rigid_body2d.cpp
  include/ovis/physics2d/physics2d_debug_layer.hpp src/physics2d_debug_layer.cpp
  include/ovis/physics2d/physics2d_contact_event.hpp src/physics2d_contact_event.cpp
  include/ovis/physics2d/physics2d_begin_contact_event.hpp src/physics2d_begin_contact_event.cpp
  include/ovis/physics2d/physics2d_end_contact_event.hpp src/physics2d_end_contact_event.cpp
  include/ovis/physics2d/physics2d_pre_solve_event.hpp src/physics2d_pre_solve_event.cpp
  include/ovis/physics2d/physics2d_post_solve_event.hpp src/physics2d_post_solve_event.cpp
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
    ovis::core
    box2d
)

target_add_assets(
  ovis-physics2d

  assets/physics2d.schema.json
)
