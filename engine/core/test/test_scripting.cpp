#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/script_context.hpp>
#include <ovis/core/asset_library.hpp>

using namespace ovis;

int test(int i) {
  return i + 1;
}

TEST_CASE("Simple functions", "[ovis][core][Scripting]") {
  ScriptContext context;

  SECTION("Register function") {
    // const auto test = [](int i) { return i + 1; };
    context.RegisterFunction<test>("test", { "i" }, { "out" });
    const auto function = context.GetFunction("test");

    REQUIRE(function->inputs.size() == 1);
    REQUIRE(function->inputs[0].type == context.GetTypeId<int>());
    REQUIRE(function->inputs[0].identifier == "i");

    REQUIRE(function->outputs.size() == 1);
    REQUIRE(function->outputs[0].type == context.GetTypeId<int>());
    REQUIRE(function->outputs[0].identifier == "out");
  }
}

TEST_CASE("Documentation", "[ovis][core][Scripting]") {
  Log::AddListener(ConsoleLogger);
  SetEngineAssetsDirectory("/ovis_assets");
  REQUIRE(global_script_context()->LoadDocumentation());

  const auto* add_function = global_script_context()->GetFunction("add");
  REQUIRE(add_function != nullptr);

  REQUIRE(add_function->text == "(x) + (y)");
  REQUIRE(add_function->description == "Adds the two numbers (x) and (y)");
}
