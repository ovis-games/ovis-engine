#pragma once

#include <optional>
#include <string>

namespace ovis {
namespace editor {

bool ClipboardContainsData(std::string_view type = "text/plain");
std::optional<std::string_view> GetClipboardData(std::string_view type = "text/plain");
void SetClipboardData(std::string_view value, std::string_view type = "text/plain");

}  // namespace editor
}  // namespace ovis
