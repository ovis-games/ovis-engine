#include <ovis/core/intersection.hpp>

namespace ovis {

std::optional<LineSegment2D> ClipLineSegment(const AxisAlignedBoundingBox2D& aabb, const LineSegment2D& line_segment) {
  // https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm

  enum class OutCode { INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };

  auto ComputeOutCode = [&](const Vector2& position) {
    Flags<OutCode> out_code = OutCode::INSIDE;
    if (position.x < aabb.min().x) {
      out_code |= OutCode::LEFT;
    } else if (position.x > aabb.max().x) {
      out_code |= OutCode::RIGHT;
    }
    if (position.y < aabb.min().y) {
      out_code |= OutCode::BOTTOM;
    } else if (position.y > aabb.max().y) {
      out_code |= OutCode::TOP;
    }
    return out_code;
  };

  Vector2 start = line_segment.start;
  Vector2 end = line_segment.end;
  Flags<OutCode> start_out_code = ComputeOutCode(start);
  Flags<OutCode> end_out_code = ComputeOutCode(end);

  do {
    if (start_out_code == OutCode::INSIDE && end_out_code == OutCode::INSIDE) {
      return LineSegment2D{start, end};
    } else if ((start_out_code & end_out_code).AnySet()) {
      return {};
    } else {
      Vector2 intermediate;
      const auto selected_out_code = start_out_code.AnySet() ? start_out_code : end_out_code;

      if (selected_out_code.IsSet(OutCode::TOP)) {
        intermediate.x = start.x + (end.x - start.x) * (aabb.max().y - start.y) / (end.y - start.y);
        intermediate.y = aabb.max().y;
      } else if (selected_out_code.IsSet(OutCode::BOTTOM)) {
        intermediate.x = start.x + (end.x - start.x) * (aabb.min().y - start.y) / (end.y - start.y);
        intermediate.y = aabb.min().y;
      } else if (selected_out_code.IsSet(OutCode::RIGHT)) {
        intermediate.x = aabb.max().x;
        intermediate.y = start.y + (end.y - start.y) * (aabb.max().x - start.x) / (end.x - start.x);
      } else if (selected_out_code.IsSet(OutCode::LEFT)) {
        intermediate.x = aabb.min().x;
        intermediate.y = start.y + (end.y - start.y) * (aabb.min().x - start.x) / (end.x - start.x);
      }

      if (selected_out_code == start_out_code) {
        start = intermediate;
        start_out_code = ComputeOutCode(start);
      } else {
        end = intermediate;
        end_out_code = ComputeOutCode(end);
      }
    }
  } while (true);
}

}  // namespace ovis
