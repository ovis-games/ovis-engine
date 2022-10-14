#include <catch2/catch.hpp>

#include "ovis/vm/function.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/virtual_machine.hpp"

using namespace ovis;

void foo() {}
double foo2(double) {
  return 42.0;
}

TEST_CASE("Function", "[ovis][vm][Function]") {
  VirtualMachine vm;
  auto test_module = vm.RegisterModule("Test");

  SECTION("Create native function") {
    const auto foo_function = vm.CreateFunction<&foo>("foo");
    REQUIRE(foo_function.function()->name() == "foo");
    REQUIRE(!foo_function.function()->is_script_function());
    REQUIRE(foo_function.function()->is_native_function());
    REQUIRE(foo_function.function()->inputs().size() == 0);
    REQUIRE(foo_function.function()->outputs().size() == 0);
    const auto foo_result = foo_function();
    REQUIRE(foo_result);

    const auto foo2_function = vm.CreateFunction<&foo2>("foo2"); 
    REQUIRE(foo2_function.function()->name() == "foo2");
    REQUIRE(!foo2_function.function()->is_script_function());
    REQUIRE(foo2_function.function()->is_native_function());
    REQUIRE(foo2_function.function()->inputs().size() == 1);
    REQUIRE(foo2_function.function()->GetInput(0)->name == "0");  // Default name
    REQUIRE(foo2_function.function()->GetInput(0)->type == vm.GetTypeId<double>());
    REQUIRE(foo2_function.function()->GetOutput(0)->name == "0");  // Default name
    REQUIRE(foo2_function.function()->GetOutput(0)->type == vm.GetTypeId<double>());
    const auto foo2_result = foo2_function(12.0);
    REQUIRE(foo2_result);
    REQUIRE(*foo2_result == 42.0);
  }

  SECTION("Create script function") {
    FunctionDescription function_description {
      .virtual_machine = &vm,
      .name = "test",
      .inputs = { { .name = "input", .type = vm.GetTypeId<double>() } },
      .outputs = { { .name = "outputs", .type = vm.GetTypeId<double>() } },
      .definition = ScriptFunctionDefinition {
        .instructions = {
          Instruction::CreatePushTrivialConstant(0),
          Instruction::CreateReturn(0),
        },
        .constants = {
          Value::Create(&vm, 2.0),
        },
      },
    };
    const auto function = Function::Create(function_description);
    REQUIRE(function);
    REQUIRE(function->is_script_function());
    REQUIRE(!function->is_native_function());
  }
}
