#include <imgui.h>

namespace ImGui {

constexpr int ImGuiAutoCompleteInputFlags_AllowNonPredefinedValues = 1;

bool AutoCompleteInput(const char* label, std::string* value, const std::string* predefined_values, int value_count, int* selected_index = nullptr, bool* is_active = nullptr, int flags = 0);

}

