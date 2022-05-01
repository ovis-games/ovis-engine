#include <catch2/catch.hpp>

#include <ovis/vm/virtual_machine.hpp>
#include <ovis/vm/function.hpp>

using namespace ovis;

void foo() {}
double foo2(double) {
  return 42.0;
}

TEST_CASE("Function", "[ovis][vm][Function]") {
  VirtualMachine vm;
  auto test_module = vm.RegisterModule("Test");

  SECTION("Create native function") {
    const auto foo_function = Function::Create(FunctionDescription::CreateForNativeFunction<&foo>(&vm, "foo"));
    REQUIRE(foo_function->name() == "foo");
    REQUIRE(foo_function->inputs().size() == 0);
    REQUIRE(foo_function->outputs().size() == 0);
    const auto foo_result = foo_function->Call();
    REQUIRE(foo_result);

    const auto foo2_function = Function::Create(FunctionDescription::CreateForNativeFunction<&foo2>(&vm, "foo2"));
    REQUIRE(foo2_function->name() == "foo2");
    REQUIRE(foo2_function->inputs().size() == 1);
    REQUIRE(foo2_function->GetInput(0)->name == "0");  // Default name
    REQUIRE(foo2_function->GetInput(0)->type == vm.GetTypeId<double>());
    REQUIRE(foo2_function->GetOutput(0)->name == "0");  // Default name
    REQUIRE(foo2_function->GetOutput(0)->type == vm.GetTypeId<double>());
    const auto foo2_result = foo2_function->Call<double>(12.0);
    REQUIRE(foo2_result);
    REQUIRE(*foo2_result == 42.0);
  }

  // SECTION("Create script function") {
  //   FunctionDescription function_description {
  //     .virtual_machine = &vm,
  //     .name = "test",
  //     .inputs = { { .name = "input", .type = vm.GetTypeId<double>() } },
  //     .outputs = { { .name = "outputs", .type = vm.GetTypeId<double>() } },
  //     .definition = ScriptFunctionDefinition {
  //       .instructions = {
  //         Instruction::CreatePushTrivialConstant(0),
  //         instructions::MultiplyNumbers(-1, 0, 1),
  //         Instruction::CreateReturn(0),
  //       },
  //       .constants = {
  //         Value::Create(&vm, 2.0),
  //       },
  //     },
  //   };
  //   const auto function = Function::Create(function_description);
  // }
}
