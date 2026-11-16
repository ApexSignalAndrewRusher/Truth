// Finite differential replay: uncached reference vs cached stationary AABB.
// Contract-inspired optimisation; no theorem or measured speed-up is claimed.
#include "../src/vacd_collision.h"
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace vacd;
void requireReplay(bool condition, const char* msg) {
  if (!condition) { std::cerr << "REPLAY FAIL: " << msg << '\n'; std::exit(1); }
}
struct Snapshot {
  fixed posX, posY, velX, velY;
  bool collision;
};
bool same(const Snapshot& a, const Snapshot& b) {
  return a.posX==b.posX && a.posY==b.posY &&
         a.velX==b.velX && a.velY==b.velY && a.collision==b.collision;
}
std::vector<Snapshot> run(std::int32_t start, bool cached) {
  const fixed zero = fixed::from_raw(0);
  Entity limb{from_integer(start),zero,from_integer(1),zero,
              from_integer(10),from_integer(10),"Sensor Limb"};
  Entity obstacle{from_integer(50),zero,zero,zero,
                  from_integer(15),from_integer(15),"Obstacle"};
  const AABB fixedBounds=obstacle.bounds();
  std::vector<Snapshot> journal;
  for(int step=0;step<60;++step){
    limb.update();
    const AABB obstacleAtStep=cached?fixedBounds:obstacle.bounds();
    const bool hit=collides(limb.bounds(),obstacleAtStep);
    if(hit) limb.velX=mul(limb.velX,from_integer(-1));
    journal.push_back({limb.posX,limb.posY,limb.velX,limb.velY,hit});
  }
  return journal;
}
int main() {
  for(std::int32_t start=-60;start<68;++start){
    const auto reference=run(start,false);
    const auto optimized=run(start,true);
    const auto replay=run(start,true);
    requireReplay(reference.size()==60 && optimized.size()==reference.size(),"journal size");
    for(std::size_t i=0;i<reference.size();++i){
      requireReplay(same(reference[i],optimized[i]),"cached path differs from reference");
      requireReplay(same(replay[i],optimized[i]),"repeated run differs");
    }
  }
  const auto representative=run(0,true);
  bool sawCollision=false;
  for(const auto& step:representative) sawCollision=sawCollision||step.collision;
  requireReplay(sawCollision,"expected collision not encountered");
}
