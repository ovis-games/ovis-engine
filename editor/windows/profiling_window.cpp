#include "profiling_window.hpp"
#include "imgui.h"

#include <ovis/utils/profiling.hpp>

namespace ovis {
namespace editor {

ProfilingWindow::ProfilingWindow() : ImGuiWindow("Profiling") {
  UpdateAfter("Dockspace Window");
  UpdateBefore("Overlay");
}

void ProfilingWindow::DrawContent() {
  std::array<float, 50> float_values;
  std::array<double, 50> double_values;
  for (const auto& profiler : ovis::ProfilingLog::default_log()->profilers()) {
    size_t values_count;
    profiler->ExtractLastMeasurements(double_values.data(), double_values.size(), &values_count);
    for (size_t i = 0; i < values_count; ++i) {
      float_values[i] = double_values[i];
    }
    ImGui::PlotLines(profiler->id().c_str(), float_values.data(), values_count);
  }
}

}
}
