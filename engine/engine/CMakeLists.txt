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
  include/ovis/engine/debug_render_pass.hpp src/debug_render_pass.cpp
  include/ovis/engine/lua.hpp src/lua.cpp
  include/ovis/engine/fetch.hpp src/fetch.cpp
  include/ovis/engine/game_settings.hpp src/game_settings.cpp
  include/ovis/engine/lua_module.hpp src/lua_module.cpp
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
)

target_compile_options(
  ovis-engine
  PUBLIC
    "SHELL:-s FETCH=1"
)
target_link_options(
  ovis-engine
  PUBLIC
    "SHELL:-s FETCH=1"
)

target_add_assets(
  ovis-engine

  assets/engine.schema.json
  assets/debug_pass_point.shader.vert assets/debug_pass_point.shader.frag
  assets/debug_pass.shader.vert assets/debug_pass.shader.frag
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-engine-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
  )

  target_add_ldoc_test(
    TARGET ovis-engine-test
    INPUT_FILES
      src/scene.cpp
      src/scene_object.cpp
  )

  target_link_libraries(
    ovis-engine-test
    PRIVATE
      catch2
      ovis::engine
  )

  add_test(
    ovis-engine-test
    ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-engine-test.js
  )
endif ()