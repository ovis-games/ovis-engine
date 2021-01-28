
add_library(
  ovis-core
  
  include/ovis/core/json.hpp src/json.cpp
  include/ovis/core/asset_library.hpp src/asset_library.cpp
  include/ovis/core/array_view.hpp
  include/ovis/core/class.hpp
  include/ovis/core/down_cast.hpp
  include/ovis/core/file.hpp src/file.cpp
  include/ovis/core/flags.hpp
  include/ovis/core/log.hpp src/log.cpp
  include/ovis/core/range.hpp
  include/ovis/core/resource_manager.hpp src/resource_manager.cpp
  include/ovis/core/resource.hpp
  include/ovis/core/profiling.hpp src/profiling.cpp
  include/ovis/core/serialize.hpp src/serialize.cpp
)
add_library(ovis::core ALIAS ovis-core)

target_include_directories(
  ovis-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-core
  PUBLIC
    nlohmann_json
    fmt::fmt
)

if (OVIS_EMSCRIPTEN)
  target_compile_options(
    ovis-core
    PUBLIC
      "SHELL:-s USE_SDL=2"
  )
  target_link_options(
    ovis-core
    PUBLIC
      "SHELL:-s USE_SDL=2"
  )
else ()
  find_package(SDL2 REQUIRED)
  target_link_libraries(
    ovis-core
    PUBLIC
      SDL2::SDL2-static
  )
endif ()

target_compile_features(
  ovis-core
  PUBLIC
    cxx_std_20
)
target_compile_definitions(
  ovis-core
  PUBLIC
    -DOVIS_ENABLE_BUILT_IN_PROFILING=0
)