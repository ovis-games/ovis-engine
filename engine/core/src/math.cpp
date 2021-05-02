#include <SDL2/SDL_assert.h>

#include <ovis/utils/range.hpp>
#include <ovis/core/intersection.hpp>
#include <ovis/core/math.hpp>

namespace ovis {

bool IsConvex(std::span<const Vector2> vertices) {
  // https://stackoverflow.com/questions/471962/how-do-i-efficiently-determine-if-a-polygon-is-convex-non-convex-or-complex
  SDL_assert(vertices.size() >= 3);

  Vector2 prev_prev = vertices[vertices.size() - 2];
  Vector2 prev = vertices[vertices.size() - 1];
  Vector2 old_delta = prev - prev_prev;

  std::optional<bool> signbit;
  for (Vector2 current : vertices) {
    const Vector2 delta = current - prev;
    const float perp_dot = PerpDot(delta, old_delta);

    if (!signbit.has_value()) {
      signbit = std::signbit(perp_dot);
    } else if (*signbit != std::signbit(perp_dot)) {
      return false;
    }

    prev_prev = prev;
    prev = current;
    old_delta = delta;
  }

  return true;
}

size_t GetLineStripInsertPosition(std::span<const Vector2> strip, Vector2 new_position) {
  SDL_assert(strip.size() >= 2);

  float shortest_distance_squared = SquaredDistance(strip.front(), new_position);
  size_t best_position = 0;

  {
    const float squared_distance_to_last = SquaredDistance(strip.back(), new_position);
    if (squared_distance_to_last < shortest_distance_squared) {
      shortest_distance_squared = squared_distance_to_last;
      best_position = strip.size();
    }
  }

  for (size_t i = 1; i < strip.size(); ++i) {
    const LineSegment2D edge = {strip[i - 1], strip[i]};
    const Vector2 closest_point_on_edge = ComputeClosestPointOnLineSegment(edge, new_position);
    const float squared_distance = SquaredDistance(closest_point_on_edge, new_position);
    if (squared_distance < shortest_distance_squared * (1.0f - EPSILON)) {
      best_position = i;
      shortest_distance_squared = squared_distance;
    }
  }

  return best_position;
}

size_t GetLineLoopInsertPosition(std::span<const Vector2> loop, Vector2 new_position) {
  SDL_assert(loop.size() >= 3);

  float shortest_distance_squared = std::numeric_limits<float>::infinity();
  size_t best_position;
  Vector2 previous_vertex = loop.back();
  for (const auto& current_vertex : IndexRange(loop)) {
    const LineSegment2D edge = {previous_vertex, current_vertex.value()};
    const Vector2 closest_point_on_edge = ComputeClosestPointOnLineSegment(edge, new_position);
    const float squared_distance = SquaredDistance(closest_point_on_edge, new_position);
    if (squared_distance < shortest_distance_squared) {
      best_position = current_vertex.index();
      shortest_distance_squared = squared_distance;
    }
  }

  return best_position;
}

}  // namespace ovis
