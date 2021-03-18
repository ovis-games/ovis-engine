add_library(
  ovis-core
  
  include/ovis/core/core_module.hpp src/core_module.cpp
  include/ovis/core/scene.hpp src/scene.cpp
  include/ovis/core/scene_controller.hpp src/scene_controller.cpp
  include/ovis/core/script_scene_controller.hpp src/script_scene_controller.cpp
  include/ovis/core/scene_object.hpp src/scene_object.cpp
  include/ovis/core/scene_object_component.hpp src/scene_object_component.cpp
  include/ovis/core/transform.hpp src/transform.cpp
  include/ovis/core/transform_controller.hpp src/transform_controller.cpp
  include/ovis/core/viewport.hpp src/viewport.cpp
  include/ovis/core/render_target_viewport.hpp src/render_target_viewport.cpp
  include/ovis/core/render_pass.hpp src/render_pass.cpp
  include/ovis/core/debug_render_pass.hpp src/debug_render_pass.cpp
  include/ovis/core/camera.hpp src/camera.cpp
)
add_library(ovis::core ALIAS ovis-core)

target_include_directories(
  ovis-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-core
  PUBLIC
    ovis::core
    ovis::graphics
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-core-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
  )

  target_add_ldoc_test(
    TARGET ovis-core-test
    MODULE core
    INPUT_FILES
      src/scene.cpp
      src/scene_object.cpp
      src/transform.cpp
  )

  target_link_libraries(
    ovis-core-test
    PRIVATE
      catch2
      ovis::core
  )

  add_test(
    ovis-core-test
    ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-core-test.js
  )
endif ()