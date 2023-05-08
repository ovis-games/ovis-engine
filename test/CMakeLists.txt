if (OVIS_BUILD_TESTS)

add_library(
  ovis-test
  src/main.cpp
)
target_include_directories(
  ovis-test
  PUBLIC
    include
)
target_link_libraries(
  ovis-test
  PUBLIC
    ovis::core
    Catch2::Catch2
)
add_library(ovis::test ALIAS ovis-test)

endif (OVIS_BUILD_TESTS)
