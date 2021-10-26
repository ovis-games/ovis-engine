#include <benchmark/benchmark.h>
#include <ovis/core/virtual_machine.hpp>
#include <ovis/core/script_function.hpp>
#include <ovis/core/script_parser.hpp>
#include <ovis/core/core_module.hpp>
#include <sol/sol.hpp>

const ovis::json factorial_json = ovis::json::parse(R"(
  {
    "inputs": [
      {
        "name": "n",
        "type": "Number",
        "module": "Core"
      }
    ],
    "outputs": [
      {
        "name": "result",
        "type": "Number",
        "module": "Core"
      }
    ],
    "actions": [
      {
        "type": "function_call",
        "function": {
          "name": "Create Number",
          "module": "Core"
        },
        "inputs": {
          "value": 1
        },
        "outputs": {
          "result": "result"
        }
      },
      {
        "type": "function_call",
        "function": {
          "name": "Is greater",
          "module": "Core"
        },
        "inputs": {
          "x": {
            "inputType": "local_variable",
            "name": "n"
          },
          "y": 1
        },
        "outputs": {
          "result": "continue"
        }
      },
      {
        "type": "while",
        "condition": {
          "inputType": "local_variable",
          "name": "continue"
        },
        "actions": [
          {
            "type": "function_call",
            "function": {
              "name": "Multiply",
              "module": "Core"
            },
            "inputs": {
              "x": {
                "inputType": "local_variable",
                "name": "n"
              },
              "y": {
                "inputType": "local_variable",
                "name": "result"
              }
            },
            "outputs": {
              "result": "result"
            }
          },
          {
            "type": "function_call",
            "function": {
              "name": "Subtract",
              "module": "Core"
            },
            "inputs": {
              "x": {
                "inputType": "local_variable",
                "name": "n"
              },
              "y": 1
            },
            "outputs": {
              "result": "n"
            }
          },
          {
            "type": "function_call",
            "function": {
              "name": "Is greater",
              "module": "Core"
            },
            "inputs": {
              "x": {
                "inputType": "local_variable",
                "name": "n"
              },
              "y": 1
            },
            "outputs": {
              "result": "continue"
            }
          }
        ]
      },
      {
        "type": "return",
        "outputs": {
          "result": {
            "inputType": "local_variable",
            "name": "result"
          }
        }
      }
    ]
  }
)");

const std::string factorial_lua = R"(
    function factorial(n)
      local result = 1
      while (n > 0) do
        result = result * n
        n = n - 1
      end
      return result
    end
)";

static void BM_ParseFactorialFunctionVM(benchmark::State& state) {
  ovis::LoadCoreModule();

  for (auto _ : state) {
    ovis::ScriptFunctionParser factorial_parser(factorial_json);
  }
}

static void BM_ParseFactorialFunctionLua(benchmark::State& state) {
  sol::state lua;

  for (auto _ : state) {
    lua.script(factorial_lua);
  }
}

static void BM_CalculateFactorialCPP(benchmark::State& state) {
  auto factorial = [](double n) {
    double result = 1.0;
    while (n > 1.0) {
      result *= n;
      --n;
    }
    return result;
  };
  const double n = state.range(0);

  for (auto _ : state) {
    benchmark::DoNotOptimize(factorial(n));
  }
}

static void BM_CalculateFactorialVM(benchmark::State& state) {
  ovis::LoadCoreModule();
  ovis::ScriptFunction factorial(factorial_json);
  const double n = state.range(0);

  for (auto _ : state) {
    benchmark::DoNotOptimize(factorial.Call<double>(n));
  }
}

static void BM_CalculateFactorialLua(benchmark::State& state) {
  sol::state lua;
  lua.script(factorial_lua);
  const double n = state.range(0);

  for (auto _ : state) {
    benchmark::DoNotOptimize(lua["factorial"](n));
  }
}

BENCHMARK(BM_ParseFactorialFunctionVM);
BENCHMARK(BM_ParseFactorialFunctionLua);
BENCHMARK(BM_CalculateFactorialCPP)->Range(1, 18);
BENCHMARK(BM_CalculateFactorialVM)->Range(1, 18);
BENCHMARK(BM_CalculateFactorialLua)->Range(1, 18);

BENCHMARK_MAIN();
