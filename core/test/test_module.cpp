#include <catch2/catch.hpp>

#include "test_utils.hpp"

TEST_CASE("Register a module", "[ovis][core][Module]") {
  REQUIRE(ovis::Module::Get("TestModule") == nullptr);
  ovis::Module::Register("TestModule");
  auto test_module = ovis::Module::Get("TestModule");
  REQUIRE(test_module != nullptr);
  REQUIRE(test_module->name() == "TestModule");

  struct SomeType {
    double number;
  };
  const auto some_type_description = ovis::TypeDescription::CreateForNativeType<SomeType>("SomeType");
  REQUIRE(some_type_description.native_type_id == ovis::TypeOf<SomeType>);
  const auto some_type = test_module->RegisterType(some_type_description);
  REQUIRE(some_type != nullptr);
  REQUIRE(some_type->id() != ovis::Type::NONE_ID);
  REQUIRE(ovis::Type::Get<SomeType>() == some_type);
  REQUIRE(ovis::Type::GetId<SomeType>() == some_type->id());
  REQUIRE(test_module->GetType("SomeType") == some_type);


  ovis::Module::Deregister("TestModule");
  REQUIRE(ovis::Module::Get("TestModule") == nullptr);

  // This should work at some point, but it would require some rework or
  // a hacky fix by storing an "registered" boolean inside the module class.
  // REQUIRE(test_module->GetType("SomeType") == nullptr);
}
