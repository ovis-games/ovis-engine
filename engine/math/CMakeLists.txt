add_library(
  ovis-math

  include/ovis/math/math_module.hpp src/math_module.cpp
  include/ovis/math/constants.hpp src/constants.cpp
  include/ovis/math/vector.hpp src/vector2.cpp src/vector3.cpp
  include/ovis/math/vector_types.hpp src/vector_types.cpp
  include/ovis/math/color.hpp src/color.cpp
  include/ovis/math/quaternion.hpp src/quaternion.cpp
  include/ovis/math/matrix.hpp src/matrix.cpp
  include/ovis/math/matrix_types.hpp src/matrix_types.cpp
  include/ovis/math/shared_operations.hpp src/shared_operations.cpp
  include/ovis/math/heightmap.hpp
  include/ovis/math/marching_cubes.hpp src/marching_cubes.cpp
  include/ovis/math/rect.hpp
  include/ovis/math/transform.hpp src/transform.cpp

  include/ovis/math/transform.hpp src/transform.cpp
)
add_library(ovis::math ALIAS ovis-math)

target_include_directories(
  ovis-math
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
  ovis-math
  PUBLIC
    ${OVIS_ROOT}/extern/glm/include
)

target_link_libraries(
  ovis-math
  PUBLIC
    ovis::core 
)

target_add_assets(
  ovis-math

  assets/math.schema.json
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-math-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
  )

  target_add_ldoc_test(
    TARGET ovis-math-test
    MODULE math
    INPUT_FILES
      src/math_module.cpp
      src/color.cpp
      src/vector2.cpp
      src/vector3.cpp
      src/transform.cpp
  )

  target_link_libraries(
    ovis-math-test
    PRIVATE
      catch2
      ovis::math
  )

  add_test(
    ovis-math-test
    ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-math-test.js
  )
endif ()
