cmake_minimum_required(VERSION 3.12)
project(ovis)

if (${CMAKE_CXX_COMPILER} MATCHES "emscripten")
  set(OVIS_EMSCRIPTEN TRUE)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -g4 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=1 -s WASM=0 -s TOTAL_MEMORY=536870912")  
else()
  set(OVIS_EMSCRIPTEN FALSE)
endif()

set(CMAKE_CXX_STANDARD 17)

if (OVIS_EMSCRIPTEN)
  add_definitions("-DOVIS_EMSCRIPTEN=1")

  if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT DEFINED CMAKE_BUILD_TYPE)
    message(STATUS "Ovis: Building in debug mode, disable optimziations.")
    add_compile_options(
      -g
      -O0
      "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
      "SHELL:-s DEMANGLE_SUPPORT=1"
    )
    add_link_options(
      -g
      -O0
      "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
      "SHELL:-s DEMANGLE_SUPPORT=1"
    )
  else ()
    add_compile_options(
      -O3
    )
    add_link_options(
      -O3
    )
  endif ()
endif()

function(target_add_assets target)

foreach(asset ${ARGN})
  list(APPEND asset_input ${CMAKE_CURRENT_SOURCE_DIR}/${asset})

  get_filename_component(filename ${asset} NAME)
  list(APPEND asset_output ${CMAKE_CURRENT_BINARY_DIR}/assets/${filename})
endforeach()

add_custom_command(
  OUTPUT ${asset_output} ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
  COMMAND ${CMAKE_COMMAND} -E copy ${asset_input} ${CMAKE_CURRENT_BINARY_DIR}/assets
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp
  DEPENDS ${asset_input}
)

target_sources(
  ${target}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp
)

target_link_options(
  ${target}
  PUBLIC
    "SHELL:--preload-file ${CMAKE_CURRENT_BINARY_DIR}/assets/@/ovis_assets"
)

endfunction()

set(OVIS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(extern)
add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(player)