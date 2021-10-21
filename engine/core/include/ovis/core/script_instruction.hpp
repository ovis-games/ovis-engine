#pragma once

#include <variant>

#include <ovis/utils/reflection.hpp>

namespace ovis {

namespace script_instructions {

struct FunctionCall {
  Function::Pointer function_pointer;
  int input_count;
  int output_count;
};

struct PushConstant {
  Value value;
};

struct PushStackValue {
  int position;
  int frame;
};

struct AssignConstant {
  Value value;
  int position;
};

struct AssignStackValue {
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

struct JumpIfTrue {
  int instruction_offset;
};

struct JumpIfFalse {
  int instruction_offset;
};

}

using ScriptInstruction = std::variant<
  script_instructions::FunctionCall,
  script_instructions::PushConstant,
  script_instructions::PushStackValue,
  script_instructions::AssignConstant,
  script_instructions::AssignStackValue,
  script_instructions::Pop,
  script_instructions::Jump,
  script_instructions::JumpIfTrue,
  script_instructions::JumpIfFalse
>;

}

namespace fmt {

template <>
struct fmt::formatter<ovis::script_instructions::FunctionCall> {
  constexpr auto parse(format_parse_context& ctx) {
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && *it != '}') throw format_error("invalid format");
    return it;
  }

  template <typename FormatContext>
  auto format(const ovis::script_instructions::FunctionCall& function_call, FormatContext& ctx) {
    return format_to(ctx.out(), "function_call");
  }
};

}

