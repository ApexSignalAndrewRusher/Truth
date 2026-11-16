#include "../src/vacd_collision.h"
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <cstdlib>
#include <iostream>

void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
  }
}
template<class F> void expect_overflow(F fn) {
  bool thrown = false;
  try { fn(); } catch (const std::overflow_error&) { thrown = true; }
  require(thrown, "expected arithmetic overflow exception");
}
template<class F> void expect_invalid(F fn) {
  bool thrown = false;
  try { fn(); } catch (const std::invalid_argument&) { thrown = true; }
  require(thrown, "expected invalid geometry exception");
}
int main() {
  using namespace vacd;
  static_assert(!std::is_convertible<std::int32_t, fixed>::value,
                "raw numbers must not implicitly become Q16.16");
  static_assert(!std::is_convertible<double, fixed>::value,
                "binary64 values must not implicitly become Q16.16");
  const fixed zero = fixed::from_raw(0);
  require(from_integer(1).raw()==65536, "Q16.16 one");
  require(from_integer(-1).raw()==-65536, "Q16.16 negative one");
  require(from_integer(-32768).raw()==std::numeric_limits<std::int32_t>::min(), "lower bound");
  require(from_integer(32767).raw()==2147418112, "upper whole bound");
  require(to_integer(from_integer(-2))==-2, "signed integer conversion");
  require(mul(from_integer(3),from_integer(2))==from_integer(6), "multiplication");
  require(mul(from_integer(-3),from_integer(2))==from_integer(-6), "signed multiply");
  require(mul(fixed::from_raw(-32768),from_integer(1))==fixed::from_raw(-32768), "fractional multiply");
  require(add(from_integer(1),from_integer(2))==from_integer(3), "addition");
  require(add(from_integer(1),fixed::from_raw(1)).raw()==65537, "explicit raw increment");
  expect_overflow([] { (void)from_integer(32768); });
  expect_overflow([] { (void)from_integer(static_cast<std::int64_t>(1)<<32); });
  expect_overflow([] { (void)from_integer(-32769); });
  expect_overflow([] { (void)add(fixed::from_raw(std::numeric_limits<std::int32_t>::max()),fixed::from_raw(1)); });
  expect_overflow([] { (void)sub(fixed::from_raw(std::numeric_limits<std::int32_t>::min()),fixed::from_raw(1)); });
  expect_overflow([] {
    (void)mul(fixed::from_raw(std::numeric_limits<std::int32_t>::max()),
              fixed::from_raw(std::numeric_limits<std::int32_t>::max()));
  });

  Entity a{zero,zero,zero,zero,from_integer(1),from_integer(1),"a"};
  Entity b{from_integer(2),zero,zero,zero,from_integer(1),from_integer(1),"b"};
  require(!collides(a.bounds(),b.bounds()), "touching boxes are not overlapping");
  b.posX=from_integer(1);
  require(collides(a.bounds(),b.bounds()), "overlapping boxes");
  expect_invalid([&] {
    const AABB invalid{from_integer(2),zero,from_integer(1),from_integer(1)};
    (void)collides(invalid,a.bounds());
  });
  require(!collides(AABB{zero,zero,zero,from_integer(1)},a.bounds()),"empty box cannot collide");

  Entity badX{fixed::from_raw(std::numeric_limits<std::int32_t>::max()),from_integer(4),
              fixed::from_raw(1),fixed::from_raw(1),zero,zero,"x-overflow"};
  expect_overflow([&]{ badX.update(); });
  require(badX.posX.raw()==std::numeric_limits<std::int32_t>::max(), "x overflow no partial state");
  require(badX.posY==from_integer(4), "x overflow retains y");

  Entity badY{from_integer(4),fixed::from_raw(std::numeric_limits<std::int32_t>::max()),
              fixed::from_raw(1),fixed::from_raw(1),zero,zero,"y-overflow"};
  expect_overflow([&]{ badY.update(); });
  require(badY.posX==from_integer(4), "y overflow cannot commit valid x first");
  require(badY.posY.raw()==std::numeric_limits<std::int32_t>::max(), "y overflow retains y");
}
