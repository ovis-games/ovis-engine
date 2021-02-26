#pragma once

#include <string>
#include <unordered_map>

#include <SDL2/SDL_scancode.h>

namespace ovis {

class Input {
 public:
  Input();

  inline void SetKeyState(SDL_Scancode scan_code, bool pressed) {
    key_states_[SCANCODE_MAPPING.at(scan_code)] = pressed;
  }
  bool GetKeyState(const std::string& key_code) const { return key_states_.at(key_code); }

 private:
  std::unordered_map<std::string, bool> key_states_;

  static const std::unordered_map<SDL_Scancode, std::string> SCANCODE_MAPPING;
};

Input* input();

}  // namespace ovis
