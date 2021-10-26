add_library(
  ovis-core
  
  include/ovis/core/core_module.hpp src/core_module.cpp
  include/ovis/core/event.hpp src/event.cpp
  include/ovis/core/json_schema.hpp src/json_schema.cpp
  include/ovis/core/scene.hpp src/scene.cpp
  include/ovis/core/scene_viewport.hpp src/scene_viewport.cpp
  include/ovis/core/scene_controller.hpp src/scene_controller.cpp
  include/ovis/core/scene_object.hpp src/scene_object.cpp
  include/ovis/core/scene_object_component.hpp src/scene_object_component.cpp
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
  include/ovis/core/script_function.hpp src/script_function.cpp
  include/ovis/core/script_parser.hpp src/script_parser.cpp
  include/ovis/core/virtual_machine.hpp src/virtual_machine.cpp
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
    middleclass
)

target_add_assets(
  ovis-core

  assets/core.script_documentation.en-US.json
  assets/vector2.script_documentation.en-US.json
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-core-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_intersection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_scene_object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_scripting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/virtual_machine.cpp
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
      catch2
      ovis::core
  )
  target_embed_assets(ovis-core-test)

  if (OVIS_EMSCRIPTEN)
    add_test(
      ovis-core-test
      ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-core-test.js
    )
  else ()
    add_test(ovis-core-test ovis-core-test)
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
