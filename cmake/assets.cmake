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

  # TODO: make it configurable whether to use --embed-file or --preload-file
  target_link_options(
    ${target}
    PUBLIC
      "SHELL:--embed-file ${CMAKE_CURRENT_BINARY_DIR}/assets/@/ovis_assets"
  )

endfunction()