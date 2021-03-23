add_library(
  ovis-networking
  
  include/ovis/networking/networking_module.hpp src/networking_module.cpp
  include/ovis/networking/fetch.hpp src/fetch.cpp
)
add_library(ovis::networking ALIAS ovis-networking)

target_include_directories(
  ovis-networking
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  ovis-networking
  PUBLIC
    ovis::utils
)