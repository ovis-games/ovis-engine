add_library(
  ovis-input
  
  include/ovis/input/input_module.hpp src/input_module.cpp
  include/ovis/input/key.hpp src/key.cpp
  include/ovis/input/key_event.hpp src/key_event.cpp
)
add_library(ovis::input ALIAS ovis-input)

target_include_directories(
  ovis-input
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-input
  PUBLIC
    ovis::core
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-input-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
  )

  target_add_ldoc_test(
    TARGET ovis-input-test
    MODULE input
    INPUT_FILES
      src/input_module.cpp
      src/key.cpp
  )

  target_link_libraries(
    ovis-input-test
    PRIVATE
      catch2
      ovis::input
  )

  add_test(
    ovis-input-test
    ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-input-test.js
  )
endif ()