#include <catch2/catch.hpp>

#include <ovis/core/virtual_machine.hpp>

ovis::safe_ptr<ovis::vm::Module> RegisterTestModule() {
  if (ovis::vm::Module::Get("Test") != nullptr) {
    ovis::vm::Module::Deregister("Test");
  }
  return ovis::vm::Module::Register("Test");
}

TEST_CASE("Register Type", "[ovis][core][vm]") {
  using namespace ovis;
  using namespace ovis::vm;
  safe_ptr<Module> test_module = RegisterTestModule();

  SECTION("Check Module Registration") {
    REQUIRE(test_module->name() == "Test");
  }

  SECTION("Basic type Registration") {
    struct Foo {};

    REQUIRE(test_module->GetType("Foo") == nullptr);
    auto foo_type = test_module->RegisterType<Foo>("Foo");
    REQUIRE(foo_type != nullptr);
    REQUIRE(foo_type == test_module->GetType("Foo"));
    REQUIRE(foo_type == Type::Get<Foo>());

    Value foo(Foo{});
    REQUIRE(foo.type() == foo_type);
    Foo& foo2 = foo.Get<Foo>();

    auto foo_type_again = test_module->RegisterType<Foo>("Foo2");
    REQUIRE(foo_type_again == nullptr);

    struct Bar {};
    auto bar_as_foo = test_module->RegisterType<Bar>("Foo");
    REQUIRE(bar_as_foo == nullptr);

    // REQUIRE(Type::Deregister("Foo"));
    // REQUIRE(test_module->GetType("Foo") == nullptr);
    // REQUIRE(test_module->GetType<Foo>() == nullptr);
    // REQUIRE(foo_type == nullptr);
    // REQUIRE(foo.type() == nullptr);

    // REQUIRE(!Type::Deregister("Foo"));
  }

  SECTION("Basic type registration with base") {
    struct Base {
      int i;
    };
    struct Derived : public Base {
    };
    struct Foo : public Base {};
    struct Functions {
      static void UseBase(const Base& b) {}
    };

    auto base_type = test_module->RegisterType<Base>("Base");
    auto derived_type = test_module->RegisterType<Derived, Base>("Derived");
    auto foo_type = test_module->RegisterType<Foo>("Foo");
    REQUIRE(derived_type->IsDerivedFrom(base_type));
    REQUIRE(derived_type->IsDerivedFrom<Base>());
    REQUIRE(!base_type->IsDerivedFrom(derived_type));
    REQUIRE(!base_type->IsDerivedFrom<Derived>());
    REQUIRE(!foo_type->IsDerivedFrom(derived_type));
    REQUIRE(!foo_type->IsDerivedFrom<Derived>());

    auto use_base = test_module->RegisterFunction<&Functions::UseBase>("Use Base", { "base" }, {});
    Derived d;
    use_base->Call<>(d);
  }

  SECTION("Type serialization") {
    struct Foo {
      static json Serialize(const vm::Value& value) {
        assert(value.type() == vm::Type::Get<Foo>().get());
        const auto& foo = value.Get<Foo>();
        return foo.number;
      }

      static vm::Value Deserialize(const json& data) {
        if (!data.is_number()) {
          LogE("'data' is not a number");
          return {};
        }
        return Foo {
          .number = data,
        };
      }

      double number;
    };

    auto foo_type = test_module->RegisterType<Foo>("Foo");
    REQUIRE(foo_type != nullptr);
    REQUIRE(foo_type->CreateValue(json(1337)).type() == nullptr);
  
    foo_type->SetSerializeFunction(&Foo::Serialize);
    foo_type->SetDeserializeFunction(&Foo::Deserialize);

    Value value = foo_type->CreateValue(json(1337));
    REQUIRE(value.type() == foo_type);
    REQUIRE(value.Get<Foo>().number == 1337);
    REQUIRE(value.Serialize() == json(1337));

    foo_type->SetSerializeFunction(nullptr);
    REQUIRE(value.Serialize().is_null());

    Value value2;
    REQUIRE(value2.Serialize().is_null());
  }

  SECTION("Constructor Registration") {
    struct Foo {
      Foo(int a) : a(a) {}
      int a;
    };

    auto int_type = test_module->RegisterType<int>("Integer");
    auto foo_type = test_module->RegisterType<Foo>("Foo");

    auto constructor = test_module->RegisterFunction<Constructor<Foo, int>>("Create Foo", { "a" }, { "foo" });
    REQUIRE(constructor != nullptr);

    Foo f = constructor->Call<Foo>(100);
    REQUIRE(f.a == 100);

    foo_type->RegisterConstructorFunction(constructor);
    Value f2_value = foo_type->Construct(100);
    REQUIRE(f2_value.type() == foo_type);
    REQUIRE(f2_value.Get<Foo>().a == 100);
  }

  SECTION("Property Registration") {
    struct Foo {
      int a;
    };

    auto int_type = test_module->RegisterType<int>("Integer");
    auto foo_type = test_module->RegisterType<Foo>("Foo");

    REQUIRE(foo_type->properties().size() == 0);
    foo_type->RegisterProperty<&Foo::a>("a");
    REQUIRE(foo_type->properties().size() == 1);
    REQUIRE(foo_type->properties()[0].name == "a");

    Value v(Foo{ .a = 100 });
    REQUIRE(v.GetProperty<int>("a") == 100); // v["a"].Get<int>() == 100 ?

    v.SetProperty("a", 200); // v["a"] = 200 ?
    REQUIRE(v.GetProperty<int>("a") == 200);
  }

  SECTION("Function Registration") {
    struct Foo {
      static int foo() { return 42; }
    };
    test_module->RegisterType<int>("Integer");

    REQUIRE(test_module->GetFunction("foo") == nullptr);
    auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {}, {"The answer"});
    REQUIRE(foo_function != nullptr);
    REQUIRE(test_module->GetFunction("foo") == foo_function);
    REQUIRE(foo_function->inputs().size() == 0);
    REQUIRE(foo_function->outputs().size() == 1);
    REQUIRE(foo_function->GetOutput(0)->name == "The answer");
    REQUIRE(foo_function->GetOutput(0)->type == Type::Get<int>());

    REQUIRE(foo_function->Call<int>() == 42);
  }

  SECTION("Function with parameter") {
    struct Foo {
      static int foo(int i) { return i * 2; }
    };
    test_module->RegisterType<int>("Integer");

    REQUIRE(test_module->GetFunction("foo") == nullptr);
    auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {"An awesome parameter"}, {"The answer"});
    REQUIRE(foo_function != nullptr);
    REQUIRE(test_module->GetFunction("foo") == foo_function);

    REQUIRE(foo_function->inputs().size() == 1);
    REQUIRE(foo_function->GetInput(0)->name == "An awesome parameter");
    REQUIRE(foo_function->GetInput(0)->type == Type::Get<int>());
    REQUIRE(foo_function->outputs().size() == 1);
    REQUIRE(foo_function->GetOutput(0)->name == "The answer");
    REQUIRE(foo_function->GetOutput(0)->type == Type::Get<int>());

    REQUIRE(foo_function->Call<int>(21) == 42);
    REQUIRE(foo_function->Call<int>(210) == 420);
    REQUIRE(foo_function->Call<int>(1337) == 2674);
  }

  SECTION("Function with tuple parameter") {
    struct Foo {
      static int foo(std::tuple<int, int> value) { return std::get<0>(value) + std::get<1>(value); }
    };
    test_module->RegisterType<int>("Integer");

    REQUIRE(test_module->GetFunction("foo") == nullptr);
    auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {"Some Value", "Some other value"}, {"The answer"});
    REQUIRE(foo_function != nullptr);
    REQUIRE(test_module->GetFunction("foo") == foo_function);

    REQUIRE(foo_function->inputs().size() == 2);
    REQUIRE(foo_function->GetInput(0)->name == "Some Value");
    REQUIRE(foo_function->GetInput(0)->type == Type::Get<int>());
    REQUIRE(foo_function->GetInput(1)->name == "Some other value");
    REQUIRE(foo_function->GetInput(1)->type == Type::Get<int>());
    REQUIRE(foo_function->outputs().size() == 1);
    REQUIRE(foo_function->GetOutput(0)->name == "The answer");
    REQUIRE(foo_function->GetOutput(0)->type == Type::Get<int>());

    REQUIRE(foo_function->Call<int>(21, 21) == 42);
    REQUIRE(foo_function->Call<int>(1337, 42) == 1379);
  }

  SECTION("Function with multiple return values") {
    struct Foo {
      static std::tuple<bool, int> foo() { return std::make_tuple(true, 42); }
    };
    test_module->RegisterType<bool>("Boolean");
    test_module->RegisterType<int>("Integer");

    REQUIRE(test_module->GetFunction("foo") == nullptr);
    auto foo_function = test_module->RegisterFunction<&Foo::foo>("foo", {}, {"Am I cool", "The answer"});
    REQUIRE(foo_function != nullptr);
    REQUIRE(test_module->GetFunction("foo") == foo_function);

    REQUIRE(foo_function->inputs().size() == 0);
    REQUIRE(foo_function->outputs().size() == 2);
    REQUIRE(foo_function->GetOutput(0)->name == "Am I cool");
    REQUIRE(foo_function->GetOutput(0)->type == Type::Get<bool>());
    REQUIRE(foo_function->GetOutput(1)->name == "The answer");
    REQUIRE(foo_function->GetOutput(1)->type == Type::Get<int>());

    auto results = foo_function->Call<bool, int>();
    REQUIRE(std::get<bool>(results) == true);
    REQUIRE(std::get<int>(results) == 42);
  }
}
