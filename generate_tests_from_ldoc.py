import sys
import re

HEADER = """
#include <catch2/catch.hpp>

#include <ovis/engine/engine.hpp>
#include <ovis/engine/lua.hpp>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ ":" S2(__LINE__)

namespace {

std::string last_error;

void SetupEnvironment() {
  static bool environment_setup = false;
  if (!environment_setup) {
    ovis::Lua::SetupEnvironment();
    ovis::Lua::on_error.Subscribe([](const std::string& error_message) {
      assert(error_message.length() > 0);
      last_error = error_message;
      INFO(error_message);
    });
    environment_setup = true;
  }
}

std::string FormatCode(const std::string& code, bool add_line_numbers = false) {
  size_t lines = 1;
  for (char c : code) {
    if (c == '\\n') {
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
    if (c == '\\n') {
      result.push_back('\\n');
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

#define LUA_SECTION_STRING(name, code)                                                      \\
  do {                                                                                      \\
    SECTION(name) {                                                                         \\
    last_error = "";                                                                        \\
    sol::protected_function_result result = ovis::Lua::state.do_string(code, "");           \\
    const bool valid_lua = result.valid();                                                  \\
    if (!result.valid() && last_error.length() == 0) {                                      \\
      last_error = result;                                                                  \\
    }                                                                                       \\
    INFO(FormatCode(code, true) + "\\n" + last_error);                                      \\
    CHECK(valid_lua);                                                                       \\
    }                                                                                       \\
  } while (false)


"""

PATTERN_WHITESPACES = '\s*'
PATTERN_SHORT_DESCRIPTION = PATTERN_WHITESPACES + '///' + PATTERN_WHITESPACES + '(.*)'
PATTERN_COMMENTED_LINE_START = PATTERN_WHITESPACES + '//\s*'
PATTERN_COMMENTED_LINE_WITHOUT_TAG = PATTERN_COMMENTED_LINE_START + '([^@\s]{1}.*)'
PATTERN_USAGE = PATTERN_COMMENTED_LINE_START + '@usage\s*(.*)'

def start_test_case(output, modulename, typename):
  output.write(f'TEST_CASE("{typename} is properly bound to Lua", "[ovis][{modulename}][{typename}]") {{\nSetupEnvironment();\n\n')

def end_test_case(output):
  output.write('}\n')

def write_test(output, description, usage):
  output.write(f'LUA_SECTION_STRING("{description}",\n{usage});\n\n')

def main():
  with open(sys.argv[2], 'w') as output:
    output.write(HEADER)

    with open(sys.argv[1], 'r') as input:

      modulename = ""
      typename = ""
      function = ""
      description = ""
      module_setup = ""
      usage = ""

      for line in input.readlines():

        match = re.search(PATTERN_COMMENTED_LINE_START + "@module (.*)", line)
        if match:
          modulename = match[1]
          start_test_case(output, modulename, "free functions")

        match = re.search(PATTERN_COMMENTED_LINE_START + "@function (.*)", line)
        if match:
          function = match[1]

        match = re.search(PATTERN_COMMENTED_LINE_START + "@type (.*)", line)
        if match:
          #if len(typename) > 0 or len(function) > 0:
          end_test_case(output)
          typename = match[1]
          start_test_case(output, modulename, typename)
        
        match = re.search(PATTERN_SHORT_DESCRIPTION, line)
        if match:
          description = match[1]

        if len(usage) > 0:
          usage = usage + '\\n"\n'
          match = re.search(PATTERN_COMMENTED_LINE_WITHOUT_TAG, line)
          if match:
            usage = usage + '"' + match[1]
          else:
            if typename != "" or function != "":
              write_test(output, description, module_setup + usage)
            else:
              module_setup = usage
            usage = ""

        match = re.search(PATTERN_USAGE, line)
        if match:
          usage = '"' + match[1]

      if len(typename) > 0:
        end_test_case(output)

      # for match in re.finditer(PATTERN_DOC_BLOCK, content):
      #   print(match[0])
      #   print(match.group('short_desc'))
      #   print()
      #   print(match.group('usage'))
      #   output.write('LUA_SECTION("')
      #   output.write(match.group('short_desc'))
      #   output.write(', "\n')
      #   output.write(match.group('usage'))
      #   output.write(')')




if __name__ == "__main__":
    main()