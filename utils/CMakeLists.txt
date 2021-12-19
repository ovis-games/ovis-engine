add_library(
  ovis-utils
  
  include/ovis/utils/utils_module.hpp src/utils_module.cpp
  include/ovis/utils/json.hpp src/json.cpp
  include/ovis/utils/array_view.hpp
  include/ovis/utils/class.hpp
  include/ovis/utils/down_cast.hpp
  include/ovis/utils/file.hpp src/file.cpp
  include/ovis/utils/flags.hpp
  include/ovis/utils/log.hpp src/log.cpp
  include/ovis/utils/range.hpp
  include/ovis/utils/profiling.hpp src/profiling.cpp
  include/ovis/utils/serialize.hpp src/serialize.cpp
  include/ovis/utils/platform.hpp src/platform.cpp
  include/ovis/utils/small_vector.hpp src/small_vector.cpp
  include/ovis/utils/safe_pointer.hpp src/safe_pointer.cpp
  include/ovis/utils/static_factory.hpp src/static_factory.cpp
  include/ovis/utils/versioned_index.hpp src/versioned_index.cpp
  include/ovis/utils/result.hpp src/result.cpp
  include/ovis/utils/type_id.hpp src/type_id.cpp
  include/ovis/utils/parameter_pack.hpp src/parameter_pack.cpp
  include/ovis/utils/reflection.hpp src/reflection.cpp
)
add_library(ovis::utils ALIAS ovis-utils)

target_include_directories(
  ovis-utils
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-utils
  PUBLIC
    nlohmann_json
    fmt::fmt
    Lua
    sol2_single
)

if (OVIS_EMSCRIPTEN)
  target_compile_options(
    ovis-utils
    PUBLIC
      "SHELL:-s USE_SDL=2"
  )
  target_link_options(
    ovis-utils
    PUBLIC
      "--bind"
      "SHELL:-s USE_SDL=2"
  )
else ()
  if (APPLE OR WIN32)
    find_package(SDL2 REQUIRED)

    target_link_libraries(
      ovis-utils
      PUBLIC
        SDL2::SDL2-static
    )
  else ()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)

    target_include_directories(
      ovis-utils
      PUBLIC
        ${SDL2_INCLUDE_DIRS}
    )
    target_link_libraries(
      ovis-utils
      PUBLIC
        ${SDL2_LIBRARIES}
    )
  endif ()
endif ()

target_compile_features(
  ovis-utils
  PUBLIC
    cxx_std_20
)
target_compile_definitions(
  ovis-utils
  PUBLIC
    -DOVIS_ENABLE_BUILT_IN_PROFILING=0
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-utils-test

    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/sparse_vector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/string.cpp
  )

  target_link_libraries(
    ovis-utils-test
    PRIVATE
      catch2
      ovis::utils
  )

  if (OVIS_EMSCRIPTEN)
    add_test(
      ovis-utils-test
      ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-utils-test.js
    )
  else ()
    add_test(ovis-utils-test ovis-utils-test)
  endif ()
endif ()
