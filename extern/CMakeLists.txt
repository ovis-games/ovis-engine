add_subdirectory(microtar)

if (OVIS_BUILD_BENCHMARKS)
  SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Don't include testing for google benchmark (requires google test)")
  SET(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Do not install benchmark library")
  add_subdirectory(benchmark)
endif ()
