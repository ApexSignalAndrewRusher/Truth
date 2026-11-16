// Finite differential replay: uncached reference vs cached stationary AABB.
// This is not a proof of correctness outside the tested domain.
#include "../src/vacd_collision.h"
#include <cassert>
#include <cstdint>
#include <vector>
using namespace vacd;

struct Snapshot {
  fixed posX;
  fixed posY;
  fixed velX;
  fixed velY;
  bool collision;
};
bool same(const Snapshot& a, const Snapshot& b) {
  return a.posX==b.posX && a.posY==b.posY &&
         a.velX==b.velX && a.velY==b.velY && a.collision==b.collision;
}

std::vector<Snapshot> run(std::int32_t start, bool cached) {
  Entity limb{from_integer(start), 0, from_integer(1), 0,
              from_integer(10),from_integer(10),"Sensor Limb"};
  Entity obstacle{from_integer(50),0,0,0,
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
    assert(reference.size()==60 && optimized.size()==reference.size());
    for(std::size_t i=0;i<reference.size();++i){
      assert(same(reference[i],optimized[i]));
      assert(same(replay[i],optimized[i]));
    }
  }
  const auto representative=run(0,true);
  bool sawCollision=false;
  for(const auto& step:representative) sawCollision=sawCollision||step.collision;
  assert(sawCollision);
}
