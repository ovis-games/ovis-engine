#include <SDL2/SDL_assert.h>

#include <ovis/core/math.hpp>

namespace ovis {

bool IsConvex(std::span<Vector2> vertices) {
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

}  // namespace ovis
