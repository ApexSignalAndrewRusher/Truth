#include "../src/vacd_collision.h"
#include <cassert>
#include <limits>
#include <stdexcept>

template<class F> void expect_overflow(F fn) {
  bool thrown = false;
  try { fn(); } catch (const std::overflow_error&) { thrown = true; }
  assert(thrown);
}

int main() {
  using namespace vacd;
  assert(from_integer(1) == 65536);
  assert(from_integer(-1) == -65536);
  assert(to_integer(from_integer(32767)) == 32767);
  assert(to_integer(from_integer(-2)) == -2);
  assert(mul(from_integer(3), from_integer(2)) == from_integer(6));
  assert(mul(from_integer(-3), from_integer(2)) == from_integer(-6));
  assert(mul(-32768, 65536) == -32768);
  assert(add(from_integer(1), from_integer(2)) == from_integer(3));
  expect_overflow([]{ (void)from_integer(32768); });
  expect_overflow([]{ (void)add(std::numeric_limits<fixed>::max(), 1); });
  expect_overflow([]{ (void)sub(std::numeric_limits<fixed>::min(), 1); });
  expect_overflow([]{ (void)mul(std::numeric_limits<fixed>::max(),
                              std::numeric_limits<fixed>::max()); });
  Entity a{0,0,0,0,from_integer(1),from_integer(1),"a"};
  Entity b{from_integer(2),0,0,0,from_integer(1),from_integer(1),"b"};
  assert(!collides(a.bounds(),b.bounds())); // touching
  b.posX=from_integer(1);
  assert(collides(a.bounds(),b.bounds())); // overlap
  Entity unstable{std::numeric_limits<fixed>::max(),from_integer(4),
                  1,1,0,0,"unstable"};
  expect_overflow([&]{ unstable.update(); });
  assert(unstable.posX == std::numeric_limits<fixed>::max());
  assert(unstable.posY == from_integer(4)); // no partial state
}
