#include <catch2/catch.hpp>

#include <ovis/utils/reflection.hpp>

TEST_CASE("Register Type", "[ovis][utils][reflection]") {
  using namespace ovis;
  Type::DeregisterAll();

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
}
