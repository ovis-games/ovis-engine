#include <catch2/catch.hpp>

#include <ovis/core/virtual_machine.hpp>

using namespace ovis;
using namespace ovis::vm;

std::shared_ptr<ovis::vm::Module> RegisterTestModule() {
  if (ovis::vm::Module::Get("Test") != nullptr) {
    ovis::vm::Module::Deregister("Test");
  }
  return ovis::vm::Module::Register("Test");
}

TEST_CASE("Value", "[ovis][core][vm]") {
  ValueStorage value(8.0);
  REQUIRE(value.as<double>() == 8.0);
  REQUIRE(!value.allocated_storage());
  value.reset(4.0f);
  REQUIRE(value.as<float>() == 4.0);
  REQUIRE(!value.allocated_storage());

  struct Foo {
    uint64_t a = 100;
    uint64_t b = 123;
  };
  value.reset(Foo());
  REQUIRE(value.allocated_storage());
  REQUIRE(value.as<Foo>().a == 100);
  REQUIRE(value.as<Foo>().b == 123);

  value.reset('c');
  REQUIRE(!value.allocated_storage());
  REQUIRE(value.as<char>() == 'c');
}

namespace {

Result<> AddNumbers(ExecutionContext* context) {
  const double number1 = context->registers()[0].as<double>();
  const double number2 = context->registers()[1].as<double>();
  context->PopValues(2);
  context->PushValue(number1 + number2);
  return Success;
}

}

#define REQUIRE_RESULT(expr) \
  do { \
    auto&& require_result = expr; \
    if (!require_result) { \
      UNSCOPED_INFO(require_result.error().message); \
    } \
    REQUIRE(require_result); \
  } while (false)

TEST_CASE("Exit", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<Instruction, 1> instructions = {
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, {}));
  REQUIRE(context.registers().size() == 0);
}

TEST_CASE("Push", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<Instruction, 3> instructions = {
    instructions::Push(2),
    instructions::Push(1),
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, {}));
  REQUIRE(context.registers().size() == 3);
}

TEST_CASE("Pop", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<Instruction, 5> instructions = {
    instructions::Push(2),
    instructions::Push(1),
    instructions::Pop(2),
    instructions::Pop(1),
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, {}));
  REQUIRE(context.registers().size() == 0);
}

TEST_CASE("Load constant", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 1> constants = {
    8.0
  };
  REQUIRE(constants[0].as<double>() == 8.0);

  std::array<Instruction, 5> instructions = {
    instructions::PushTrivialConstant(0),
    instructions::Exit(),
  };
  REQUIRE(instructions[0].opcode == std::uint32_t(OpCode::PUSH_TRIVIAL_CONSTANT));
  REQUIRE(instructions[0].push_trivial_constant.constant == 0);

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 1);
  REQUIRE(context.registers()[0].as<double>() == 8.0);
}

TEST_CASE("Copy trivial value", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 1> constants = {
    8.0,
  };
  REQUIRE(constants[0].as<double>() == 8.0);

  std::array<Instruction, 4> instructions = {
    instructions::PushTrivialConstant(0),
    instructions::Push(1),
    instructions::CopyTrivialValue(1, 0),
    instructions::Exit(),
  };
  REQUIRE(instructions[0].opcode == std::uint32_t(OpCode::PUSH_TRIVIAL_CONSTANT));
  REQUIRE(instructions[0].push_trivial_constant.constant == 0);

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 2);
  REQUIRE(context.registers()[0].as<double>() == 8.0);
  REQUIRE(context.registers()[1].as<double>() == 8.0);
}

TEST_CASE("Native function call", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 2> constants = {
    8.0,
    &AddNumbers,
  };
  REQUIRE(constants[0].as<double>() == 8.0);

  std::array<Instruction, 5> instructions = {
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(1),
    instructions::CallNativeFunction(2),
    instructions::Exit(),
  };
  REQUIRE(instructions[0].opcode == std::uint32_t(OpCode::PUSH_TRIVIAL_CONSTANT));
  REQUIRE(instructions[0].push_trivial_constant.constant == 0);

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 1);
  REQUIRE(context.registers()[0].as<double>() == 16.0);
}

TEST_CASE("Jump", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 5> constants = {
    1.0,
    2.0,
  };
  REQUIRE(constants[0].as<double>() == 1.0);
  REQUIRE(constants[1].as<double>() == 2.0);
  for (const auto& c : constants) {
    REQUIRE(!c.allocated_storage());
  }

  std::array<Instruction, 4> instructions = {
    instructions::Jump(2),
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(1),
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 1);
  REQUIRE(context.registers()[0].as<double>() == 2.0);
}

TEST_CASE("Jump if true", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 5> constants = {
    1.0,
    2.0,
    3.0,
    true,
    false,
  };
  REQUIRE(constants[0].as<double>() == 1.0);
  REQUIRE(constants[1].as<double>() == 2.0);
  REQUIRE(constants[2].as<double>() == 3.0);
  REQUIRE(constants[3].as<bool>() == true);
  REQUIRE(constants[4].as<bool>() == false);
  for (const auto& c : constants) {
    REQUIRE(!c.allocated_storage());
  }

  std::array<Instruction, 8> instructions = {
    instructions::PushTrivialConstant(3),
    instructions::JumpIfTrue(2),
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(4),
    instructions::JumpIfTrue(2),
    instructions::PushTrivialConstant(1),
    instructions::PushTrivialConstant(2),
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 2);
  REQUIRE(context.registers()[0].as<double>() == 2.0);
  REQUIRE(context.registers()[1].as<double>() == 3.0);
}

TEST_CASE("Jump if false", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 5> constants = {
    1.0,
    2.0,
    3.0,
    true,
    false,
  };
  REQUIRE(constants[0].as<double>() == 1.0);
  REQUIRE(constants[1].as<double>() == 2.0);
  REQUIRE(constants[2].as<double>() == 3.0);
  REQUIRE(constants[3].as<bool>() == true);
  REQUIRE(constants[4].as<bool>() == false);
  for (const auto& c : constants) {
    REQUIRE(!c.allocated_storage());
  }

  std::array<Instruction, 8> instructions = {
    instructions::PushTrivialConstant(4),
    instructions::JumpIfFalse(2),
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(3),
    instructions::JumpIfFalse(2),
    instructions::PushTrivialConstant(1),
    instructions::PushTrivialConstant(2),
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 2);
  REQUIRE(context.registers()[0].as<double>() == 2.0);
  REQUIRE(context.registers()[1].as<double>() == 3.0);
}

TEST_CASE("Subtract numbers", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 2> constants = {
    2.0,
    3.0,
  };

  const auto instructions = std::array {
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(1),
    instructions::Push(2),
    instructions::SubtractNumbers(2, 0, 1),
    instructions::SubtractNumbers(3, 1, 0),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 4);
  REQUIRE(context.registers()[0].as<double>() == 2.0);
  REQUIRE(context.registers()[1].as<double>() == 3.0);
  REQUIRE(context.registers()[2].as<double>() == -1.0);
  REQUIRE(context.registers()[3].as<double>() == 1.0);
}

TEST_CASE("Multiply numbers", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 2> constants = {
    2.0,
    3.0,
  };

  const auto instructions = std::array {
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(1),
    instructions::Push(1),
    instructions::MultiplyNumbers(2, 0, 1),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 3);
  REQUIRE(context.registers()[0].as<double>() == 2.0);
  REQUIRE(context.registers()[1].as<double>() == 3.0);
  REQUIRE(context.registers()[2].as<double>() == 6.0);
}

TEST_CASE("Is number greater", "[ovis][core][vm]") {
  ExecutionContext context;

  std::array<ValueStorage, 2> constants = {
    2.0,
    3.0,
  };

  const auto instructions = std::array {
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(1),
    instructions::Push(2),
    instructions::IsNumberGreater(2, 0, 1),
    instructions::IsNumberGreater(3, 1, 0),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 4);
  REQUIRE(context.registers()[0].as<double>() == 2.0);
  REQUIRE(context.registers()[1].as<double>() == 3.0);
  REQUIRE(context.registers()[2].as<bool>() == false);
  REQUIRE(context.registers()[3].as<bool>() == true);
}

namespace {

bool IsGreater(double number1, double number2) {
  return number1 > number2;
}

double Add(double number1, double number2) {
  return number1 + number2;
}

double Subtract(double number1, double number2) {
  return number1 - number2;
}

double Multiply(double number1, double number2) {
  return number1 * number2;
}

}

TEST_CASE("Native function wrapper", "[ovis][core][vm]") {
  ExecutionContext context;
  std::array<ValueStorage, 2> constants = {
    8.0,
    &NativeFunctionWrapper<Add>,
  };
  REQUIRE(constants[0].as<double>() == 8.0);

  std::array<Instruction, 5> instructions = {
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(0),
    instructions::PushTrivialConstant(1),
    instructions::CallNativeFunction(2),
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));
  REQUIRE(context.registers().size() == 1);
  REQUIRE(context.registers()[0].as<double>() == 16.0);
}

namespace {

double CalculateFactorial(double number) {
  ExecutionContext context;
  std::array<ValueStorage, 100> constants = {
    number,
    1.0,
    &NativeFunctionWrapper<Subtract>,
    &NativeFunctionWrapper<Multiply>,
    &NativeFunctionWrapper<IsGreater>,
  };

  // r[0] = result
  // r[1] = input
  std::array<Instruction, 24> instructions = {
    instructions::PushTrivialConstant(1), // [result]
    instructions::PushTrivialConstant(0), // [result, input]
    instructions::Push(1),                // [result, input, void]
    instructions::CopyTrivialValue(2, 1), // [result, input, input]
    instructions::PushTrivialConstant(1), // [result, input, input, 1.0]
    instructions::PushTrivialConstant(4), // [result, input, input, 1.0, &IsGreater]
    instructions::CallNativeFunction(2),  // [result, input, IsGreater_result]
    instructions::JumpIfFalse(16),        // [result, input]
    instructions::Push(2),                // [result, input, void, void]
    instructions::CopyTrivialValue(2, 0), // [result, input, result, void]
    instructions::CopyTrivialValue(3, 1), // [result, input, result, input]
    instructions::PushTrivialConstant(3), // [result, input, result, input, &Multiply]
    instructions::CallNativeFunction(2),  // [result, input, Multiply_result]
    instructions::CopyTrivialValue(0, 2), // [result, input, Multiply_result]
    instructions::Pop(1),                 // [result, input]
    instructions::Push(1),                // [result, input, void]
    instructions::CopyTrivialValue(2, 1), // [result, input, input]
    instructions::PushTrivialConstant(1), // [result, input, input, 1.0]
    instructions::PushTrivialConstant(2), // [result, input, input, 1.0, &Subtract]
    instructions::CallNativeFunction(2),  // [result, input, Subtract_result]
    instructions::CopyTrivialValue(1, 2), // [result, input, Subtract_result]
    instructions::Pop(1),                 // [result, input]
    instructions::Jump(-20),              // [result, input]
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));

  return context.registers()[0].as<double>();
}

double CalculateFactorial2(double number) {
  ExecutionContext context;
  std::array<ValueStorage, 2> constants = {
    number,
    1.0,
  };

  // r[0] = result
  // r[1] = input
  std::array<Instruction, 13> instructions = {
    instructions::PushTrivialConstant(1),   // [result]
    instructions::PushTrivialConstant(0),   // [result, input]
    instructions::PushTrivialConstant(1),   // [result, input, 1]
    instructions::Push(1),                  // [result, input, 1, IsGreater_result]
    instructions::IsNumberGreater(3, 1, 2), // [result, input, 1, IsGreater_result]
    instructions::JumpIfFalse(4),           // [result, input, 1]
    instructions::MultiplyNumbers(0, 0, 1), // [result, input, 1]
    instructions::SubtractNumbers(1, 1, 2), // [result, input, 1]
    instructions::Jump(-5),                 // [result, input, 1]
    instructions::Exit(),
  };

  REQUIRE(context.registers().size() == 0);
  REQUIRE_RESULT(context.Execute(instructions, constants));

  return context.registers()[0].as<double>();
}

}

TEST_CASE("Calculate factorial", "[ovis][core][vm]") {
  REQUIRE(CalculateFactorial(0) == 1.0);
  REQUIRE(CalculateFactorial(1) == 1.0);
  REQUIRE(CalculateFactorial(2) == 2.0);
  REQUIRE(CalculateFactorial(3) == 6.0);
  REQUIRE(CalculateFactorial(4) == 24.0);
  REQUIRE(CalculateFactorial(18) == 6402373705728000);

  REQUIRE(CalculateFactorial2(0) == 1.0);
  REQUIRE(CalculateFactorial2(1) == 1.0);
  REQUIRE(CalculateFactorial2(2) == 2.0);
  REQUIRE(CalculateFactorial2(3) == 6.0);
  REQUIRE(CalculateFactorial2(4) == 24.0);
  REQUIRE(CalculateFactorial2(18) == 6402373705728000);
}

// TEST_CASE("Register Type", "[ovis][core][vm]") {
//   using namespace ovis;
//   using namespace ovis::vm;
//   std::shared_ptr<Module> test_module = RegisterTestModule();

//   SECTION("Check Module Registration") {
//     REQUIRE(test_module->name() == "Test");
//   }

//   SECTION("Basic type Registration") {
//     struct Foo {};

//     REQUIRE(test_module->GetType("Foo") == nullptr);
//     auto foo_type = test_module->RegisterType<Foo>("Foo");
//     REQUIRE(foo_type != nullptr);
//     REQUIRE(foo_type == test_module->GetType("Foo"));
//     REQUIRE(foo_type == Type::Get<Foo>());

//     Value foo = Value::Create(Foo{});
//     REQUIRE(vm::Type::Get(foo.type_id()) == foo_type);
//     Foo& foo2 = foo.Get<Foo>();

//     auto foo_type_again = test_module->RegisterType<Foo>("Foo2");
//     REQUIRE(foo_type_again == nullptr);

//     struct Bar {};
//     auto bar_as_foo = test_module->RegisterType<Bar>("Foo");
//     REQUIRE(bar_as_foo == nullptr);
//   }

//   SECTION("Register reference type") {
//     struct Foo : public SafelyReferenceable {};

//     auto foo_type = test_module->RegisterType<Foo>("Foo");

//     std::unique_ptr<Foo> foo = std::make_unique<Foo>();
//     Value foo_value = Value::Create(foo.get());
//     REQUIRE(vm::Type::Get(foo_value.type_id()) == foo_type);
//     REQUIRE(foo_value.Get<Foo*>() == foo.get());
//     Value foo_value2 = vm::Value::Create(*foo.get());
//     REQUIRE(vm::Type::Get(foo_value2.type_id()) == foo_type);
//     REQUIRE(foo_value2.Get<Foo*>() == foo.get());

//     foo.reset();

//     REQUIRE(foo_value.Get<Foo*>() == nullptr);
//     REQUIRE(foo_value2.Get<Foo*>() == nullptr);
//   }

//   SECTION("Push value") {
//     auto int_type = test_module->RegisterType<int>("Integer");
//     {
//       ExecutionContext::global_context()->PushValue2(Value::CreateViewIfPossible(10));
//       Value v = ExecutionContext::global_context()->GetTopValue();
//       REQUIRE(vm::Type::Get(v.type_id()) == int_type);
//       REQUIRE(v.is_view() == false);
//       REQUIRE(v.Get<int>() == 10);
//       ExecutionContext::global_context()->PopValue();
//     }

//     {
//       int i = 10;
//       ExecutionContext::global_context()->PushValue2(Value::CreateViewIfPossible(i));
//       Value v = ExecutionContext::global_context()->GetTopValue();
//       REQUIRE(vm::Type::Get(v.type_id()) == int_type);
//       REQUIRE(v.is_view() == true);
//       REQUIRE(v.Get<int>() == 10);
//       i = 11;
//       REQUIRE(v.Get<int>() == 11);
//       ExecutionContext::global_context()->PopValue();
//     }
//   }

//   SECTION("Basic type registration with base") {
//     struct Base {
//       int i;
//     };
//     struct Derived : public Base {};
//     struct DerivedSquared : public Derived {};
//     struct Foo : public Base {};
//     struct Functions {
//       static int UseBase(const Base& b) {
//         return 2 * b.i;
//       }
//     };

//     auto base_type = test_module->RegisterType<Base>("Base");
//     auto derived_type = test_module->RegisterType<Derived, Base>("Derived");
//     auto derived_squared_type = test_module->RegisterType<DerivedSquared, Derived>("Derived Squared");
//     auto foo_type = test_module->RegisterType<Foo>("Foo");
//     REQUIRE(derived_type->IsDerivedFrom(base_type));
//     REQUIRE(derived_type->IsDerivedFrom<Base>());
//     REQUIRE(!base_type->IsDerivedFrom(derived_type));
//     REQUIRE(!base_type->IsDerivedFrom<Derived>());
//     REQUIRE(derived_squared_type->IsDerivedFrom(derived_type));
//     REQUIRE(derived_squared_type->IsDerivedFrom<Derived>());
//     REQUIRE(derived_squared_type->IsDerivedFrom(base_type));
//     REQUIRE(derived_squared_type->IsDerivedFrom<Base>());
//     REQUIRE(!foo_type->IsDerivedFrom(derived_type));
//     REQUIRE(!foo_type->IsDerivedFrom<Derived>());

//     auto use_base = test_module->RegisterFunction<&Functions::UseBase>("Use Base", { "base" }, { "result" });
//     Derived d;
//     d.i = 1336;
//     Value test = Value::CreateViewIfPossible(d);
//     REQUIRE(test.Get<Derived>().i == 1336);
//     d.i = 1337;
//     REQUIRE(test.Get<Derived>().i == 1337);

//     Base& b = test.Get<Base>();
//     REQUIRE(b.i == 1337);

//     REQUIRE(use_base->Call<int>(d) == 2674);

//     DerivedSquared ds;
//     ds.i = 69;
//     REQUIRE(use_base->Call<int>(ds) == 138);
//   }

//   SECTION("Basic type registration with base") {
//     struct Base {
//       int i;
//     };
//     struct Derived : public Base {
//     };
//     struct Foo : public Base {};
//     struct Functions {
//       static void UseBase(const Base& b) {}
//     };

//     auto base_type = test_module->RegisterType<Base>("Base");
//     auto derived_type = test_module->RegisterType<Derived, Base>("Derived");
//     auto foo_type = test_module->RegisterType<Foo>("Foo");
//     REQUIRE(derived_type->IsDerivedFrom(base_type));
//     REQUIRE(derived_type->IsDerivedFrom<Base>());
//     REQUIRE(!base_type->IsDerivedFrom(derived_type));
//     REQUIRE(!base_type->IsDerivedFrom<Derived>());
//     REQUIRE(!foo_type->IsDerivedFrom(derived_type));
//     REQUIRE(!foo_type->IsDerivedFrom<Derived>());

//     auto use_base = test_module->RegisterFunction<&Functions::UseBase>("Use Base", { "base" }, {});
//     Derived d;
//     use_base->Call<>(d);
//   }

//   SECTION("Type serialization") {
//     struct Foo {
//       static json Serialize(const vm::Value& value) {
//         assert(value.type_id() == vm::Type::GetId<Foo>());
//         const auto& foo = value.Get<Foo>();
//         return foo.number;
//       }

//       static vm::Value Deserialize(const json& data) {
//         if (!data.is_number()) {
//           LogE("'data' is not a number");
//           return Value::None();
//         }
//         return Value::Create(Foo{
//           .number = data,
//         });
//       }

//       double number;
//     };

//     auto foo_type = test_module->RegisterType<Foo>("Foo");
//     REQUIRE(foo_type != nullptr);
//     REQUIRE(vm::Type::Get(foo_type->CreateValue(json(1337)).type_id()) == nullptr);
  
//     foo_type->SetSerializeFunction(&Foo::Serialize);
//     foo_type->SetDeserializeFunction(&Foo::Deserialize);

//     Value value = foo_type->CreateValue(json(1337));
//     REQUIRE(vm::Type::Get(value.type_id()) == foo_type);
//     REQUIRE(value.Get<Foo>().number == 1337);
//     REQUIRE(value.Serialize() == json(1337));

//     foo_type->SetSerializeFunction(nullptr);
//     REQUIRE(value.Serialize().is_null());

//     Value value2 = Value::None();
//     REQUIRE(value2.Serialize().is_null());
//   }

//   SECTION("Constructor Registration") {
//     struct Foo {
//       Foo(int a) : a(a) {}
//       int a;
//     };

//     auto int_type = test_module->RegisterType<int>("Integer");
//     auto foo_type = test_module->RegisterType<Foo>("Foo");

//     auto constructor = test_module->RegisterFunction<Constructor<Foo, int>>("Create Foo", { "a" }, { "foo" });
//     REQUIRE(constructor != nullptr);

//     Foo f = constructor->Call<Foo>(100);
//     REQUIRE(f.a == 100);

//     foo_type->RegisterConstructorFunction(constructor);
//     Value f2_value = foo_type->Construct(100);
//     REQUIRE(vm::Type::Get(f2_value.type_id()) == foo_type);
//     REQUIRE(f2_value.Get<Foo>().a == 100);
//   }

//   SECTION("Property Registration") {
//     struct Foo {
//       int a;
//     };

//     auto int_type = test_module->RegisterType<int>("Integer");
//     auto foo_type = test_module->RegisterType<Foo>("Foo");

//     REQUIRE(foo_type->properties().size() == 0);
//     foo_type->RegisterProperty<&Foo::a>("a");
//     REQUIRE(foo_type->properties().size() == 1);
//     REQUIRE(foo_type->properties()[0].name == "a");

//     Value v = Value::Create(Foo{ .a = 100 });
//     REQUIRE(v.GetProperty<int>("a") == 100); // v["a"].Get<int>() == 100 ?

//     v.SetProperty("a", 200); // v["a"] = 200 ?
//     REQUIRE(v.GetProperty<int>("a") == 200);
//   }

//   SECTION("Function Registration") {
//     struct Foo {
//       static int foo() { return 42; }
//     };
//     test_module->RegisterType<int>("Integer");

//     REQUIRE(test_module->GetFunction("foo") == nullptr);
//     auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {}, {"The answer"});
//     REQUIRE(foo_function != nullptr);
//     REQUIRE(test_module->GetFunction("foo") == foo_function);
//     REQUIRE(foo_function->inputs().size() == 0);
//     REQUIRE(foo_function->outputs().size() == 1);
//     REQUIRE(foo_function->GetOutput(0)->name == "The answer");
//     REQUIRE(foo_function->GetOutput(0)->type.lock() == Type::Get<int>());

//     REQUIRE(foo_function->Call<int>() == 42);
//   }

//   SECTION("Function with parameter") {
//     struct Foo {
//       static int foo(int i) { return i * 2; }
//     };
//     test_module->RegisterType<int>("Integer");

//     REQUIRE(test_module->GetFunction("foo") == nullptr);
//     auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {"An awesome parameter"}, {"The answer"});
//     REQUIRE(foo_function != nullptr);
//     REQUIRE(test_module->GetFunction("foo") == foo_function);

//     REQUIRE(foo_function->inputs().size() == 1);
//     REQUIRE(foo_function->GetInput(0)->name == "An awesome parameter");
//     REQUIRE(foo_function->GetInput(0)->type.lock() == Type::Get<int>());
//     REQUIRE(foo_function->outputs().size() == 1);
//     REQUIRE(foo_function->GetOutput(0)->name == "The answer");
//     REQUIRE(foo_function->GetOutput(0)->type.lock() == Type::Get<int>());

//     REQUIRE(foo_function->Call<int>(21) == 42);
//     REQUIRE(foo_function->Call<int>(210) == 420);
//     REQUIRE(foo_function->Call<int>(1337) == 2674);
//   }

//   SECTION("Function with reference parameter") {
//     struct Foo {
//       static void foo(int& i) { i *= 2; }
//     };

//     test_module->RegisterType<int>("Integer");
//     auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {"An awesome parameter"}, {});
//     REQUIRE(foo_function != nullptr);

//     int y = 42;
//     foo_function->Call(y);
//     REQUIRE(y == 84);

//     // ValueView from value not implemented yet.
//     // Value x = Value::Create(42);
//     // REQUIRE(x.type().lock() == Type::Get<int>());
//     // REQUIRE(x.Get<int>() == 42);

//     // foo_function->Call(x);

//     // REQUIRE(x.type().lock() == Type::Get<int>());
//     // REQUIRE(x.Get<int>() == 84);
//   }

//   SECTION("Function with tuple parameter") {
//     struct Foo {
//       static int foo(std::tuple<int, int> value) { return std::get<0>(value) + std::get<1>(value); }
//     };
//     test_module->RegisterType<int>("Integer");

//     REQUIRE(test_module->GetFunction("foo") == nullptr);
//     auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {"Some Value", "Some other value"}, {"The answer"});
//     REQUIRE(foo_function != nullptr);
//     REQUIRE(test_module->GetFunction("foo") == foo_function);

//     REQUIRE(foo_function->inputs().size() == 2);
//     REQUIRE(foo_function->GetInput(0)->name == "Some Value");
//     REQUIRE(foo_function->GetInput(0)->type.lock() == Type::Get<int>());
//     REQUIRE(foo_function->GetInput(1)->name == "Some other value");
//     REQUIRE(foo_function->GetInput(1)->type.lock() == Type::Get<int>());
//     REQUIRE(foo_function->outputs().size() == 1);
//     REQUIRE(foo_function->GetOutput(0)->name == "The answer");
//     REQUIRE(foo_function->GetOutput(0)->type.lock() == Type::Get<int>());

//     REQUIRE(foo_function->Call<int>(21, 21) == 42);
//     REQUIRE(foo_function->Call<int>(1337, 42) == 1379);
//   }

//   SECTION("Function with multiple return values") {
//     struct Foo {
//       static std::tuple<bool, int> foo() { return std::make_tuple(true, 42); }
//     };
//     test_module->RegisterType<bool>("Boolean");
//     test_module->RegisterType<int>("Integer");

//     REQUIRE(test_module->GetFunction("foo") == nullptr);
//     auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {}, {"Am I cool", "The answer"});
//     REQUIRE(foo_function != nullptr);
//     REQUIRE(test_module->GetFunction("foo") == foo_function);

//     REQUIRE(foo_function->inputs().size() == 0);
//     REQUIRE(foo_function->outputs().size() == 2);
//     REQUIRE(foo_function->GetOutput(0)->name == "Am I cool");
//     REQUIRE(foo_function->GetOutput(0)->type.lock() == Type::Get<bool>());
//     REQUIRE(foo_function->GetOutput(1)->name == "The answer");
//     REQUIRE(foo_function->GetOutput(1)->type.lock() == Type::Get<int>());

//     auto results = foo_function->Call<bool, int>();
//     REQUIRE(std::get<bool>(results) == true);
//     REQUIRE(std::get<int>(results) == 42);
//   }
// }
