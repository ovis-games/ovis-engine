#include <catch2/catch.hpp>

#include <ovis/core/scripting.hpp>

using namespace ovis;

float add(float x, float y) {
  return x + y;
}

TEST_CASE("Simple functions", "[ovis][core][Scripting]") {
  ScriptContext context;
  SECTION("Register function") {
    // context.RegisterFunction(
    //     "test",
    //     [](std::span<ScriptVariable> inputs, std::span<ScriptVariable> outputs) {
    //       REQUIRE(outputs.size() == 1);
    //       outputs[0].type = "test";
    //     },
    //     0, 1);

    const auto result = context.Execute("test", {});
    REQUIRE(std::holds_alternative<std::vector<ScriptVariable>>(result));
    const auto outputs = std::get<std::vector<ScriptVariable>>(result);
    REQUIRE(outputs.size() == 1);
    REQUIRE(outputs[0].type == "test");
  }


  SECTION("Register with templates") {
    context.RegisterFunction<float(float, float), add>("add");
    std::vector<ScriptVariable> inputs { ScriptVariable{"", 1.0f}, ScriptVariable{"", 2.0f} };

    const auto result = context.Execute("add", inputs);
    REQUIRE(std::holds_alternative<std::vector<ScriptVariable>>(result));
    
    const auto outputs = std::get<std::vector<ScriptVariable>>(result);
    REQUIRE(outputs.size() == 1);
    REQUIRE(outputs[0].value.type() == typeid(float));
    REQUIRE(std::any_cast<float>(outputs[0].value) == 3.0f);
  }


  SECTION("Register with templates") {
    json x = R"(
      {
        actions: [
          {
            "type": "function_call",
            "function": "add",
            "arguments": [ "!Number:1", "!Number:2" ]
          },
          {
            "type": "function_call",
            "function": "negate",
            "arguments": [ "$0:0" ]
          },
          {
            "type": "function_call",
            "function": "number_to_string",
            "arguments": [ "$1:0" ]
          },
          {
            "type": "function_call",
            "function": "log",
            "arguments": [ "!String:\"The result is: \"", :q

          }
        ]
      }
    )"_json;
  }
}
