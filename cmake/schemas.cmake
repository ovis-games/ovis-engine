function(target_add_schemas target)
  foreach(schema_file ${ARGN})
    target_add_schema(${target} ${schema_file})
  endforeach()
endfunction()

function(target_add_schema target schema_filename)
  if (NOT ${schema_filename} MATCHES "\\.schema\\.json$")
    message(SEND_ERROR "Invalid schema file ${schema_filename}")
    return()
  endif ()
  string(REGEX REPLACE "\\.schema\\.json$" "" base_filename ${schema_filename})
  
  get_filename_component(schema_source_dir ${schema_filename} DIRECTORY) 
  set(output_directory ${CMAKE_CURRENT_BINARY_DIR}/include/${schema_source_dir})
  set(json_file_path ${output_directory}/json.hpp)
  if (NOT EXISTS ${json_file_path})
    file(WRITE ${json_file_path} "#include \"nlohmann/json.hpp\"")
  endif ()
  
  set(output_filename ${CMAKE_CURRENT_BINARY_DIR}/include/${base_filename}.hpp)
  set(depfile ${CMAKE_CURRENT_BINARY_DIR}/${base_filename}.dep)
  add_custom_command(
    OUTPUT ${output_filename}
    BYPRODUCTS ${depfile}
    COMMAND
      ${CMAKE_COMMAND}
      -P
      ${CMAKE_SOURCE_DIR}/cmake/compile_schema.cmake
      ${QUICKTYPE_EXECUTABLE}
      ${schema_filename}
      ${output_filename}
      ${depfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    MAIN_DEPENDENCY ${schema_filename}
    DEPENDS ${CMAKE_SOURCE_DIR}/cmake/compile_schema.cmake
    DEPFILE ${depfile}
  )

  target_sources(
    ${target}
    PRIVATE
      ${output_filename}
  )
endfunction()
