#include "clipboard.hpp"

#include "editor_window.hpp"
#include <iterator>
#include <unordered_map>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>

namespace {

static std::unordered_map<std::string, std::string> clipboard_data;

}  // namespace

extern "C" {

int EMSCRIPTEN_KEEPALIVE OvisClipboard_Copy() {
  clipboard_data.clear();
  // Do a "0" update so imgui is able to copy the string
  ovis::editor::EditorWindow::instance()->Update(std::chrono::microseconds(0));
  return clipboard_data.size();
}

int EMSCRIPTEN_KEEPALIVE OvisClipboard_Cut() {
  clipboard_data.clear();
  // Do a "0" update so imgui is able to copy the string
  ovis::editor::EditorWindow::instance()->Update(std::chrono::microseconds(0));
  return clipboard_data.size();
}

const char* EMSCRIPTEN_KEEPALIVE OvisClipboard_GetCopiedDataFormat(int index) {
  if (index < clipboard_data.size()) {
    return std::next(clipboard_data.begin(), index)->first.c_str();
  } else {
    return nullptr;
  }
}

const char* EMSCRIPTEN_KEEPALIVE OvisClipboard_GetCopiedData(int index) {
  if (index < clipboard_data.size()) {
    return std::next(clipboard_data.begin(), index)->second.c_str();
  } else {
    return nullptr;
  }
}

void EMSCRIPTEN_KEEPALIVE OvisClipboard_BeginPaste() {
  clipboard_data.clear();
}

void EMSCRIPTEN_KEEPALIVE OvisClipboard_Paste(const char* type, const char* data) {
  clipboard_data.insert(std::make_pair(type, data));
}

void EMSCRIPTEN_KEEPALIVE OvisClipboard_EndPaste() {
  // TODO: Callback here?
}

}
#endif 

namespace ovis {
namespace editor {

bool ClipboardContainsData(const std::string& type) {
#if OVIS_EMSCRIPTEN
  return clipboard_data.count(type) > 0;
#else
  return type == "text/plain" && SDL_HasClipboardText();
#endif
}

std::optional<std::string> GetClipboardData(const std::string& type) {
#if OVIS_EMSCRIPTEN
  const auto it = clipboard_data.find(type);
  if (it != clipboard_data.end()) {
    return it->second;
  } else {
    return {};
  }
#else
  if (type != "text/plain") {
    return {};
  }

  char* clipboard_text = SDL_GetClipboardText();
  if (clipboard_text != nullptr) {
    const std::string clipboard_data = clipboard_text;
    SDL_free(clipboard_text);
    return clipboard_data;
  } else {
    return {};
  }
#endif
}

void SetClipboardData(const std::string& value, const std::string& type) {
#if OVIS_EMSCRIPTEN
  clipboard_data[type] = value;
#else
  if (type != "text/plain") {
    return;
  }

  SDL_SetClipboardText(value.c_str());
#endif
}

}  // namespace editor
}  // namespace ovis
