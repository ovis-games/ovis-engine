if (${CMAKE_CXX_COMPILER} MATCHES "emscripten")
  set(OVIS_EMSCRIPTEN TRUE)
else()
  set(OVIS_EMSCRIPTEN FALSE)
endif()

if (OVIS_EMSCRIPTEN)
  add_definitions("-DOVIS_EMSCRIPTEN=1")

  if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT DEFINED CMAKE_BUILD_TYPE)
    message(STATUS "Ovis: Building in debug mode, disable optimziations.")
    add_compile_options(
      -g
      -O0
      "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    )
    add_link_options(
      -g
      -O0
      "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    )
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
      -O3
    )
    add_link_options(
      -O3
    )
  endif ()
endif()
