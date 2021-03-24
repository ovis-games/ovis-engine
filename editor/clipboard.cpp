#include "clipboard.hpp"

#include "editor_window.hpp"
#include <iterator>
#include <unordered_map>

#include <emscripten.h>

namespace {

static std::unordered_map<std::string, std::string> clipboard_data;

}  // namespace

extern "C" {

int EMSCRIPTEN_KEEPALIVE OvisClipboard_Copy() {
  clipboard_data.clear();
  // ovis::editor::EditorWindow::instance()->ComputeImGuiFrame(); // TODO: this needs to change
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

namespace ovis {
namespace editor {

bool ClipboardContainsData(const std::string& type) {
  return clipboard_data.count(type) > 0;
}

std::optional<std::string> GetClipboardData(const std::string& type) {
  const auto it = clipboard_data.find(type);
  if (it != clipboard_data.end()) {
    return it->second;
  } else {
    return {};
  }
}

void SetClipboardData(const std::string& value, const std::string& type) {
  clipboard_data[type] = value;
}

}  // namespace editor
}  // namespace ovis