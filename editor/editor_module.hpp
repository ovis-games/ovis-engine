#pragma once

#include <ovis/engine/module.hpp>

namespace ovis {
namespace editor {

class EditorModule : public Module {
 public:
  EditorModule();

  inline std::vector<std::string>* log_history() { return &log_history_; }

 private:
  std::vector<std::string> log_history_;
};

}  // namespace editor
}  // namespace ovis