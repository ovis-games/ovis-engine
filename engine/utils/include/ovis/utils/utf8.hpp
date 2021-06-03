#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

#include <ovis/utils/log.hpp>

namespace ovis {

inline std::string to_lower(const std::string& string) {
  std::string lowercase_string;
  lowercase_string.resize(string.size());
  std::transform(string.begin(), string.end(), lowercase_string.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return lowercase_string;
}

inline std::string replace_all(std::string_view string, std::string_view source, std::string_view target) {
  std::string result;

  while (true) {
    auto next_occurence = string.find(source);
    result += string.substr(0, next_occurence);
    if (next_occurence == std::string_view::npos) {
      break;
    }
    result += target;
    string = string.substr(next_occurence + source.length());
  }

  return result;
}

}  // namespace ovis
