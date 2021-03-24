add_library(
  ovis-input
  
  include/ovis/input/input_module.hpp src/input_module.cpp
  include/ovis/input/key.hpp src/key.cpp
  include/ovis/input/key_events.hpp src/key_press_event.cpp src/key_release_event.cpp
  include/ovis/input/text_input_event.hpp src/text_input_event.cpp
  include/ovis/input/mouse_button.hpp src/mouse_button.cpp
  include/ovis/input/mouse_events.hpp src/mouse_button_press_event.cpp src/mouse_button_release_event.cpp src/mouse_move_event.cpp src/mouse_wheel_event.cpp
  include/ovis/input/emscripten_callbacks.hpp src/emscripten_callbacks.cpp
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
      src/key_press_event.cpp
      src/key_release_event.cpp
      src/mouse_button.cpp
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