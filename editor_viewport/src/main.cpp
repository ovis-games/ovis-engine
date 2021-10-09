#include <cstring>

#if OVIS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#endif

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/rendering/clear_pass.hpp>
#include <ovis/application/application.hpp>
#include <ovis/application/window.hpp>

#include <ovis/editor_viewport/editor_viewport.hpp>

#if OVIS_EMSCRIPTEN
std::vector<std::string> GetObjectKeys(const emscripten::val& object) {
  emscripten::val global_object = emscripten::val::global("Object");
  emscripten::val keys = global_object.call<emscripten::val>("keys", object);
  return emscripten::vecFromJSArray<std::string>(keys);
}

extern "C" {
using namespace ovis;
using namespace ovis::editor;

class AssetProvider : public AssetLibrary {
 public:
  AssetProvider(const emscripten::val& assets) : assets_(assets) {
    for (const auto& asset_id : GetObjectKeys(assets)) {
      LogD(asset_id);
    }
  }

  bool Contains(std::string_view asset_id) const override final {
    const auto assets = GetAssets();
    return std::find_if(assets.begin(), assets.end(), [asset_id](const std::string& id) { return id == asset_id; }) !=
           assets.end();
  }

  std::vector<std::string> GetAssets() const override final {
    return GetObjectKeys(assets_);
  }

  std::string GetAssetType(std::string_view asset_id) const override final {
    LogD("GetAssetType {}", asset_id);
    if (Contains(asset_id)) {
      const std::string asset_id_string(asset_id);
      return assets_[asset_id_string]["type"].as<std::string>();
    } else {
      return "";
    }
  }

  std::vector<std::string> GetAssetFileTypes(std::string_view asset_id) const override final {
    LogD("GetAssetFileTypes {}", asset_id);
    if (!Contains(asset_id)) {
      return {};
    }

    const std::string asset_id_string(asset_id);
    return GetObjectKeys(assets_[asset_id_string]["files"]);
  }

  std::optional<std::string> LoadAssetTextFile(std::string_view asset_id,
                                               std::string_view filename) const override final {
    
    LogD("LoadAssetTextFile {} {}", asset_id, filename);
    const std::string asset_id_string(asset_id);
    const std::string filename_string(filename);
    emscripten::val blob = assets_[asset_id_string]["files"][filename_string]["content"];
    emscripten::val promise = blob.call<emscripten::val>("text");
    emscripten::val text = promise.await();

    LogD("Type: {}", text.typeOf().as<std::string>());

    return text.as<std::string>();
  }
  std::optional<Blob> LoadAssetBinaryFile(std::string_view asset_id, std::string_view filename) const override final {
    LogD("LoadAssetBinaryFile {} {}", asset_id, filename);
    return {};
  }
  std::vector<std::string> GetAssetsWithType(std::string_view type) const override final {
    LogD("GetAssetsWithType {}", type);
    return {};
  }

 private:
  emscripten::val assets_;
};

void EMSCRIPTEN_KEEPALIVE OvisEditorViewport_Quit() {
  SDL_Event sdl_event;
  sdl_event.type = SDL_QUIT;
  SDL_PushEvent(&sdl_event);
}

int EMSCRIPTEN_KEEPALIVE OvisEditorViewport_GetRegisteredSceneObjectComponentCount() {
  return ovis::SceneObjectComponent::GetRegisteredFactoriesCount();
}

const char* EMSCRIPTEN_KEEPALIVE OvisEditorViewport_GetRegisteredSceneObjectComponentId(int index) {
  if (index >= OvisEditorViewport_GetRegisteredSceneObjectComponentCount()) {
    return nullptr;
  } else {
    return std::next(ovis::SceneObjectComponent::registered_ids().begin(), index)->c_str();
  }
}

}

void SetEventCallback(emscripten::val callback) {
  EditorViewport::instance()->SetEventCallback(callback);
}

emscripten::val log_callback = emscripten::val::undefined();
void SetLogCallback(emscripten::val callback) {
  log_callback = callback;
}

void SetAssets(const emscripten::val& assets) {
  ovis::CreateApplicationAssetLibrary<AssetProvider>(assets);
}

EMSCRIPTEN_BINDINGS(editor_viewport_module) {
  function("viewportSetEventCallback", &SetEventCallback);
  function("viewportSetLogCallback", &SetLogCallback);
  function("viewportSetAssets", &SetAssets);
}

#endif

// Usage: ovis-player backend_url project_id
int main(int argc, char* argv[]) {
  using namespace ovis;
  using namespace ovis::editor;

  // Log::AddListener(ConsoleLogger);
  Log::AddListener([](LogLevel level, const std::string& text) {
    if (log_callback.typeOf() == emscripten::val("function")) {
      log_callback(emscripten::val(static_cast<int>(level)), emscripten::val(text));
    }
  });

  Init();

#if OVIS_EMSCRIPTEN
  SetEngineAssetsDirectory("/ovis_assets");
#endif

  EditorViewport viewport;

  Run();

  Quit();
}
