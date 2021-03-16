add_library(
  ovis-rendering
  
  include/ovis/rendering/rendering_module.hpp src/rendering_module.cpp
)
add_library(ovis::rendering ALIAS ovis-rendering)

target_include_directories(
  ovis-rendering
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-rendering
  PUBLIC
    ovis::core
    ovis::math
)

# if (OVIS_BUILD_TESTS)
#   add_executable(
#     ovis-rendering-test

#     ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
#   )

#   target_add_ldoc_test(
#     TARGET ovis-rendering-test
#     MODULE rendering
#     INPUT_FILES
#       src/rendering.cpp
#       src/rendering_object.cpp
#   )

#   target_link_libraries(
#     ovis-rendering-test
#     PRIVATE
#       catch2
#       ovis::rendering
#   )

#   add_test(
#     ovis-rendering-test
#     ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-rendering-test.js
#   )
# endif ()