#pragma once

#include <ovis/core/script_function.hpp>

namespace ovis {

class ScriptChunk;

class ScriptChunkArguments {
  friend class ScriptChunk;

 public:
  inline ScriptChunkArguments(const ScriptChunk& chunk);

  inline bool Add(std::string_view identifier, ScriptValue value);
  template <typename T> inline bool Add(std::string_view identifier, T&& value);

 private:
  const ScriptChunk& chunk_;
  std::vector<ScriptValue> arguments_;
};

class ScriptChunk : public ScriptFunction {
  friend class ScriptChunkArguments;

 public:
  static std::variant<ScriptChunk, ScriptError> Load(ScriptContext* context, const json& definition);

  std::vector<ScriptValueDefinition> GetVisibleLocalVariables(ScriptActionReference action);

  void Print() const;
  std::string GetInstructionsAsText() const;

 private:
  ScriptChunk(ScriptContext* context, std::vector<ScriptValueDefinition> inputs, std::vector<ScriptValueDefinition> outputs);

  // Needed for running
  ScriptContext* context_;
  std::vector<Instruction> instructions_;

  // Only necessary for parsing / debugging
  std::vector<ScriptActionReference> instruction_to_action_mappings_;
  struct LocalVariable {
    std::string name;
    ScriptActionReference declaring_action;
    ScriptTypeId type;
    int position;
  };
  std::vector<LocalVariable> local_variables_;

  struct Scope {
    std::vector<ScriptChunk::Instruction> instructions;
    std::vector<ScriptActionReference> instruction_to_action_mappings;
    std::vector<LocalVariable> local_variables_;
  };
  std::variant<Scope, ScriptError> ParseScope(const json& actions, const ScriptActionReference& parent = ScriptActionReference::Root());
  bool PushValue(const json& value, ScriptChunk::Scope* scope, int frame = 0);

  std::optional<LocalVariable> GetLocalVariable(std::string_view name);
  std::optional<ScriptError> Execute();
};

}  // namespace ovis

