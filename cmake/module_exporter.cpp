#include <iostream>
#include <iomanip>
#include <fstream>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/${MODULE_NAME_LOWERCASE}/${MODULE_NAME_LOWERCASE}_module.hpp>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#endif

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    return -2;
  }
  if (!ovis::Load${MODULE_NAME}Module()) {
    return -1;
  }
  auto module = ovis::vm::Module::Get("${MODULE_NAME}");
#if OVIS_EMSCRIPTEN
  EM_ASM(
    FS.mkdir('/cwd');
    FS.mount(NODEFS, { root: '.' }, '/cwd');
  );
  std::ofstream file(fmt::format("/cwd/{}", argv[1]));
#else
  std::ofstream file(argv[1]);
#endif
  if (!file.is_open()) {
    return -3;
  }
  file
#ifndef NDEBUG
    << std::setw(2)
#endif
    << module->Serialize()
    << std::endl;
  return 0;
}
