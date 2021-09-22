add_executable(
  ovis-script-compiler
  src/main.cpp
)

target_include_directories(
  ovis-script-compiler
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (OVIS_EMSCRIPTEN)
  target_compile_options(
    ovis-script-compiler
    PUBLIC
    "SHELL:-s FETCH=1"
    "SHELL:-s --bind"
    "SHELL:-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0"
  )
  target_link_options(
    ovis-script-compiler
    PUBLIC
      "SHELL:-s FETCH=1"
      "SHELL:-s --bind"
      "SHELL:-s FORCE_FILESYSTEM=1"
      "SHELL:-s MODULARIZE=1"
      "SHELL:-s ALLOW_MEMORY_GROWTH=1"
      "SHELL:-s EXPORT_NAME=createScriptCompilerModule"
      "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=['FS','ccall','cwrap']"
      "SHELL:-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0"
  )
endif (OVIS_EMSCRIPTEN)

target_link_libraries(
  ovis-script-compiler
  PUBLIC
    ovis::application
)

target_include_assets(ovis-script-compiler)

install(
  TARGETS ovis-script-compiler
  RUNTIME DESTINATION .
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/ovis-script-compiler.data ${CMAKE_CURRENT_BINARY_DIR}/ovis-script-compiler.wasm
  DESTINATION .
)
