add_library(
  ovis-vm
  
  include/ovis/vm/virtual_machine.hpp src/virtual_machine.cpp
  include/ovis/vm/virtual_machine_instructions.hpp src/virtual_machine_instructions.cpp
  include/ovis/vm/execution_context.hpp src/execution_context.cpp
  include/ovis/vm/type.hpp src/type.cpp
  include/ovis/vm/type_memory_layout.hpp src/type_memory_layout.cpp
  include/ovis/vm/function.hpp src/function.cpp
  include/ovis/vm/contiguous_storage.hpp src/contiguous_storage.cpp
  include/ovis/vm/list.hpp src/list.cpp
  include/ovis/vm/value_storage.hpp src/value_storage.cpp
  include/ovis/vm/value.hpp src/value.cpp
  include/ovis/vm/parse_script_error.hpp src/parse_script_error.cpp
  include/ovis/vm/script_function_parser.hpp src/script_function_parser.cpp
  include/ovis/vm/script_type_parser.hpp src/script_type_parser.cpp
  include/ovis/vm/script_parser.hpp src/script_parser.cpp
)
add_library(ovis::vm ALIAS ovis-vm)

target_include_directories(
  ovis-vm
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(
  ovis-vm
  PUBLIC
    ovis::utils
)

target_add_schemas(
  ovis-vm

  schemas/function.schema.json
  schemas/type.schema.json
)

install(
  DIRECTORY schemas/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/schemas
  FILES_MATCHING PATTERN "*.schema.json"
)

if (OVIS_BUILD_TESTS)
  add_executable(
    ovis-vm-test

    test/test_virtual_machine.cpp
    test/test_value_storage.cpp
    test/test_value.cpp
    test/test_type.cpp
    test/test_function.cpp
    test/test_execution_context.cpp
    test/test_script_type_parsing.cpp
    test/test_script_function_parser.cpp
    test/test_script_parser.cpp
    test/test_list.cpp
  )

  target_link_libraries(
    ovis-vm-test
    PRIVATE
      ovis::vm
      ovis::test
  )
  target_add_assets(
    ovis-vm-test

    test/assets/test_script_type.script_type.json
  )
  target_embed_assets(ovis-vm-test)

  if (OVIS_EMSCRIPTEN)
    add_test(
      ovis-vm-test
      ${NODE_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/ovis-vm-test.js
    )
  else ()
    add_test(
      NAME ovis-vm-test
      COMMAND ovis-vm-test
    )
  endif ()
endif ()
