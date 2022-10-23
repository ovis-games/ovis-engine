add_library(
  ovis-application

  include/ovis/application/application.hpp src/application.cpp
  include/ovis/application/game_settings.hpp src/game_settings.cpp
  include/ovis/application/sdl_window.hpp src/sdl_window.cpp
  include/ovis/application/canvas_viewport.hpp src/canvas_viewport.cpp
)
add_library(ovis::application ALIAS ovis-application)

target_include_directories(
  ovis-application
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-application
  PUBLIC
    ovis::core
    ovis::rendering
    ovis::rendering2d
    # ovis::physics2d
    ovis::input
    ovis::networking
    # ovis::imgui
)

# target_add_assets(
#   ovis-application

#   assets/application.schema.json
# )

# if (OVIS_BUILD_TESTS)
#   add_executable(
#     ovis-application-test

#     ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
#   )

#   target_add_ldoc_test(
#     TARGET ovis-application-test
#     INPUT_FILES
#       src/lua_module.cpp
#       src/color.cpp
#       src/vector2.cpp
#       src/vector3.cpp
#   )

#   target_link_libraries(
#     ovis-application-test
#     PRIVATE
#       catch2
#       ovis::application
#       ovis::engine
#   )

#   add_test(
#     ovis-application-test
#     ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-application-test.js
#   )
# endif ()
