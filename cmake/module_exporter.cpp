#include <iostream>
#include <fstream>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/${MODULE_NAME_LOWERCASE}/${MODULE_NAME_LOWERCASE}_module.hpp>

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    return -2;
  }
  if (!ovis::Load${MODULE_NAME}Module()) {
    return -1;
  }
  auto module = ovis::vm::Module::Get("${MODULE_NAME}");
  std::ofstream file(argv[1]);
  file
#ifndef NDEBUG
    << std::setw(2)
#endif
    << module->Serialize()
    << std::endl;
  return 0;
}
