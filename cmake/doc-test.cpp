#include <catch2/catch.hpp>
#include <sol/sol.hpp>
#include <ovis/core/lua.hpp>
#include <ovis/@TARGET_ADD_LDOC_TEST_MODULE@/@TARGET_ADD_LDOC_TEST_MODULE@_module.hpp>

@ if module then
@   if module.tags.testinginclude then
@     for i = 1, #module.tags.testinginclude do
#include $(module.tags.testinginclude[i])
@     end
@   end
@ end

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ ":" S2(__LINE__)

namespace {

std::string last_error;

void SetupEnvironment() {
  // static bool environment_setup = false;
  // if (!environment_setup) {
  //   ovis::Lua::SetupEnvironment();
  //   ovis::Lua::on_error.Subscribe([](const std::string& error_message) {
  //     assert(error_message.length() > 0);
  //     last_error = error_message;
  //     INFO(error_message);
  //   });
  //   environment_setup = true;
  // }
  ovis::Load@MODULE_CAPITALIZED@Module();
}

std::string FormatCode(const std::string& code, bool add_line_numbers = false) {
  size_t lines = 1;
  for (char c : code) {
    if (c == '\n') {
      ++lines;
    }
  }
  const size_t line_number_width = std::to_string(lines).length();

  std::string result;
  result.reserve(code.length() + lines * (line_number_width + 3) + 1);

  auto add_line_number = [&result,line_number_width](int line_number) {
      result.push_back('[');
      const std::string line_number_string = std::to_string(line_number);
      result.insert(result.end(), line_number_width - line_number_string.size(), ' ');
      result += line_number_string;
      result.push_back(']');
      result.push_back(' ');
  };

  size_t current_line = 1;
  if (add_line_numbers) {
    add_line_number(current_line);
  }
  result.push_back(' ');
  for (char c : code) {
    if (c == '\n') {
      result.push_back('\n');
      current_line += 1;
      if (add_line_numbers) {
        add_line_number(current_line);
      }
    } else {
      result.push_back(c);
    }
  }

  return result;
}

}

#define CHECK_LUA(code)                                                                     \
  do {                                                                                      \
    last_error = "";                                                                        \
    sol::protected_function_result result = ovis::lua.do_string(code, "");                  \
    const bool valid_lua = result.valid();                                                  \
    if (!result.valid() && last_error.length() == 0) {                                      \
      last_error = result;                                                                  \
    }                                                                                       \
    INFO(FormatCode(code, true) + "\n" + last_error);                                       \
    CHECK(valid_lua);                                                                       \
  } while (false)

@ local iter = ldoc.modules.iter

@ if module then
@ local tags = '[' .. module.name:gsub('%.', '][') .. ']'
TEST_CASE("$(module.summary)", "$(tags)" ) {
  SetupEnvironment();
@   if module.tags.cppsetup then
@     for i = 1, #module.tags.cppsetup do
  $(module.tags.cppsetup[i])
@     end
@   end
@   local module_setup = ''
@   if module.usage then
@     module_setup = module.usage[1]
@     for i = 2, #module.usage do
@       usage = module_setup .. '\n\n' .. module.usage[i]
@       usage = usage:gsub('\n', '\\n')
@       usage = usage:gsub('"', '\\"')
  SECTION("$(module.name)") {
    CHECK_LUA("$(usage)");
  }
@     end
@   end
@   for kind, items in module.kinds() do
@     for item in items() do
@       if item.usage then
@         for i = 1, #item.usage do
@           usage = module_setup .. '\n\n' .. item.usage[i]
@           usage = usage:gsub('\n', '\\n')
@           usage = usage:gsub('"', '\\"')
  SECTION("$(item.name)") {
    CHECK_LUA("$(usage)");
  }
@         end
@       end
@     end
@   end
}
@ end
