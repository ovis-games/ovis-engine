add_library(
  ovis-engine
  
  include/ovis/engine/engine.hpp src/engine.cpp
  include/ovis/engine/viewport.hpp src/viewport.cpp
  include/ovis/engine/window.hpp src/window.cpp
  include/ovis/engine/render_target_viewport.hpp src/render_target_viewport.cpp
  include/ovis/engine/camera.hpp src/camera.cpp
  include/ovis/engine/scene_object_component.hpp src/scene_object_component.cpp
  include/ovis/engine/scene_controller.hpp src/scene_controller.cpp
  include/ovis/engine/script_scene_controller.hpp src/script_scene_controller.cpp
  include/ovis/engine/scene_object.hpp src/scene_object.cpp
  include/ovis/engine/scene.hpp src/scene.cpp
  include/ovis/engine/module.hpp src/module.cpp
  include/ovis/engine/render_pass.hpp src/render_pass.cpp
  include/ovis/engine/lua.hpp src/lua.cpp
)
add_library(ovis::engine ALIAS ovis-engine)

target_include_directories(
  ovis-engine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-engine
  PUBLIC
    ovis::core
    ovis::math
    ovis::graphics
    DearImGui
    Lua
    sol2_single
    middleclass
)
