add_library(
  ovis-emscripten
  
  include/ovis/emscripten/canvas.hpp src/canvas.cpp
  include/ovis/emscripten/event_callbacks.hpp src/event_callbacks.cpp
)
add_library(ovis::emscripten ALIAS ovis-emscripten)

target_include_directories(
  ovis-emscripten
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-emscripten
  PUBLIC
    ovis::rendering
    stb::truetype
)

target_add_assets(
  ovis-emscripten

  # path/to/asset
)

# if (OVIS_BUILD_TESTS)
#   add_executable(
#     ovis-emscripten-test

#     # ...
#   )
#   target_link_libraries(
#     ovis-emscripten-test
#     PRIVATE
#       ovis::emscripten
#       ovis::test
#   )
#   target_add_assets(
#     ovis-emscripten-test

#     # path/to/asset
#   )
#   target_embed_assets(ovis-emscripten-test)

#   if (OVIS_EMSCRIPTEN)
#     add_test(
#       ovis-emscripten-test
#       ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-emscripten-test.js
#     )
#   else ()
#     add_test(
#       NAME ovis-emscripten-test
#       COMMAND ovis-emscripten-test --assets-directory ${CMAKE_CURRENT_BINARY_DIR}/assets/ovis-emscripten-test/
#     )
#   endif ()
# endif ()
