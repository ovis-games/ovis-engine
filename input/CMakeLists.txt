add_library(
  ovis-input
  
  include/ovis/input/key.hpp src/key.cpp
  include/ovis/input/key_events.hpp src/key_events.cpp
  include/ovis/input/text_input_event.hpp src/text_input_event.cpp
  include/ovis/input/mouse_button.hpp src/mouse_button.cpp
  include/ovis/input/mouse_events.hpp src/mouse_events.cpp
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

# if (OVIS_BUILD_TESTS)
#   add_executable(
#     ovis-input-test


#   )

#   target_link_libraries(
#     ovis-input-test
#     PRIVATE
#       ovis::input
#       ovis::test
#   )

#   if (OVIS_EMSCRIPTEN)
#     add_test(
#       ovis-input-test
#       ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-input-test.js
#     )
#   else ()
#     add_test(ovis-input-test ovis-input-test)
#   endif ()
# endif ()
