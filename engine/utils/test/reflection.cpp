#include <catch2/catch.hpp>

#include <ovis/utils/reflection.hpp>

TEST_CASE("Register Type", "[ovis][utils][reflection]") {
  using namespace ovis;
  Type::DeregisterAll();
  Function::DeregisterAll();

  SECTION("Basic Registration") {
    struct Foo {};

    REQUIRE(Type::Get("Foo") == nullptr);
    auto foo_type = Type::Register<Foo>("Foo");
    REQUIRE(foo_type != nullptr);
    REQUIRE(foo_type == Type::Get("Foo"));
    REQUIRE(foo_type == Type::Get<Foo>());

    Value foo(Foo{});
    REQUIRE(foo.type() == foo_type);
    Foo& foo2 = foo.Get<Foo>();

    auto foo_type_again = Type::Register<Foo>("Foo2");
    REQUIRE(foo_type_again == nullptr);

    struct Bar {};
    auto bar_as_foo = Type::Register<Bar>("Foo");
    REQUIRE(bar_as_foo == nullptr);

    REQUIRE(Type::Deregister("Foo"));
    REQUIRE(Type::Get("Foo") == nullptr);
    REQUIRE(Type::Get<Foo>() == nullptr);
    REQUIRE(foo_type == nullptr);
    REQUIRE(foo.type() == nullptr);

    REQUIRE(!Type::Deregister("Foo"));
  }

  SECTION("Property Registration") {
    struct Foo {
      int a;
    };

    auto int_type = Type::Register<int>("Integer");
    auto foo_type = Type::Register<Foo>("Foo");

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
    Type::Register<int>("Integer");

    REQUIRE(Function::Get("foo") == nullptr);
    auto foo_function = Function::Register<&Foo::foo>("foo", {}, {"The answer"});
    REQUIRE(foo_function != nullptr);
    REQUIRE(Function::Get("foo") == foo_function);

    auto results = foo_function->Call();
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].type() == Type::Get<int>());
    REQUIRE(results[0].Get<int>() == 42);

    auto results2 = Function::Call("foo");
    REQUIRE(results2.size() == 1);
    REQUIRE(results2[0].type() == Type::Get<int>());
    REQUIRE(results2[0].Get<int>() == 42);
  }

  SECTION("Function with parameter") {
    struct Foo {
      static int foo(int i) { return i * 2; }
    };
    Type::Register<int>("Integer");

    REQUIRE(Function::Get("foo") == nullptr);
    auto foo_function = Function::Register<&Foo::foo>("foo", {"An awesome parameter"}, {"The answer"});
    REQUIRE(foo_function != nullptr);
    REQUIRE(Function::Get("foo") == foo_function);

    auto results = foo_function->Call(std::vector{ Value(21) });
    REQUIRE(results.size() == 1);
    REQUIRE(results[0].type() == Type::Get<int>());
    REQUIRE(results[0].Get<int>() == 42);
  }

  SECTION("Function with multiple return values") {
    struct Foo {
      static std::tuple<bool, int> foo() { return std::make_tuple(true, 42); }
    };
    Type::Register<bool>("Boolean");
    Type::Register<int>("Integer");

    REQUIRE(Function::Get("foo") == nullptr);
    auto foo_function = Function::Register<&Foo::foo>("foo", {}, {"Am I cool", "The answer"});
    REQUIRE(foo_function != nullptr);
    REQUIRE(Function::Get("foo") == foo_function);

    auto results = Function::Call("foo");
    REQUIRE(results.size() == 2);
    REQUIRE(results[0].type() == Type::Get<bool>());
    REQUIRE(results[0].Get<bool>() == true);
    REQUIRE(results[1].type() == Type::Get<int>());
    REQUIRE(results[1].Get<int>() == 42);
  }
}
