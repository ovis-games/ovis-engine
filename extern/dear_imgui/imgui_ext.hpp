#include <string>

#include <imgui.h>

namespace ImGui {

inline IMGUI_API bool InputTextAsString(const char* label, std::string* text,
                                        ImGuiInputTextFlags flags = 0) {}

}  // namespace ImGui
