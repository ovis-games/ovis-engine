find_program(
  LDOC_EXECUTABLE
  ldoc
  DOC "LDoc executable for generating documentation"
)
if (LDOC_EXECUTABLE)
  add_custom_target(
    doc ALL
    COMMAND ${LDOC_EXECUTABLE} -d ${CMAKE_CURRENT_BINARY_DIR}/documenation -c ${CMAKE_SOURCE_DIR}/ldoc/config.ld ${CMAKE_SOURCE_DIR}/engine
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
    DEPENDS ${CMAKE_SOURCE_DIR}/ldoc/config.ld
  )
  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documenation/
    DESTINATION doc
  )
  install(
    FILES ${CMAKE_SOURCE_DIR}/ldoc/jquery-3.6.0.min.js
    DESTINATION doc
  )
endif ()