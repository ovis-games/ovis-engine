#pragma once

#include <ovis/core/module.hpp>

inline std::shared_ptr<ovis::Module> RegisterTestModule() {
  if (ovis::Module::Get("Test") != nullptr) {
    ovis::Module::Deregister("Test");
  }
  return ovis::Module::Register("Test");
}
