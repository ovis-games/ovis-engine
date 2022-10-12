add_library(
  ovis-core
  
  include/ovis/core/vm_bindings.hpp src/vm_bindings.cpp
  include/ovis/core/main_vm.hpp src/main_vm.cpp
  include/ovis/core/event.hpp src/event.cpp
  include/ovis/core/json_schema.hpp src/json_schema.cpp
  include/ovis/core/scene.hpp src/scene.cpp
  include/ovis/core/component_storage.hpp src/component_storage.cpp
  include/ovis/core/scene_viewport.hpp src/scene_viewport.cpp
  include/ovis/core/scene_controller.hpp src/scene_controller.cpp
  include/ovis/core/scene_object.hpp src/scene_object.cpp
  include/ovis/core/scene_object_animation.hpp src/scene_object_animation.cpp
  include/ovis/core/animator.hpp src/animator.cpp
  include/ovis/core/animator_controller.hpp src/animator_controller.cpp
  include/ovis/core/camera.hpp src/camera.cpp
  include/ovis/core/transform.hpp src/transform.cpp
  include/ovis/core/asset_library.hpp src/asset_library.cpp
  include/ovis/core/lua.hpp src/lua.cpp
  include/ovis/core/vector_types.hpp src/vector_types.cpp
  include/ovis/core/vector.hpp src/vector2.cpp src/vector3.cpp
  include/ovis/core/color_type.hpp src/color_type.cpp
  include/ovis/core/color.hpp src/color.cpp
  include/ovis/core/matrix_types.hpp src/matrix_types.cpp
  include/ovis/core/matrix.hpp src/matrix.cpp
  include/ovis/core/math_operations.hpp src/math_operations.cpp
  include/ovis/core/intersection.hpp src/intersection.cpp
  include/ovis/core/math.hpp src/math.cpp
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
    ovis::utils
    ovis::vm
    middleclass
)

target_add_assets(
  ovis-core

  assets/core.script_documentation.en-US.json
  assets/vector2.script_documentation.en-US.json
)

ovis_export_module_definition(
  MODULE_NAME Core
)

install(
  DIRECTORY schemas/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/schemas
  FILES_MATCHING PATTERN "*.schema.json"
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-core-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_intersection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_scene_object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_simple_scene_controller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_scripting.cpp
  )

  target_compile_definitions(
    ovis-core-test
    PRIVATE
      -DCATCH_CONFIG_ENABLE_BENCHMARKING=1
  )

  target_add_ldoc_test(
    TARGET ovis-core-test
    MODULE core
    INPUT_FILES
      src/core_module.cpp
      src/scene.cpp
      src/scene_object.cpp
      src/scene_controller.cpp
      src/scene_object_component.cpp
      src/transform.cpp
      src/camera.cpp
      src/vector2.cpp
      src/vector3.cpp
      src/matrix.cpp
      src/color.cpp
  )

  target_link_libraries(
    ovis-core-test
    PRIVATE
      Catch2
      ovis::core
  )
  target_add_assets(
    ovis-core-test

    test/assets/template.scene_object.json
    test/assets/recursive_template.scene_object.json
    test/assets/child_recursion_template.scene_object.json
    test/assets/indirect_recursion1.scene_object.json
    test/assets/indirect_recursion2.scene_object.json
    test/assets/animation_test.scene_object.json
    test/assets/test_script_type.script_type.json
    test/assets/test_script.script.json
  )
  target_embed_assets(ovis-core-test)

  if (OVIS_EMSCRIPTEN)
    add_test(
      ovis-core-test
      ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-core-test.js
    )
  else ()
    add_test(
      NAME ovis-core-test
      COMMAND ovis-core-test --assets-directory ${CMAKE_CURRENT_BINARY_DIR}/assets/ovis-core-test/
    )
  endif ()
endif ()

if (OVIS_BUILD_BENCHMARKS)
  add_executable(
    ovis-core-benchmark

    ${CMAKE_CURRENT_SOURCE_DIR}/benchmark/bench_scripting.cpp
  )

  target_link_libraries(
    ovis-core-benchmark
    PRIVATE
      benchmark::benchmark
      ovis::core
      Lua
      sol2_single
  )
  target_embed_assets(ovis-core-benchmark)
endif ()
