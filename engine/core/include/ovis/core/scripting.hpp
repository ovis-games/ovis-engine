#pragma once

#include <any>
#include <functional>
#include <map>

namespace ovis
{

struct ScriptVariable {
  std::string type;
  std::any value;
};

using ScriptFunctionParameters = std::map<std::string, ScriptVariable>;

// using ScriptFunction = std::function<ScriptVariable()>;

class ScriptContext {
public:

  void RegisterFunction();

private:
};

} // namespace ovis
