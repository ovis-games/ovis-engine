add_library(
  ovis-math

  include/ovis/math/constants.hpp src/constants.cpp
  include/ovis/math/vector.hpp src/vector.cpp
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
