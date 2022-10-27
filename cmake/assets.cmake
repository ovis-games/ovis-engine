define_property(
  TARGET
  PROPERTY OVIS_ASSETS
  BRIEF_DOCS "Asset files of the target"
  FULL_DOCS "Asset files of the target"
)

function(target_add_assets target)
  foreach(asset ${ARGN})
    list(APPEND asset_input ${CMAKE_CURRENT_SOURCE_DIR}/${asset})
  endforeach()

  set_target_properties(
    ${target}
    PROPERTIES
      OVIS_ASSETS "${asset_input}"
  )
endfunction()

function (extract_assets asset_list target)

  set(inherited_assets "")
  get_target_property(dependencies ${target} LINK_LIBRARIES)
  if (dependencies)
    foreach (dependency ${dependencies})
      if (TARGET ${dependency})
        get_target_property(dependency_type ${dependency} TYPE)
        if (NOT ${dependency_type} STREQUAL "INTERFACE_LIBRARY")
          extract_assets(dependency_assets ${dependency})
          list(APPEND inherited_assets ${dependency_assets})
        endif ()
      endif ()
    endforeach ()
  endif ()

  get_target_property(assets ${target} OVIS_ASSETS)
  if (assets)
    set(${asset_list} ${inherited_assets} ${assets} PARENT_SCOPE)
  else ()
    set(${asset_list} ${inherited_assets} PARENT_SCOPE)
  endif ()
endfunction()

function (target_include_assets target)
  extract_assets(asset_files ${target})
  list(REMOVE_DUPLICATES asset_files)

  if (OVIS_EMSCRIPTEN)
    foreach(asset ${asset_files})
      target_link_options(
        ${target}
        PUBLIC
          "SHELL:--preload-file ${asset}@/ovis_assets/"
      )
    endforeach()

    target_sources(
      ${target}
      PRIVATE
        ${asset_files}
    )
  else ()
    set(base_dir ${CMAKE_CURRENT_BINARY_DIR}/assets/${target})
    foreach(asset ${asset_files})
      get_filename_component(filename ${asset} NAME)
      list(APPEND asset_output ${base_dir}/${filename})
    endforeach()

    add_custom_command(
      OUTPUT ${asset_output}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${base_dir}
      COMMAND ${CMAKE_COMMAND} -E copy ${asset_files} ${base_dir}
      DEPENDS ${asset_files}
    )

    set(assets_dummy_cpp ${CMAKE_CURRENT_BINARY_DIR}/${target}_assets.cpp)
    add_custom_command(
      OUTPUT ${assets_dummy_cpp}
      COMMAND ${CMAKE_COMMAND} -E touch ${assets_dummy_cpp}
      DEPENDS ${asset_files}
    )

    target_sources(
      ${target}
      PRIVATE
        ${assets_dummy_cpp}
    )
  endif ()
endfunction()

function (target_embed_assets target)
  extract_assets(asset_files ${target})
  list(REMOVE_DUPLICATES asset_files)

  if (OVIS_EMSCRIPTEN)
    foreach(asset ${asset_files})
      target_link_options(
        ${target}
        PUBLIC
          "SHELL:--embed-file ${asset}@/ovis_assets/"
      )
      # target_compile_options(
      #   ${target}
      #   PUBLIC
      #     "SHELL:--embed-file ${asset}@/ovis_assets/"
      # )
    endforeach()

    set(assets_dummy_cpp ${CMAKE_CURRENT_BINARY_DIR}/${target}_assets.cpp)
    add_custom_command(
      OUTPUT ${assets_dummy_cpp}
      COMMAND ${CMAKE_COMMAND} -E touch ${assets_dummy_cpp}
      DEPENDS ${asset_files}
    )

    target_sources(
      ${target}
      PRIVATE
        ${assets_dummy_cpp}
    )
  else ()
    set(base_dir ${CMAKE_CURRENT_BINARY_DIR}/assets/${target})
    foreach(asset ${asset_files})
      get_filename_component(filename ${asset} NAME)
      list(APPEND asset_output ${base_dir}/${filename})
    endforeach()

    add_custom_command(
      OUTPUT ${asset_output}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${base_dir}
      COMMAND ${CMAKE_COMMAND} -E copy ${asset_files} ${base_dir}
      DEPENDS ${asset_files}
    )

    target_sources(
      ${target}
      PRIVATE
        ${asset_output}
    )
  endif ()
endfunction()
