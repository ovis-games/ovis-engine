add_library(
  ovis-scene
  
  include/ovis/scene/scene_module.hpp src/scene_module.cpp
  include/ovis/scene/scene.hpp src/scene.cpp
  include/ovis/scene/scene_controller.hpp src/scene_controller.cpp
  include/ovis/scene/script_scene_controller.hpp src/script_scene_controller.cpp
  include/ovis/scene/scene_object.hpp src/scene_object.cpp
  include/ovis/scene/scene_object_component.hpp src/scene_object_component.cpp
  include/ovis/scene/transform_component.hpp src/transform_component.cpp
)
add_library(ovis::scene ALIAS ovis-scene)

target_include_directories(
  ovis-scene
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-scene
  PUBLIC
    ovis::core
    ovis::math
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-scene-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
  )

  target_add_ldoc_test(
    TARGET ovis-scene-test
    MODULE scene
    INPUT_FILES
      src/scene.cpp
      src/scene_object.cpp
      src/transform_component.cpp
  )

  target_link_libraries(
    ovis-scene-test
    PRIVATE
      catch2
      ovis::scene
  )

  add_test(
    ovis-scene-test
    ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-scene-test.js
  )
endif ()