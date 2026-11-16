// Checked signed Q16.16 arithmetic and 2D AABB geometry.
// This is conventional fixed-point arithmetic, not a novelty claim.
#pragma once
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace vacd {
using fixed = std::int32_t;
constexpr std::int64_t scale = 65536;

inline fixed checked(std::int64_t v) {
  if (v < std::numeric_limits<fixed>::min() ||
      v > std::numeric_limits<fixed>::max())
    throw std::overflow_error("Q16.16 result outside signed 32-bit range");
  return static_cast<fixed>(v);
}
inline fixed from_integer(std::int32_t v) {
  return checked(static_cast<std::int64_t>(v) * scale);
}
inline std::int32_t to_integer(fixed v) { return v / scale; }
inline fixed add(fixed a, fixed b) {
  return checked(static_cast<std::int64_t>(a) + b);
}
inline fixed sub(fixed a, fixed b) {
  return checked(static_cast<std::int64_t>(a) - b);
}
inline fixed mul(fixed a, fixed b) {
  // 32-bit x 32-bit fits in signed 64 bits. Quotient truncates toward zero.
  return checked((static_cast<std::int64_t>(a) * b) / scale);
}

struct AABB { fixed minX, minY, maxX, maxY; };
struct Entity {
  fixed posX, posY;
  fixed velX, velY;
  fixed halfWidth, halfHeight;
  const char* name;

  AABB bounds() const {
    if (halfWidth < 0 || halfHeight < 0)
      throw std::invalid_argument("AABB half extents must be nonnegative");
    return {sub(posX, halfWidth), sub(posY, halfHeight),
            add(posX, halfWidth), add(posY, halfHeight)};
  }
  void update() {
    // Check both coordinates before mutating, so a rejected step is atomic.
    const fixed nextX = add(posX, velX);
    const fixed nextY = add(posY, velY);
    posX = nextX;
    posY = nextY;
  }
};

// Touching edges do not count as overlap (matches original demo semantics).
inline bool collides(const AABB& a, const AABB& b) {
  return a.minX < b.maxX && a.maxX > b.minX &&
         a.minY < b.maxY && a.maxY > b.minY;
}
} // namespace vacd
