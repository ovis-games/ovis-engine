#include "editor_viewport.hpp"

#if OVIS_EMSCRIPTEN
#include <emscripten.h>

namespace ovis {
namespace editor {

namespace {

}

extern "C" {

EditorViewport* EMSCRIPTEN_KEEPALIVE CreateEditorViewport(const char* canvas_id) {
  return new EditorViewport(); 
}

}

EditorViewport::EditorViewport() : Window(WindowDescription{}) {
  ovis::LogD("Editor viewport created!");
}

}
}

#endif
