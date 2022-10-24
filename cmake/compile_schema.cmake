cmake_minimum_required(VERSION 3.9)

set(quicktype_executable ${CMAKE_ARGV3})
set(schema_filename ${CMAKE_ARGV4})
set(output_filename ${CMAKE_ARGV5})
set(dep_filename ${CMAKE_ARGV6})

execute_process(
  COMMAND
    ${quicktype_executable}
    --src-lang schema
    --lang cpp 
    --namespace ovis::schemas
    --code-format with-struct
    --type-style pascal-case
    --member-style underscore-case
    --enumerator-style upper-underscore-case
    --no-boost
    --hide-null-optional
    --debug print-schema-resolving
    -o ${output_filename}
    ${schema_filename}
  OUTPUT_VARIABLE output
)

set(dependencies)
string(REPLACE "\n" ";" outputs ${output})
foreach(output_line ${outputs})
  if (${output_line} MATCHES "^successully fetched (.*)$")
    list(APPEND dependencies ${CMAKE_SOURCE_DIR}/${CMAKE_MATCH_1})
  endif ()
endforeach()

string(JOIN " " depfile_content ${dependencies})
file(WRITE ${dep_filename} "${output_filename}: ${depfile_content}")
