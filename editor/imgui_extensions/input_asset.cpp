#include "input_asset.hpp"

#include <SDL2/SDL_assert.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <ovis/core/asset_library.hpp>
#include <ovis/utils/log.hpp>

namespace ImGui {

namespace {

int AssetWidgetInputCallback(ImGuiInputTextCallbackData* data) {
  return 0;
}

bool IsAssetValid(const std::string& asset_id, const std::string& asset_type) {
  ovis::AssetLibrary* asset_library = ovis::GetApplicationAssetLibrary();
  return asset_library->Contains(asset_id) && asset_library->GetAssetType(asset_id) == asset_type;
}

}  // namespace

bool InputAsset(const char* label, std::string* asset_id, const std::string& asset_type, int flags) {
  SDL_assert(asset_id != nullptr);

  bool asset_changed = false;

  std::string candidate_asset_id = *asset_id;
  if (ImGui::InputText(label, &candidate_asset_id,
                       ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue,
                       AssetWidgetInputCallback, 0)) {
    if (candidate_asset_id == "" && flags & ImGuiInputAssetFlags_AcceptEmpty) {
      *asset_id = "";
      asset_changed = true;
    } else if (IsAssetValid(candidate_asset_id, asset_type)) {
      *asset_id = candidate_asset_id;
      asset_changed = true;
    }
  }

  if (*asset_id != "" && !IsAssetValid(*asset_id, asset_type)) {
    *asset_id = "";
    asset_changed = true;
  }

  if (ImGui::BeginDragDropTarget()) {
    std::string dropped_asset_id;
    if (AcceptDragDropAsset(asset_type, &dropped_asset_id)) {
      *asset_id = dropped_asset_id;
      asset_changed = true;
    }
    ImGui::EndDragDropTarget();
  }

  return asset_changed;
}

bool AcceptDragDropAsset(const std::string& asset_type, std::string* asset_id) {
  const std::string payload_type = "asset<" + asset_type + '>';
  if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_type.c_str())) {
    const char* dropped_asset_id_ptr = static_cast<const char*>(payload->Data);
    const std::string dropped_asset_id(dropped_asset_id_ptr, dropped_asset_id_ptr + payload->DataSize);
    if (IsAssetValid(dropped_asset_id, asset_type)) {
      *asset_id = dropped_asset_id;
      return true;
    }
  }

  return false;
}

}  // namespace ImGui