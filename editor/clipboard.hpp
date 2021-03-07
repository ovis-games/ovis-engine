#pragma once

#include <optional>
#include <string>

namespace ovis {
namespace editor {

bool ClipboardContainsData(const std::string& type = "text/plain");
std::optional<std::string> GetClipboardData(const std::string& type = "text/plain");
void SetClipboardData(const std::string& value, const std::string& type = "text/plain");

}  // namespace editor
}  // namespace ovis
