add_library(
  ovis-input
  
  include/ovis/input/input.hpp src/input.cpp
  include/ovis/input/keyboard.hpp src/keyboard.cpp
  include/ovis/input/lua_module.hpp src/lua_module.cpp
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
    ovis::math
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-input-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
  )

  target_add_ldoc_test(
    TARGET ovis-input-test
    INPUT_FILES
      src/keyboard.cpp
      src/input.cpp
  )

  target_link_libraries(
    ovis-input-test
    PRIVATE
      catch2
      ovis::engine
      ovis::input
  )

  add_test(
    ovis-input-test
    ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-input-test.js
  )
endif ()