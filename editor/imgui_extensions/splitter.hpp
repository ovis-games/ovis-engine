#include <imgui.h>

namespace ImGui {

bool HorizontalSplitter(const char* label, float* top_height, float* bottom_height, float top_weight = 1.0f,
                        float bottom_weight = 1.0f, float height = 0.0f, float top_min_height = 4.0f, float bottom_min_height = 4.0f,
                        float thickness = 4.0f);

}
