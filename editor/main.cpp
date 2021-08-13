#include "editor_asset_library.hpp"
#include "editor_window.hpp"
#include "global.hpp"
#include "imgui_extensions/input_json.hpp"
#include "imgui_extensions/input_math.hpp"
#include "windows/log_window.hpp"
#include <cstring>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/application/application.hpp>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>

extern "C" {

int EMSCRIPTEN_KEEPALIVE Ovis_GetAssetCount() {
  ovis::GetApplicationAssetLibrary();
}

}
#endif

// Usage: ovis-editor backend_url project_id authentication_token
int main(int argc, char* argv[]) {
  using namespace ovis;
  using namespace ovis::editor;
  try {
    // Log::AddListener(ConsoleLogger);
    Log::AddListener([](LogLevel level, const std::string& text) {
      emscripten::val::global("window").call<void>("ovis_log", emscripten::val(GetLogLevelString(level)),
                                                   emscripten::val(text.c_str()));
    });

    ImGui::SetCustomJsonFunction("core#/$defs/vector2", &ImGui::InputVector2);
    ImGui::SetCustomJsonFunction("core#/$defs/vector3", &ImGui::InputVector3);
    ImGui::SetCustomJsonFunction("core#/$defs/vector4", &ImGui::InputVector4);
    ImGui::SetCustomJsonFunction("core#/$defs/color", &ImGui::InputColor);

    if (argc != 4) {
      LogE("Invalid number of arguments to editor");
      return -1;
    }

    backend_url = argv[1];
    user_name = argv[2];
    game_name = argv[3];

    Init();
#if OVIS_EMSCRIPTEN
    SetEngineAssetsDirectory("/ovis_assets");
    CreateApplicationAssetLibrary<EditorAssetLibrary>("/assets/");
#else
    SetEngineAssetsDirectory(argv[1]);
    CreateApplicationAssetLibrary<EditorAssetLibrary>(argv[2]);
#endif
    // LoadModule<BaseModule>();
    // LoadModule<Rendering2DModule>();
    // LoadModule<Physics2DModule>();
    // LoadModule<EditorModule>();

    // EditorWindow editor_window;

    Run();

    LogI("Quitting editor...");

    Quit();
  } catch (const std::exception& error) {
    LogE("An unhandled exception occured: {}", error.what());
  } catch (...) {
    LogE("An unhandled exception occured");
  }
  return 0;
}
