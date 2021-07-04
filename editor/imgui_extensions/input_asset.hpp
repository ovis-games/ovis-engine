#pragma once

#include <string>

constexpr int ImGuiInputAssetFlags_AcceptEmpty = 1;

namespace ImGui {

bool InputAsset(const char* label, std::string* asset_id, const std::string& asset_type,
                int flags = ImGuiInputAssetFlags_AcceptEmpty);

bool AcceptDragDropAsset(const std::string& asset_type, std::string* asset_id);

}  // namespace ImGui
