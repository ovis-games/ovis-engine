#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include <ovis/vm/function.hpp>
#include <ovis/vm/virtual_machine.hpp>

using namespace ovis;

TEST_CASE("ExecutionContext", "[ovis][vm][ExecutionContext]") {
  VirtualMachine vm;
  ExecutionContext* main_execution_context = vm.main_execution_context();

  SECTION("Call empty script function") {
    FunctionDescription function_description {
      .virtual_machine = &vm,
      .name = "test",
      .inputs = {},
      .outputs = {},
      .definition = ScriptFunctionDefinition {
        .instructions = {
          Instruction::CreateReturn(0),
        },
        .constants = {},
      },
    };
    const auto function = Function::Create(function_description);
    REQUIRE(function);
    REQUIRE_RESULT(function->Call());
  }

  SECTION("Call function with simple return") {
    FunctionDescription function_description {
      .virtual_machine = &vm,
      .name = "test",
      .inputs = {},
      .outputs = { { .name = "something", .type = vm.GetTypeId<double>() } },
      .definition = ScriptFunctionDefinition {
        .instructions = {
          instructions::PushTrivialConstant(0),
          instructions::CopyTrivialValue(
            ExecutionContext::GetOutputOffset(0),
            ExecutionContext::GetFunctionBaseOffset(1, 0)
          ),
          Instruction::CreateReturn(1),
        },
        .constants = {
          Value::Create(&vm, 42.0),
        },
      },
    };
    const auto function = Function::Create(function_description);
    REQUIRE(function);
    const auto function_result = function->Call<double>();
    REQUIRE_RESULT(function_result);
    REQUIRE(*function_result == 42.0);
  }

  SECTION("Call function with inputs") {
    FunctionDescription function_description {
      .virtual_machine = &vm,
      .name = "test",
      .inputs = { { .name = "something", .type = vm.GetTypeId<double>() } },
      .outputs = { { .name = "something", .type = vm.GetTypeId<double>() } },
      .definition = ScriptFunctionDefinition {
        .instructions = {
          instructions::PushTrivialConstant(0),
          instructions::MultiplyNumbers(
            ExecutionContext::GetOutputOffset(0),
            ExecutionContext::GetInputOffset(1, 0),
            ExecutionContext::GetFunctionBaseOffset(1, 1)
          ),
          Instruction::CreateReturn(1),
        },
        .constants = {
          Value::Create(&vm, 2.0),
        },
      },
    };
    const auto function = Function::Create(function_description);
    REQUIRE(function);
    const auto function_result = function->Call<double>(21.0);
    REQUIRE_RESULT(function_result);
    REQUIRE(*function_result == 42.0);
  }
}
