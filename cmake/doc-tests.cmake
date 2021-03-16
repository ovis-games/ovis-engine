find_program(
  LDOC_EXECUTABLE
  ldoc
  DOC "LDoc executable for generating documentation"
)

function(target_add_ldoc_test)
  set(options)
  set(oneValueArgs TARGET MODULE)
  set(multiValueArgs INPUT_FILES)
  cmake_parse_arguments(TARGET_ADD_LDOC_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  string(SUBSTRING ${TARGET_ADD_LDOC_TEST_MODULE} 0 1 FIRST_LETTER_OF_MODULE)
  string(TOUPPER ${FIRST_LETTER_OF_MODULE} FIRST_LETTER_OF_MODULE)
  string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER_OF_MODULE}\\1" MODULE_CAPITALIZED "${TARGET_ADD_LDOC_TEST_MODULE}")

  configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/doc-test.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/doc-test-template/ldoc.ltp
    @ONLY
  )

  if (LDOC_EXECUTABLE)
    foreach(FILE ${TARGET_ADD_LDOC_TEST_INPUT_FILES})
      get_filename_component(FILENAME ${FILE} NAME_WE)
      get_filename_component(EXTENSION ${FILE} EXT)

      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.generated${EXTENSION}
        COMMAND
          ${LDOC_EXECUTABLE}
          -d ${CMAKE_CURRENT_BINARY_DIR} # output directory
          -o ${FILENAME}.generated # output name
          -l ${CMAKE_CURRENT_BINARY_DIR}/doc-test-template/
          -c ${CMAKE_SOURCE_DIR}/ldoc-test-generation/config.ld
          ${FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS
          ${FILE}
          ${CMAKE_CURRENT_BINARY_DIR}/doc-test-template/ldoc.ltp
      )

      target_sources(
        ${TARGET_ADD_LDOC_TEST_TARGET}
        PRIVATE
          ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.generated${EXTENSION}
      )
    endforeach()
  else ()
    message(WARNING "Failed to generate lua tests: ldoc executable could not be found.")
  endif ()
endfunction()
