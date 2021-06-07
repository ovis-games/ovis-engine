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

// Usage: ovis-editor backend_url project_id authentication_token
int main(int argc, char* argv[]) {
  using namespace ovis;
  using namespace ovis::editor;
  try {
    Log::AddListener(ConsoleLogger);
    Log::AddListener([](LogLevel, const std::string& text) { LogWindow::log_history.push_back(text); });

    ImGui::SetCustomJsonFunction("core#/$defs/vector2", &ImGui::InputVector2);
    ImGui::SetCustomJsonFunction("core#/$defs/vector3", &ImGui::InputVector3);
    ImGui::SetCustomJsonFunction("core#/$defs/vector4", &ImGui::InputVector4);
    ImGui::SetCustomJsonFunction("core#/$defs/color", &ImGui::InputColor);

    if (argc != 4) {
      LogE("Invalid number of arguments to editor");
      return -1;
    }

    backend_url = argv[1];
    project_id = argv[2];
    authentication_token = argv[3];

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

    EditorWindow editor_window;

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
