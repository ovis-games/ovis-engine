#pragma once

#include <optional>
#include <vector>

#include <ovis/utils/reflection.hpp>
#include <ovis/core/asset_library.hpp>

namespace ovis {

class ScriptFunction {
  public:
    struct DebugInfo {};
    void Call(std::span<const Value> inputs, std::span<Value> outputs);

  private:

  std::vector<Instruction> instructions_;
};

ScriptFunction LoadScriptFunction(std::string_view asset_id, std::string_view asset_file);
ScriptFunction LoadScriptFunction(AssetLibrary* asset_library, std::string_view asset_id, std::string_view asset_file);

}  // namespace ovis

