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

const std::string factorial_lua_function_calls = R"(
  function factorial(n)
    local result = 1
    while greater(n, 1) do
      result = multiply(result, n)
      n = subtract(n, 1)
    end
    return result
  end
)";

template <typename... T>
double Product(T... values) {
  static_assert((... && std::is_same_v<T, double>));
  return (values * ... * 1.0);
}

template <typename... T>
double __attribute__((noinline)) ProductNoInline(T... values) {
  static_assert((... && std::is_same_v<T, double>));
  return (values * ... * 1.0);
}

template <int... Values>
void BM_FunctionCallCPP(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(ProductNoInline(static_cast<double>(Values)...));
  }
}
auto BM_FunctionCallCPP0 = &BM_FunctionCallCPP<>;
auto BM_FunctionCallCPP1 = &BM_FunctionCallCPP<1>;
auto BM_FunctionCallCPP2 = &BM_FunctionCallCPP<1, 2>;
auto BM_FunctionCallCPP3 = &BM_FunctionCallCPP<1, 2, 3>;
auto BM_FunctionCallCPP4 = &BM_FunctionCallCPP<1, 2, 3, 4>;
auto BM_FunctionCallCPP5 = &BM_FunctionCallCPP<1, 2, 3, 4, 5>;
auto BM_FunctionCallCPP6 = &BM_FunctionCallCPP<1, 2, 3, 4, 5, 6>;
auto BM_FunctionCallCPP7 = &BM_FunctionCallCPP<1, 2, 3, 4, 5, 6, 7>;
auto BM_FunctionCallCPP8 = &BM_FunctionCallCPP<1, 2, 3, 4, 5, 6, 7, 8>;
auto BM_FunctionCallCPP9 = &BM_FunctionCallCPP<1, 2, 3, 4, 5, 6, 7, 8, 9>;
auto BM_FunctionCallCPP10 = &BM_FunctionCallCPP<1, 2, 3, 4, 5, 6, 7, 8, 9, 10>;
BENCHMARK(BM_FunctionCallCPP0);
BENCHMARK(BM_FunctionCallCPP1);
BENCHMARK(BM_FunctionCallCPP2);
BENCHMARK(BM_FunctionCallCPP3);
BENCHMARK(BM_FunctionCallCPP4);
BENCHMARK(BM_FunctionCallCPP5);
BENCHMARK(BM_FunctionCallCPP6);
BENCHMARK(BM_FunctionCallCPP7);
BENCHMARK(BM_FunctionCallCPP8);
BENCHMARK(BM_FunctionCallCPP9);
BENCHMARK(BM_FunctionCallCPP10);

// template <int... Values>
// static void BM_FunctionCallVM(benchmark::State& state) {
//   ovis::LoadCoreModule();
//   auto module = ovis::vm::Module::Get("Benchmark");
//   if (module == nullptr) {
//     module = ovis::vm::Module::Register("Benchmark");
//   }
//   const std::string function_name = fmt::format("Product{}", sizeof...(Values));
//   auto product = module->GetFunction(function_name);
//   if (product == nullptr) {
//     std::vector<std::string_view> parameter_names(sizeof...(Values), "");
//     product = module->RegisterFunction<&Product<decltype(static_cast<double>(Values))...>>(function_name, parameter_names, { "result" });
//   }

//   for (auto _ : state) {
//     benchmark::DoNotOptimize(product->Call<double>(static_cast<double>(Values)...));
//   }
// }
// auto BM_FunctionCallVM0 = &BM_FunctionCallVM<>;
// auto BM_FunctionCallVM1 = &BM_FunctionCallVM<1>;
// auto BM_FunctionCallVM2 = &BM_FunctionCallVM<1, 2>;
// auto BM_FunctionCallVM3 = &BM_FunctionCallVM<1, 2, 3>;
// auto BM_FunctionCallVM4 = &BM_FunctionCallVM<1, 2, 3, 4>;
// auto BM_FunctionCallVM5 = &BM_FunctionCallVM<1, 2, 3, 4, 5>;
// auto BM_FunctionCallVM6 = &BM_FunctionCallVM<1, 2, 3, 4, 5, 6>;
// auto BM_FunctionCallVM7 = &BM_FunctionCallVM<1, 2, 3, 4, 5, 6, 7>;
// auto BM_FunctionCallVM8 = &BM_FunctionCallVM<1, 2, 3, 4, 5, 6, 7, 8>;
// auto BM_FunctionCallVM9 = &BM_FunctionCallVM<1, 2, 3, 4, 5, 6, 7, 8, 9>;
// auto BM_FunctionCallVM10 = &BM_FunctionCallVM<1, 2, 3, 4, 5, 6, 7, 8, 9, 10>;
// BENCHMARK(BM_FunctionCallVM0);
// BENCHMARK(BM_FunctionCallVM1);
// BENCHMARK(BM_FunctionCallVM2);
// BENCHMARK(BM_FunctionCallVM3);
// BENCHMARK(BM_FunctionCallVM4);
// BENCHMARK(BM_FunctionCallVM5);
// BENCHMARK(BM_FunctionCallVM6);
// BENCHMARK(BM_FunctionCallVM7);
// BENCHMARK(BM_FunctionCallVM8);
// BENCHMARK(BM_FunctionCallVM9);
// BENCHMARK(BM_FunctionCallVM10);

template <int... Values>
static void BM_FunctionCallLua(benchmark::State& state) {
  sol::state lua;
  lua["Product"] = &Product<decltype(static_cast<double>(Values))...>;
  auto product = lua["Product"];

  for (auto _ : state) {
    benchmark::DoNotOptimize(product(static_cast<double>(Values)...));
  }
}
auto BM_FunctionCallLua0 = &BM_FunctionCallLua<>;
auto BM_FunctionCallLua1 = &BM_FunctionCallLua<1>;
auto BM_FunctionCallLua2 = &BM_FunctionCallLua<1, 2>;
auto BM_FunctionCallLua3 = &BM_FunctionCallLua<1, 2, 3>;
auto BM_FunctionCallLua4 = &BM_FunctionCallLua<1, 2, 3, 4>;
auto BM_FunctionCallLua5 = &BM_FunctionCallLua<1, 2, 3, 4, 5>;
auto BM_FunctionCallLua6 = &BM_FunctionCallLua<1, 2, 3, 4, 5, 6>;
auto BM_FunctionCallLua7 = &BM_FunctionCallLua<1, 2, 3, 4, 5, 6, 7>;
auto BM_FunctionCallLua8 = &BM_FunctionCallLua<1, 2, 3, 4, 5, 6, 7, 8>;
auto BM_FunctionCallLua9 = &BM_FunctionCallLua<1, 2, 3, 4, 5, 6, 7, 8, 9>;
auto BM_FunctionCallLua10 = &BM_FunctionCallLua<1, 2, 3, 4, 5, 6, 7, 8, 9, 10>;
BENCHMARK(BM_FunctionCallLua0);
BENCHMARK(BM_FunctionCallLua1);
BENCHMARK(BM_FunctionCallLua2);
BENCHMARK(BM_FunctionCallLua3);
BENCHMARK(BM_FunctionCallLua4);
BENCHMARK(BM_FunctionCallLua5);
BENCHMARK(BM_FunctionCallLua6);
BENCHMARK(BM_FunctionCallLua7);
BENCHMARK(BM_FunctionCallLua8);
BENCHMARK(BM_FunctionCallLua9);
BENCHMARK(BM_FunctionCallLua10);

// static void BM_ParseFactorialFunctionVM(benchmark::State& state) {
//   ovis::LoadCoreModule();

//   for (auto _ : state) {
//     ovis::ScriptFunctionParser factorial_parser(factorial_json);
//   }
// }

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

static void BM_CalculateFactorialVM(benchmark::State& state) {
  const double n = state.range(0);

  ovis::vm::ExecutionContext context;
  std::array<ovis::vm::ValueStorage, 5> constants = {
    n,
    1.0,
    &ovis::vm::NativeFunctionWrapper<Subtract>,
    &ovis::vm::NativeFunctionWrapper<Multiply>,
    &ovis::vm::NativeFunctionWrapper<IsGreater>,
  };

  // r[0] = result
  // r[1] = input
  std::array<ovis::vm::Instruction, 24> instructions = {
    ovis::vm::instructions::PushTrivialConstant(1), // [result]
    ovis::vm::instructions::PushTrivialConstant(0), // [result, input]
    ovis::vm::instructions::Push(1),                // [result, input, void]
    ovis::vm::instructions::CopyTrivialValue(2, 1), // [result, input, input]
    ovis::vm::instructions::PushTrivialConstant(1), // [result, input, input, 1.0]
    ovis::vm::instructions::PushTrivialConstant(4), // [result, input, input, 1.0, &IsGreater]
    ovis::vm::instructions::CallNativeFunction(2),  // [result, input, IsGreater_result]
    ovis::vm::instructions::JumpIfFalse(16),        // [result, input]
    ovis::vm::instructions::Push(2),                // [result, input, void, void]
    ovis::vm::instructions::CopyTrivialValue(2, 0), // [result, input, result, void]
    ovis::vm::instructions::CopyTrivialValue(3, 1), // [result, input, result, input]
    ovis::vm::instructions::PushTrivialConstant(3), // [result, input, result, input, &Multiply]
    ovis::vm::instructions::CallNativeFunction(2),  // [result, input, Multiply_result]
    ovis::vm::instructions::CopyTrivialValue(0, 2), // [result, input, Multiply_result]
    ovis::vm::instructions::PopTrivial(1),          // [result, input]
    ovis::vm::instructions::Push(1),                // [result, input, void]
    ovis::vm::instructions::CopyTrivialValue(2, 1), // [result, input, input]
    ovis::vm::instructions::PushTrivialConstant(1), // [result, input, input, 1.0]
    ovis::vm::instructions::PushTrivialConstant(2), // [result, input, input, 1.0, &Subtract]
    ovis::vm::instructions::CallNativeFunction(2),  // [result, input, Subtract_result]
    ovis::vm::instructions::CopyTrivialValue(1, 2), // [result, input, Subtract_result]
    ovis::vm::instructions::PopTrivial(1),          // [result, input]
    ovis::vm::instructions::Jump(-20),              // [result, input]
  };

  for (auto _ : state) {
    benchmark::DoNotOptimize(context.Execute(instructions, constants));
    context.PopAll();
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

static void BM_CalculateFactorialLuaFunctionCalls(benchmark::State& state) {
  sol::state lua;
  lua["greater"] = [](double n1, double n2) { return n1 > n2; };
  lua["subtract"] = [](double n1, double n2) { return n1 - n2; };
  lua["multiply"] = [](double n1, double n2) { return n1 * n2; };
  lua.script(factorial_lua_function_calls);
  const double n = state.range(0);

  for (auto _ : state) {
    benchmark::DoNotOptimize(lua["factorial"](n));
  }
}

static void BM_CalculateFactorialVMImproved(benchmark::State& state) {
  const double n = state.range(0);
  using namespace ovis::vm;

  ovis::vm::ExecutionContext context;
  std::array<ValueStorage, 2> constants = {
    n,
    1.0,
  };

  const auto instructions = std::array {
    instructions::PushTrivialConstant(1),   // [result]
    instructions::PushTrivialConstant(0),   // [result, input]
    instructions::PushTrivialConstant(1),   // [result, input, 1]
    instructions::Push(1),                  // [result, input, 1, IsGreater_result]
    instructions::IsNumberGreater(3, 1, 2), // [result, input, 1, IsGreater_result]
    instructions::JumpIfFalse(4),           // [result, input, 1]
    instructions::MultiplyNumbers(0, 0, 1), // [result, input, 1]
    instructions::SubtractNumbers(1, 1, 2), // [result, input, 1]
    instructions::Jump(-5),                 // [result, input, 1]
  };

  for (auto _ : state) {
    benchmark::DoNotOptimize(context.Execute(instructions, constants));
    context.PopAll();
  }
}

// BENCHMARK(BM_ParseFactorialFunctionVM);
BENCHMARK(BM_ParseFactorialFunctionLua);
BENCHMARK(BM_CalculateFactorialCPP)->Range(1, 18);
BENCHMARK(BM_CalculateFactorialVM)->Range(1, 18);
BENCHMARK(BM_CalculateFactorialVMImproved)->Range(1, 18);
BENCHMARK(BM_CalculateFactorialLua)->Range(1, 18);
BENCHMARK(BM_CalculateFactorialLuaFunctionCalls)->Range(1, 18);

BENCHMARK_MAIN();
