// VACD 5.0 inspired fixed-point collision demo.
// Original demonstration and attribution: Andrew Rusher.
// Correctness patch: checked signed Q16.16 arithmetic and regression tests.
#include "src/vacd_collision.h"
#include <iostream>

int main() {
  using namespace vacd;
  try {
    Entity limb{from_integer(0), from_integer(0),
                from_integer(1), from_integer(0),
                from_integer(10), from_integer(10), "Sensor Limb"};
    Entity obstacle{from_integer(50), from_integer(0),
                    0, 0, from_integer(15), from_integer(15), "Obstacle"};

    // OPT-style invariant: the obstacle does not move, so its validated bounds
    // may be reused until the obstacle geometry changes.
    const AABB obstacleBounds = obstacle.bounds();
    std::cout << "VACD 5.0 Fixed-Point Collision Demo\n\n";
    for (int step = 0; step < 60; ++step) {
      limb.update();
      std::cout << "Step " << step << " | Limb Pos: ("
                << to_integer(limb.posX) << ", " << to_integer(limb.posY) << ")";
      if (collides(limb.bounds(), obstacleBounds)) {
        std::cout << " >>> COLLISION DETECTED! Demo reversal.\n";
        limb.velX = mul(limb.velX, from_integer(-1));
      } else {
        std::cout << " - Clear\n";
      }
    }
    std::cout << "\nDeterministic bounded fixed-point demo completed.\n";
  } catch (const std::exception& error) {
    std::cerr << "Simulation aborted: " << error.what() << "\n";
    return 1;
  }
  return 0;
}
