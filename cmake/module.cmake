function(ovis_export_module_definition)
  set(options)
  set(oneValueArgs MODULE_NAME)
  set(multiValueArgs)
  cmake_parse_arguments(EXPORT_MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  set(MODULE_NAME ${EXPORT_MODULE_MODULE_NAME})

  string(TOLOWER ${MODULE_NAME} MODULE_NAME_LOWERCASE)
  configure_file(
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/module_exporter.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/module_exporter.cpp
  )
  add_executable(
    ovis-${MODULE_NAME_LOWERCASE}-exporter

    ${CMAKE_CURRENT_BINARY_DIR}/module_exporter.cpp
  )
  target_link_libraries(
    ovis-${MODULE_NAME_LOWERCASE}-exporter
    PRIVATE
      ovis::${MODULE_NAME_LOWERCASE}
  )
  if (OVIS_EMSCRIPTEN)
    add_custom_command(
      TARGET ovis-${MODULE_NAME_LOWERCASE}-exporter
      POST_BUILD
      COMMAND ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-${MODULE_NAME_LOWERCASE}-exporter.js ${MODULE_NAME_LOWERCASE}.module.json
    )
  else ()
    add_custom_command(
      TARGET ovis-${MODULE_NAME_LOWERCASE}-exporter
      POST_BUILD
      COMMAND ovis-${MODULE_NAME_LOWERCASE}-exporter ${MODULE_NAME_LOWERCASE}.module.json
    )
  endif ()
endfunction()
