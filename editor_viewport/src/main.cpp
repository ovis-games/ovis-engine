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
  AssetProvider(const emscripten::val& asset_provider) : asset_provider_(asset_provider) {
    LogD("Assets:");
    for (const auto& asset_id : GetAssets()) {
      LogD(asset_id);
    }
  }

  bool Contains(std::string_view asset_id) const override final {
    const auto assets = GetAssets();
    return std::find_if(assets.begin(), assets.end(), [asset_id](const std::string& id) { return id == asset_id; }) !=
           assets.end();
  }

  std::vector<std::string> GetAssets() const override final {
    return GetObjectKeys(asset_provider_["assets"]);
  }

  Result<std::string> GetAssetType(std::string_view asset_id) const override final {
    if (!Contains(asset_id)) {
      return Error("Asset {} does not exist", asset_id);
    }

    const std::string asset_id_string(asset_id);
    return asset_provider_["assets"][asset_id_string]["type"].as<std::string>();
  }

  std::vector<std::string> GetAssetFileTypes(std::string_view asset_id) const override final {
    if (!Contains(asset_id)) {
      return {};
    }

    const std::string asset_id_string(asset_id);
    return GetObjectKeys(asset_provider_["assets"][asset_id_string]["files"]);
  }

  Result<std::string> LoadAssetTextFile(std::string_view asset_id,
                                               std::string_view filename) const override final {
    LogD("LoadAssetTextFile {} {}", asset_id, filename);
    const std::string asset_id_string(asset_id);
    const std::string filename_string(filename);
    emscripten::val content = asset_provider_["assets"][asset_id_string]["files"][filename_string]["content"];
    if (const auto content_type = content.typeOf().as<std::string>(); content_type != "object") {
      return Error("Invalid asset content: {}", content_type);
    } else {
      return emscripten::val::global("JSON").call<std::string>("stringify", content);
    }
  }

  Result<Blob> LoadAssetBinaryFile(std::string_view asset_id, std::string_view filename) const override final {
    LogD("LoadAssetBinaryFile {} {}", asset_id, filename);
    return Error("Not implemented");
  }

  std::vector<std::string> GetAssetsWithType(std::string_view type) const override final {
    LogD("GetAssetsWithType {}", type);
    return {};
  }

 private:
  emscripten::val asset_provider_;
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

void SetAssetProvider(const emscripten::val& asset_provider) {
  ovis::CreateApplicationAssetLibrary<AssetProvider>(asset_provider);
}

EMSCRIPTEN_BINDINGS(editor_viewport_module) {
  function("viewportSetEventCallback", &SetEventCallback);
  function("viewportSetLogCallback", &SetLogCallback);
  function("viewportSetAssetProvider", &SetAssetProvider);
}

#endif

// Usage: ovis-player backend_url project_id
int main(int argc, char* argv[]) {
  using namespace ovis;
  using namespace ovis::editor;

  Log::AddListener(ConsoleLogger);
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
