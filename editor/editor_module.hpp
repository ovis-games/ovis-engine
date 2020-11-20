#pragma once

#include <ovis/engine/module.hpp>

namespace ove {

class EditorModule : public ovis::Module {
 public:
  EditorModule();

 private:
  std::vector<std::string> log_history_;
};

}  // namespace ove