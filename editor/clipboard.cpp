#include "clipboard.hpp"

#include "editor_window.hpp"
#include <iterator>
#include <vector>

namespace {

struct ClipboardData {
  std::string type;
  std::string content;

  ClipboardData(std::string_view type, std::string_view content) : type(type), content(content) {}
};
static std::vector<ClipboardData> clipboard_data;

auto FindClipboardEntry(std::string_view type) {
  return std::find_if(clipboard_data.begin(), clipboard_data.end(), [type](const auto& clipboard_entry) {
        return clipboard_entry.type == type;
      });
}

}  // namespace

#if OVIS_EMSCRIPTEN
#include <emscripten.h>

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
    return clipboard_data[index].type.c_str();
  } else {
    return nullptr;
  }
}

const char* EMSCRIPTEN_KEEPALIVE OvisClipboard_GetCopiedData(int index) {
  if (index < clipboard_data.size()) {
    return clipboard_data[index].content.c_str();
  } else {
    return nullptr;
  }
}

void EMSCRIPTEN_KEEPALIVE OvisClipboard_BeginPaste() {
  clipboard_data.clear();
}

void EMSCRIPTEN_KEEPALIVE OvisClipboard_Paste(const char* type, const char* data) {
  ovis::editor::SetClipboardData(type, data);
}

void EMSCRIPTEN_KEEPALIVE OvisClipboard_EndPaste() {
  // TODO: Callback here?
}

}
#endif 

namespace ovis {
namespace editor {

bool ClipboardContainsData(std::string_view type) {
#if OVIS_EMSCRIPEN
  return FindClipboardEntry(type) == clipboard_data.end();
#else
  return type == "text/plain" && SDL_HasClipboardText();
#endif
}

std::optional<std::string_view> GetClipboardData(std::string_view type) {
#if OVIS_EMSCRIPTEN
  const auto it = FindClipboardEntry(type);

  if (it != clipboard_data.end()) {
    return it->content;
  } else {
    return {};
  }
#else
  if (type != "text/plain") {
    return {};
  }

  char* clipboard_text = SDL_GetClipboardText();
  if (clipboard_text != nullptr) {
    clipboard_data.clear();
    clipboard_data.emplace_back("text/plain", clipboard_text);
    SDL_free(clipboard_text);
    return clipboard_data.back().content;
  } else {
    return {};
  }
#endif
}

void SetClipboardData(std::string_view value, std::string_view type) {
  const auto it = FindClipboardEntry(type);

  if (it != clipboard_data.end()) {
    it->content = value;
#if !OVIS_EMSCRIPTEN
    if (type == "text/plain") {
      SDL_SetClipboardText(it->content.c_str());
    }
#endif
  } else {
    clipboard_data.emplace_back(type, value);
#if !OVIS_EMSCRIPTEN
    if (type == "text/plain") {
      SDL_SetClipboardText(clipboard_data.back().content.c_str());
    }
#endif
  }
}

}  // namespace editor
}  // namespace ovis
