add_library(
  ovis-application

  include/ovis/application/constants.hpp src/constants.cpp
  include/ovis/application/vector.hpp src/vector2.cpp src/vector3.cpp
  include/ovis/application/vector_types.hpp src/vector_types.cpp
  include/ovis/application/color.hpp src/color.cpp
  include/ovis/application/quaternion.hpp src/quaternion.cpp
  include/ovis/application/matrix.hpp src/matrix.cpp
  include/ovis/application/matrix_types.hpp src/matrix_types.cpp
  include/ovis/application/shared_operations.hpp src/shared_operations.cpp
  include/ovis/application/heightmap.hpp
  include/ovis/application/marching_cubes.hpp src/marching_cubes.cpp
  include/ovis/application/rect.hpp
  include/ovis/application/transform.hpp src/transform.cpp
  include/ovis/application/lua_module.hpp src/lua_module.cpp

  include/ovis/application/transform.hpp src/transform.cpp
)
add_library(ovis::application ALIAS ovis-application)

target_include_directories(
  ovis-application
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(
  ovis-application
  PUBLIC
    ${OVIS_ROOT}/extern/glm/include
)

target_link_libraries(
  ovis-application
  PUBLIC
    ovis::core 
)

target_add_assets(
  ovis-application

  assets/application.schema.json
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-application-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
  )

  target_add_ldoc_test(
    TARGET ovis-application-test
    INPUT_FILES
      src/lua_module.cpp
      src/color.cpp
      src/vector2.cpp
      src/vector3.cpp
  )

  target_link_libraries(
    ovis-application-test
    PRIVATE
      catch2
      ovis::application
      ovis::engine
  )

  add_test(
    ovis-application-test
    ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-application-test.js
  )
endif ()
