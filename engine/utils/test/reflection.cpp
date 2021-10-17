#include <catch2/catch.hpp>

#include <ovis/utils/reflection.hpp>

TEST_CASE("Register Type", "[ovis][utils][reflection]") {
  using namespace ovis;

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
  }

  SECTION("Property Registration") {
    struct Foo {
      int a;
    };

    auto int_type = Type::Register<int>("Integer");
    auto foo_type = Type::Register<Foo>("Foo");
    foo_type->RegisterProperty<&Foo::a>("a");

    Value v(Foo{ .a = 100 });
    REQUIRE(v.GetProperty<int>("a") == 100); // v["a"].Get<int>() == 100 ?

    v.SetProperty("a", 200); // v["a"] = 200 ?
    REQUIRE(v.GetProperty<int>("a") == 200);
  }
}
