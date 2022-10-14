#include "test_utils.hpp"

#include <catch2/catch.hpp>

#include "ovis/vm/function.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/virtual_machine.hpp"

using namespace ovis;

TEST_CASE("ExecutionContext", "[ovis][vm][ExecutionContext]") {
  VirtualMachine vm;
  ExecutionContext* execution_context = vm.main_execution_context();

  SECTION("Test instructions") {
    struct BigType {
      double d1;
      double d2;
    };

    SECTION("HALT") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 0);
    }

    SECTION("PUSH") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePush(2),
        Instruction::CreateHalt(),
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
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(!value.has_allocated_storage());
      REQUIRE(value.as<double>() == 42.0);
    }

    SECTION("PUSH_TRIVIAL_STACK_VALUE") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 42.0)
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialStackValue(0),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(!value.has_allocated_storage());
      REQUIRE(value.as<double>() == 42.0);
      REQUIRE(execution_context->GetStackValue<double>(1) == 42.0);
    }

    SECTION("PUSH_ALLOCATED") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushAllocated(128, 128),
        Instruction::CreateHalt(),
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
        Instruction::CreatePushStackValueDataAddress(0),
        Instruction::CreateHalt(),
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
        Instruction::CreatePushStackValueAllocatedAddress(0),
        Instruction::CreateHalt(),
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
        Instruction::CreatePushConstantDataAddress(0),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& address = execution_context->GetStackValue(0);
      REQUIRE(!address.has_allocated_storage());
      REQUIRE(address.as<void*>() == vm.GetConstantPointer(constant_offset)->data());
    }

    SECTION("PUSH_CONSTANT_ALLOCATED_ADDRESS") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, BigType{ .d1 = 42.0, .d2 = 420.0 })
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushConstantAllocatedAddress(0),
        Instruction::CreateHalt(),
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
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);
    }

    SECTION("POP_TRIVIAL") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePush(5),
        Instruction::CreatePopTrivial(1),
        Instruction::CreatePopTrivial(2),
        Instruction::CreateHalt(),
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
        Instruction::CreateHalt(),
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
        Instruction::CreateHalt(),
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
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, BigType{ .d1 = 42.0, .d2 = 420.0 })
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushAllocated(alignof(BigType), sizeof(BigType)),
        Instruction::CreatePushStackValueAllocatedAddress(0),
        Instruction::CreatePushConstantAllocatedAddress(0),
        Instruction::CreateMemoryCopy(sizeof(BigType)),
        Instruction::CreateHalt(),
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
        Instruction::CreateHalt(),
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
      struct Foo {
        static double Add(double first, double second) {
          return first + second;
        }
      };

      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, &NativeFunctionWrapper<&Foo::Add>),
        Value::Create(&vm, 20.0),
        Value::Create(&vm, 22.0),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(2),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateCallNativeFunction(2),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);

      const auto& value = execution_context->GetStackValue(0);
      REQUIRE(!value.has_allocated_storage());
      REQUIRE(value.as<double>() == 42.0);
    }

    SECTION("PREPARE_SCRIPT_FUNCTION_CALL") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePrepareScriptFunctionCall(3),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 3);

      // Return address is not set yet
      const auto& return_address = execution_context->GetStackValue(0);
      REQUIRE(!return_address.has_allocated_storage());

      const auto& constant_offset = execution_context->GetStackValue(1);
      REQUIRE(!constant_offset.has_allocated_storage());
      REQUIRE(constant_offset.as<std::uint32_t>() == 0);

      const auto& stack_offset = execution_context->GetStackValue(2);
      REQUIRE(!stack_offset.has_allocated_storage());
      REQUIRE(stack_offset.as<std::uint32_t>() == 0);
    }

    SECTION("CALL_SCRIPT_FUNCTION") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create<std::uint32_t>(&vm, 0),
        Value::Create<std::uint32_t>(&vm, 10),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePush(2), // Just some values to test the stack offset value
        Instruction::CreatePush(1), // Output
        Instruction::CreatePush(1), // Return address
        Instruction::CreatePushTrivialConstant(0), // constant offset
        Instruction::CreatePushTrivialConstant(0), // stack offset
        Instruction::CreatePush(2), // Inputs
        Instruction::CreatePushTrivialConstant(1), // function address
        Instruction::CreateScriptFunctionCall(1, 2),
        Instruction::CreatePush(100), // should not go here
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 8);

      REQUIRE(execution_context->stack_offset() == 2);
      REQUIRE(execution_context->GetStackValue(execution_context->stack_offset() + 1).as<std::uint32_t>() == 9);
    }

    SECTION("SET_CONSTANT_BASE_OFFSET") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 0.0),
        Value::Create(&vm, 1.0),
        Value::Create(&vm, BigType{ .d1 = 12.0, .d2 = 134.0 }),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreateSetConstantBaseOffset(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushConstantDataAddress(0),
        Instruction::CreatePushConstantAllocatedAddress(1),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 3);
      REQUIRE(execution_context->GetStackValue<double>(0) == 1.0);
      REQUIRE(execution_context->GetStackValue<void*>(1) == vm.GetConstantPointer(1)->data());
      REQUIRE(*execution_context->GetStackValue<double*>(1) == 1.0);
      REQUIRE(execution_context->GetStackValue<void*>(2) == vm.GetConstantPointer(2)->allocated_storage_pointer());
      REQUIRE(execution_context->GetStackValue<BigType*>(2)->d1 == 12.0);
      REQUIRE(execution_context->GetStackValue<BigType*>(2)->d2 == 134.0);
    }

    SECTION("RETURN") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 11), // Return adress
        Value::Create(&vm, 42.0), // Interesting value
        Value::Create(&vm, 34.0), // output value
        Value::Create<std::uint32_t>(&vm, 5), // constant offset
        Value::Create<std::uint32_t>(&vm, 0), // stack offset
        Value::Create(&vm, 1337.0),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(2),  // Output
        Instruction::CreatePushTrivialConstant(0),  // Return address
        Instruction::CreatePushTrivialConstant(3),  // constant offset
        Instruction::CreatePushTrivialConstant(4),  // stack offset
        Instruction::CreatePush(100),               // Inputs and other values inside the "function"
        Instruction::CreateReturn(1),
        Instruction::CreatePush(100),               // should never be executed
        Instruction::CreatePush(100),               // should never be executed
        Instruction::CreatePush(100),               // should never be executed
        Instruction::CreatePush(100),               // should never be executed
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 2);
      REQUIRE(execution_context->constant_offset() == 5);
      REQUIRE(execution_context->stack_offset() == 0);
      // REQUIRE(execution_context->GetStackValue<double>(100) == 42.0);
      REQUIRE(execution_context->GetStackValue<double>(0) == 34.0);
      REQUIRE(execution_context->GetStackValue<double>(1) == 1337.0);
    }

    SECTION("NOT") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, true),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateNot(0),
        Instruction::CreateNot(1),
        Instruction::CreateHalt(),
      }));
      REQUIRE(execute_result);
      REQUIRE(execution_context->stack_size() == 3);
      REQUIRE(execution_context->GetStackValue<bool>(0) == true);
      REQUIRE(execution_context->GetStackValue<bool>(1) == false);
      REQUIRE(execution_context->GetStackValue<bool>(2) == true);
    }

    SECTION("AND") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, true),
        Value::Create(&vm, false),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateAnd(0, 0),
        Instruction::CreateAnd(0, 1),
        Instruction::CreateAnd(1, 0),
        Instruction::CreateAnd(1, 1),
        Instruction::CreateHalt(),
      }));
      REQUIRE(execute_result);
      REQUIRE(execution_context->stack_size() == 6);
      REQUIRE(execution_context->GetStackValue<bool>(0) == true);
      REQUIRE(execution_context->GetStackValue<bool>(1) == false);
      REQUIRE(execution_context->GetStackValue<bool>(2) == true);
      REQUIRE(execution_context->GetStackValue<bool>(3) == false);
      REQUIRE(execution_context->GetStackValue<bool>(4) == false);
      REQUIRE(execution_context->GetStackValue<bool>(5) == false);
    }

    SECTION("OR") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, true),
        Value::Create(&vm, false),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateOr(0, 0),
        Instruction::CreateOr(0, 1),
        Instruction::CreateOr(1, 0),
        Instruction::CreateOr(1, 1),
        Instruction::CreateHalt(),
      }));
      REQUIRE(execute_result);
      REQUIRE(execution_context->stack_size() == 6);
      REQUIRE(execution_context->GetStackValue<bool>(0) == true);
      REQUIRE(execution_context->GetStackValue<bool>(1) == false);
      REQUIRE(execution_context->GetStackValue<bool>(2) == true);
      REQUIRE(execution_context->GetStackValue<bool>(3) == true);
      REQUIRE(execution_context->GetStackValue<bool>(4) == true);
      REQUIRE(execution_context->GetStackValue<bool>(5) == false);
    }

    SECTION("(ADD/SUBTRACT/MULTIPLY/DIVIDE)_NUMBERS") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 2.0),
        Value::Create(&vm, 21.0),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateAddNumbers(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateSubtractNumbers(),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateMultiplyNumbers(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateDivideNumbers(),
        Instruction::CreateHalt(),
      }));
      REQUIRE(execute_result);
      REQUIRE(execution_context->stack_size() == 4);
      REQUIRE(execution_context->GetStackValue<double>(0) == 23.0);
      REQUIRE(execution_context->GetStackValue<double>(1) == 19.0);
      REQUIRE(execution_context->GetStackValue<double>(2) == 42.0);
      REQUIRE(execution_context->GetStackValue<double>(3) == 10.5);
    }

    SECTION("IS_NUMBER_(GREATER/LESS/GREATER_EQUAL/LESS_EQUAL/EQUAL/NOT_EQUAL)") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, 2.0),
        Value::Create(&vm, 21.0),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateIsNumberGreater(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateIsNumberGreater(),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateIsNumberLess(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateIsNumberLess(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateIsNumberGreaterEqual(),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateIsNumberGreaterEqual(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateIsNumberLessEqual(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateIsNumberLessEqual(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateIsNumberEqual(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateIsNumberEqual(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateIsNumberNotEqual(),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateIsNumberNotEqual(),
        Instruction::CreateHalt(),
      }));
      REQUIRE(execute_result);
      REQUIRE(execution_context->stack_size() == 12);

      REQUIRE(execution_context->GetStackValue<bool>(0) == true);
      REQUIRE(execution_context->GetStackValue<bool>(1) == false);
      REQUIRE(execution_context->GetStackValue<bool>(2) == true);
      REQUIRE(execution_context->GetStackValue<bool>(3) == false);
      REQUIRE(execution_context->GetStackValue<bool>(4) == true);
      REQUIRE(execution_context->GetStackValue<bool>(5) == false);
      REQUIRE(execution_context->GetStackValue<bool>(6) == true);
      REQUIRE(execution_context->GetStackValue<bool>(7) == false);
      REQUIRE(execution_context->GetStackValue<bool>(8) == true);
      REQUIRE(execution_context->GetStackValue<bool>(9) == false);
      REQUIRE(execution_context->GetStackValue<bool>(10) == true);
      REQUIRE(execution_context->GetStackValue<bool>(11) == false);
    }

    SECTION("JUMP") {
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreateJump(3),
        Instruction::CreateJump(3),
        Instruction::CreatePush(100),
        Instruction::CreateJump(-2),
        Instruction::CreateHalt(),
      }));
      REQUIRE(execute_result);
      REQUIRE(execution_context->stack_size() == 0);
    }

    SECTION("JUMP_IF_TRUE") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, true),
        Value::Create(&vm, false),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateJumpIfTrue(2),
        Instruction::CreatePush(100),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateJumpIfTrue(2),
        Instruction::CreatePush(1),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 1);
    }

    SECTION("JUMP_IF_FALSE") {
      const auto constant_offset = vm.InsertConstants(std::array{
        Value::Create(&vm, true),
        Value::Create(&vm, false),
      });
      const auto execute_result = execution_context->Execute(vm.InsertInstructions(std::array{
        Instruction::CreatePushTrivialConstant(0),
        Instruction::CreateJumpIfFalse(2),
        Instruction::CreatePush(100),
        Instruction::CreatePushTrivialConstant(1),
        Instruction::CreateJumpIfFalse(2),
        Instruction::CreatePush(1),
        Instruction::CreateHalt(),
      }));
      REQUIRE_RESULT(execute_result);
      REQUIRE(execution_context->stack_size() == 100);
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
    REQUIRE_RESULT(execution_context->Call(function->handle()));
  }

  SECTION("Call function with simple return") {
    FunctionDescription function_description {
      .virtual_machine = &vm,
      .name = "test",
      .inputs = {},
      .outputs = { { .name = "something", .type = vm.GetTypeId<double>() } },
      .definition = ScriptFunctionDefinition {
        .instructions = {
          Instruction::CreatePushTrivialConstant(0),
          Instruction::CreateCopyTrivial(
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
    const auto function_result = execution_context->Call<double>(function->handle());
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
          Instruction::CreatePushTrivialConstant(0),
          Instruction::CreatePushTrivialStackValue(ExecutionContext::GetInputOffset(1, 0)),
          Instruction::CreateMultiplyNumbers(),
          Instruction::CreateAssignTrivial(ExecutionContext::GetOutputOffset(0)),
          Instruction::CreateReturn(1),
        },
        .constants = {
          Value::Create(&vm, 2.0),
        },
      },
    };
    const auto function = Function::Create(function_description);
    REQUIRE(function);
    const auto function_result = execution_context->Call<double>(function->handle(), 21.0);
    REQUIRE_RESULT(function_result);
    REQUIRE(*function_result == 42.0);
  }
}
