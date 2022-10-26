add_library(
  ovis-sdl
  
  include/ovis/sdl/sdl_window.hpp src/sdl_window.cpp
  include/ovis/sdl/sdl_event_processor.hpp src/sdl_event_processor.cpp
  include/ovis/sdl/sdl_init_subsystem.hpp src/sdl_init_subsystem.cpp
)
add_library(ovis::sdl ALIAS ovis-sdl)

target_include_directories(
  ovis-sdl
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-sdl
  PUBLIC
    ovis::input
    ovis::graphics
    stb::truetype
)

target_add_assets(
  ovis-sdl

  # path/to/asset
)

if (OVIS_EMSCRIPTEN)
  target_compile_options(
    ovis-sdl
    PUBLIC
      "SHELL:-s USE_SDL=2"
  )
  target_link_options(
    ovis-sdl
    PUBLIC
      "--bind"
      "SHELL:-s USE_SDL=2"
  )
else ()
  target_link_libraries(
    ovis-sdl
    PUBLIC
      SDL2::SDL2-static
  )
endif ()

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-sdl-test

    test/test_window.cpp
  )
  target_link_libraries(
    ovis-sdl-test
    PRIVATE
      ovis::sdl
      ovis::test
  )
  target_add_assets(
    ovis-sdl-test

    # path/to/asset
  )
  target_embed_assets(ovis-sdl-test)

  if (OVIS_EMSCRIPTEN)
    add_test(
      ovis-sdl-test
      ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-sdl-test.js
    )
  else ()
    add_test(
      NAME ovis-sdl-test
      COMMAND ovis-sdl-test --assets-directory ${CMAKE_CURRENT_BINARY_DIR}/assets/ovis-sdl-test/
    )
  endif ()
endif ()
