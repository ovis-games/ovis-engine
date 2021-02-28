add_library(
  ovis-math

  include/ovis/math/heightmap.hpp
  include/ovis/math/marching_cubes.hpp src/marching_cubes.cpp
  include/ovis/math/plane.hpp
  include/ovis/math/rect.hpp
  include/ovis/math/transform.hpp src/transform.cpp
  include/ovis/math/triangle.hpp
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
