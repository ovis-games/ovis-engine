find_program(
  LDOC_EXECUTABLE
  ldoc
  DOC "LDoc executable for generating documentation"
)

function(target_add_ldoc_test)
  set(options)
  set(oneValueArgs TARGET)
  set(multiValueArgs INPUT_FILES)
  cmake_parse_arguments(TARGET_ADD_LDOC_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  message(STATUS "target_add_ldoc_test")

  if (LDOC_EXECUTABLE)
    foreach(FILE ${TARGET_ADD_LDOC_TEST_INPUT_FILES})
      get_filename_component(FILENAME ${FILE} NAME_WE)
      get_filename_component(EXTENSION ${FILE} EXT)

      message(STATUS "${FILE} -> ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.generated${EXTENSION}")

      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}.generated${EXTENSION}
        COMMAND
          ${LDOC_EXECUTABLE}
          -d ${CMAKE_CURRENT_BINARY_DIR} # output directory
          -o ${FILENAME}.generated # output name
          -c ${CMAKE_SOURCE_DIR}/ldoc-test-generation/config.ld
          ${FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS
          ${FILE}
          ${CMAKE_SOURCE_DIR}/ldoc-test-generation/ldoc.ltp
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
