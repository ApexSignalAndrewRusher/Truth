// Checked signed Q16.16 arithmetic and 2D AABB geometry.
// Contract adaptation inspired by QSOLKCB/VANILLAJS (not a formal refinement).
// Q16.16 is established fixed-point arithmetic, not a novelty claim.
#pragma once
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace vacd {
constexpr std::int64_t scale = 65536;

// Strong representation: callers must explicitly choose raw bits or whole units.
// No implicit conversion from an arbitrary int or double can change the scale.
class fixed {
  std::int32_t value_ = 0;
  explicit constexpr fixed(std::int32_t raw) : value_(raw) {}
public:
  fixed() = default;
  // Validate raw *integer* values before narrowing. Both signed and unsigned
  // 64-bit callers are supported; floats are rejected at compile time.
  template <typename Integer,
            typename = std::enable_if_t<std::is_integral<Integer>::value>>
  static constexpr fixed from_raw(Integer raw) {
    if constexpr (std::is_signed<Integer>::value) {
      const auto wide = static_cast<std::int64_t>(raw);
      if (wide < std::numeric_limits<std::int32_t>::min() ||
          wide > std::numeric_limits<std::int32_t>::max())
        throw std::overflow_error("raw Q16.16 value outside signed 32-bit range");
    } else {
      const auto wide = static_cast<std::uint64_t>(raw);
      if (wide > static_cast<std::uint64_t>(
            std::numeric_limits<std::int32_t>::max()))
        throw std::overflow_error("raw Q16.16 value outside signed 32-bit range");
    }
    return fixed(static_cast<std::int32_t>(raw));
  }
  constexpr std::int32_t raw() const { return value_; }
  friend constexpr bool operator==(fixed a, fixed b) { return a.value_ == b.value_; }
  friend constexpr bool operator!=(fixed a, fixed b) { return !(a == b); }
  friend constexpr bool operator<(fixed a, fixed b) { return a.value_ < b.value_; }
  friend constexpr bool operator>(fixed a, fixed b) { return b < a; }
  friend constexpr bool operator<=(fixed a, fixed b) { return !(b < a); }
  friend constexpr bool operator>=(fixed a, fixed b) { return !(a < b); }
};
inline fixed checked(std::int64_t v) {
  if (v < std::numeric_limits<std::int32_t>::min() ||
      v > std::numeric_limits<std::int32_t>::max())
    throw std::overflow_error("Q16.16 result outside signed 32-bit range");
  return fixed::from_raw(static_cast<std::int32_t>(v));
}
inline fixed from_integer(std::int64_t whole) {
  // Accept the wider type so no narrowing occurs before validation.
  if (whole < -32768 || whole > 32767)
    throw std::overflow_error("whole integer outside Q16.16 range");
  return checked(whole * scale);
}
inline std::int32_t to_integer(fixed v) { return v.raw() / scale; }
inline fixed add(fixed a, fixed b) {
  return checked(static_cast<std::int64_t>(a.raw()) + b.raw());
}
inline fixed sub(fixed a, fixed b) {
  return checked(static_cast<std::int64_t>(a.raw()) - b.raw());
}
inline fixed mul(fixed a, fixed b) {
  // 32-bit x 32-bit fits in signed 64 bits. Quotient truncates toward zero.
  return checked((static_cast<std::int64_t>(a.raw()) * b.raw()) / scale);
}

struct AABB {
  fixed minX, minY, maxX, maxY;
};
struct Entity {
  fixed posX, posY;
  fixed velX, velY;
  fixed halfWidth, halfHeight;
  const char* name;

  AABB bounds() const {
    const fixed zero = fixed::from_raw(0);
    if (halfWidth < zero || halfHeight < zero)
      throw std::invalid_argument("AABB half-extents must be nonnegative");
    return {sub(posX, halfWidth), sub(posY, halfHeight),
            add(posX, halfWidth), add(posY, halfHeight)};
  }
  void update() {
    // Validate both coordinates before mutation so overflow is atomic.
    const fixed nextX = add(posX, velX);
    const fixed nextY = add(posY, velY);
    posX = nextX;
    posY = nextY;
  }
};

// Inverted AABBs are rejected. Zero-area boxes do not collide.
// Touching edges do not count as overlap (original demo semantics).
inline bool collides(const AABB& a, const AABB& b) {
  if (a.minX > a.maxX || a.minY > a.maxY ||
      b.minX > b.maxX || b.minY > b.maxY)
    throw std::invalid_argument("AABB has inverted endpoints");
  if (a.minX == a.maxX || a.minY == a.maxY ||
      b.minX == b.maxX || b.minY == b.maxY)
    return false;
  return a.minX < b.maxX && a.maxX > b.minX &&
         a.minY < b.maxY && a.maxY > b.minY;
}
} // namespace vacd
