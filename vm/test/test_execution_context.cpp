#include <catch2/catch.hpp>

#include "test_utils.hpp"
#include <ovis/vm/function.hpp>
#include <ovis/vm/virtual_machine.hpp>

using namespace ovis;

TEST_CASE("ExecutionContext", "[ovis][vm][ExecutionContext]") {
  VirtualMachine vm;
  ExecutionContext* execution_context = vm.main_execution_context();

  SECTION("Test instructions") {
    SECTION("HALT") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreateHalt()
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 0);
    }

    SECTION("PUSH") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePush(2)
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);

      const auto& value1 = execution_context->GetStackValue(0);
      REQUIRE(!value1.has_allocated_storage());

      const auto& value2 = execution_context->GetStackValue(1);
      REQUIRE(!value2.has_allocated_storage());
    }

    SECTION("PUSH_TRIVIAL_CONSTANT") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 42.0)
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0)
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(!value.has_allocated_storage());
      REQUIRE(value.as<double>() == 42.0);
    }

    SECTION("PUSH_ALLOCATED") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushAllocated(128, 128)
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(value.has_allocated_storage());
      REQUIRE(reinterpret_cast<std::uintptr_t>(value.allocated_storage_pointer()) % 128 == 0);
    }

    SECTION("PUSH_STACK_VALUE_DATA_ADDRESS") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 42.0)
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushStackValueDataAddress(0)
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(!value.has_allocated_storage());
      REQUIRE(value.as<double>() == 42.0);

      const auto& address = execution_context->GetStackValue(1);
      REQUIRE(!address.has_allocated_storage());
      REQUIRE(address.as<void*>() == value.data());
    }

    SECTION("PUSH_STACK_VALUE_ALLOCATED_ADDRESS") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushAllocated(32, 32),
        Instruction::CreatePushStackValueAllocatedAddress(0)
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(value.has_allocated_storage());

      const auto& address = execution_context->GetStackValue(1);
      REQUIRE(!address.has_allocated_storage());
      REQUIRE(address.as<void*>() == value.allocated_storage_pointer());
    }

    SECTION("PUSH_CONSTANT_DATA_ADDRESS") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 42.0)
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushConstantDataAddress(0)
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& address = execution_context->GetStackValue(0);
      REQUIRE(!address.has_allocated_storage());
      REQUIRE(address.as<void*>() == vm.GetConstantPointer(constant_offset)->data());
    }

    SECTION("PUSH_CONSTANT_ALLOCATED_ADDRESS") {
      struct BigType {
        double d1;
        double d2;
      };
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, BigType{ .d1 = 42.0, .d2 = 420.0 })
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushConstantAllocatedAddress(0)
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& address = execution_context->GetStackValue(0);
      REQUIRE(!address.has_allocated_storage());
      REQUIRE(address.as<void*>() == vm.GetConstantPointer(constant_offset)->allocated_storage_pointer());
    }

    SECTION("POP") {
      // TODO: test for proper destruction
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePush(5),
        Instruction::CreatePop(2),
        Instruction::CreatePop(1),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);
    }

    SECTION("POP_TRIVIAL") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePush(5),
        Instruction::CreatePopTrivial(1),
        Instruction::CreatePopTrivial(2),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);
    }

    SECTION("ASSIGN_TRIVIAL") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 42.0),
        Value::Create(&vm, 420.0),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateAssignTrivial(0),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& address = execution_context->GetStackValue(0);
      REQUIRE(!address.has_allocated_storage());
      REQUIRE(address.as<double>() == 420.0);
    }

    SECTION("COPY_TRIVIAL") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 42.0),
        Value::Create(&vm, 420.0),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateCopyTrivial(0, 1),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(!value.has_allocated_storage());
      REQUIRE(value.as<double>() == 420.0);

      const auto& value2 = execution_context->GetStackValue(1);
      REQUIRE(!value2.has_allocated_storage());
      REQUIRE(value2.as<double>() == 420.0);
    }

    SECTION("MEMORY_COPY") {
      struct BigType {
        double d1;
        double d2;
      };
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, BigType{ .d1 = 42.0, .d2 = 420.0 })
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushAllocated(alignof(BigType), sizeof(BigType)),
        Instruction::CreatePushStackValueAllocatedAddress(0),
        Instruction::CreatePushConstantAllocatedAddress(0),
        Instruction::CreateMemoryCopy(sizeof(BigType)),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(value.has_allocated_storage());
      REQUIRE(std::memcmp(&value.as<BigType>(), &vm.GetConstantPointer(constant_offset)->as<BigType>(),
                          sizeof(BigType)) == 0);
    }

    SECTION("OFFSET_ADDRESS") {
      struct TwoInts {
        std::int32_t i1;
        std::int32_t i2;
      };
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, TwoInts{ .i1 = 42, .i2 = 420 })
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushStackValueDataAddress(0),
        Instruction::CreateOffsetAddress(1, 4),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(!value.has_allocated_storage());
      const auto& address = execution_context->GetStackValue(1);
      REQUIRE(!address.has_allocated_storage());

      REQUIRE(&value.as<TwoInts>().i2 == address.as<void*>());
    }

    SECTION("CALL_NATIVE_FUNCTION") {
    }
    SECTION("PUSH_EXECUTION_STATE") {
    }
    SECTION("CALL_SCRIPT_FUNCTION") {
    }
    SECTION("SET_CONSTANT_BASE_OFFSET") {
    }
    SECTION("RETURN") {
    }
    SECTION("NOT") {
    }
    SECTION("AND") {
    }
    SECTION("OR") {
    }
    SECTION("ADD_NUMBERS") {
    }
    SECTION("SUBTRACT_NUMBERS") {
    }
    SECTION("MULTIPLY_NUMBERS") {
    }
    SECTION("DIVIDE_NUMBERS") {
    }
    SECTION("IS_NUMBER_GREATER") {
    }
    SECTION("IS_NUMBER_LESS") {
    }
    SECTION("IS_NUMBER_GREATER_EQUAL") {
    }
    SECTION("IS_NUMBER_LESS_EQUAL") {
    }
    SECTION("IS_NUMBER_EQUAL") {
    }
    SECTION("IS_NUMBER_NOT_EQUAL") {
    }
    SECTION("JUMP") {
    }
    SECTION("JUMP_IF_TRUE") {
    }
    SECTION("JUMP_IF_FALSE") {
    }
  }

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

  // SECTION("Call function with simple return") {
  //   FunctionDescription function_description {
  //     .virtual_machine = &vm,
  //     .name = "test",
  //     .inputs = {},
  //     .outputs = { { .name = "something", .type = vm.GetTypeId<double>() } },
  //     .definition = ScriptFunctionDefinition {
  //       .instructions = {
  //         Instruction::CreatePushTrivialConstant(0),
  //         Instruction::CreateCopyTrivial(
  //           ExecutionContext::GetFunctionBaseOffset(1, 0),
  //           ExecutionContext::GetOutputOffset(0)
  //         ),
  //         Instruction::CreateReturn(1),
  //       },
  //       .constants = {
  //         Value::Create(&vm, 42.0),
  //       },
  //     },
  //   };
  //   const auto function = Function::Create(function_description);
  //   REQUIRE(function);
  //   const auto function_result = function->Call<double>();
  //   REQUIRE_RESULT(function_result);
  //   REQUIRE(*function_result == 42.0);
  // }

  // SECTION("Call function with inputs") {
  //   FunctionDescription function_description {
  //     .virtual_machine = &vm,
  //     .name = "test",
  //     .inputs = { { .name = "something", .type = vm.GetTypeId<double>() } },
  //     .outputs = { { .name = "something", .type = vm.GetTypeId<double>() } },
  //     .definition = ScriptFunctionDefinition {
  //       .instructions = {
  //         Instruction::CreatePushTrivialConstant(0),
  //         Instruction::CreateMultiplyNumbers(
  //           ExecutionContext::GetInputOffset(1, 0),
  //           ExecutionContext::GetFunctionBaseOffset(1, 1)
  //         ),
  //         Instruction::CreateAssignTrivial(ExecutionContext::GetOutputOffset(0)),
  //         Instruction::CreateReturn(1),
  //       },
  //       .constants = {
  //         Value::Create(&vm, 2.0),
  //       },
  //     },
  //   };
  //   const auto function = Function::Create(function_description);
  //   REQUIRE(function);
  //   const auto function_result = function->Call<double>(21.0);
  //   REQUIRE_RESULT(function_result);
  //   REQUIRE(*function_result == 42.0);
  // }
}
