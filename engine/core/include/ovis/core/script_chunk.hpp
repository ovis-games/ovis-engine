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
  enum class InstructionType : uint8_t {
    PUSH_STACK_FRAME,
    POP_STACK_FRAME,
    FUNCTION_CALL,
    PUSH_CONSTANT,
    PUSH_STACK_VARIABLE,
    ASSIGN_CONSTANT,
    ASSIGN_STACK_VARIABLE,
    POP,
    JUMP,
    JUMP_IF_TRUE,
    JUMP_IF_FALSE,
  };
  struct FunctionCall {
    uint8_t input_count;
    uint8_t output_count;
    ScriptFunctionPointer function;
  };
  struct PushConstant {
    ScriptValue value;
  };
  struct PushStackValue {
    int position;
    int frame;
  };
  struct AssignConstant {
    ScriptValue value;
    int position;
  };
  struct AssignStackVariable {
    int16_t source_position;
    int16_t source_frame;
    int16_t destination_position;
    int16_t destination_frame;
  };
  struct Pop {
    int count;
  };
  struct Jump {
    int instruction_offset;
  };
  struct Instruction {
    InstructionType type;
    std::variant<std::monostate, FunctionCall, PushConstant, PushStackValue, Pop, AssignConstant, AssignStackVariable, Jump> data;
  };


  static std::variant<ScriptChunk, ScriptError> Load(ScriptContext* context, const json& definition);

  std::variant<ScriptError, std::vector<ScriptValue>> Call(const ScriptChunkArguments& arguments);
  std::variant<ScriptError, std::vector<ScriptValue>> Call(std::span<const ScriptValue> arguments);

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

  std::optional<LocalVariable> GetLocalVariable(std::string_view name);
  std::optional<ScriptError> Execute();
};

}  // namespace ovis

