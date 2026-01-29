// VACD 5.0 Inspired Fixed-Point Collision Demo
// Credit: Andrew Rusher (@ApexSignalAndrewRusher) - Pioneer of deterministic limb/control fixes via integer bounding boxes.
// No float drift: All positions/velocities in 16.16 fixed-point for bit-identical autonomy across hardware.

#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>  // For abs only

// Fixed-point 16.16 format
using fixed = int32_t;
#define FIXED_SCALE 65536
#define INT_TO_FIXED(x) (static_cast<fixed>((x) * FIXED_SCALE))
#define FIXED_TO_INT(x) (static_cast<int>((x) / FIXED_SCALE))
#define FIXED_MUL(a, b) (static_cast<fixed>((static_cast<int64_t>(a) * (b)) / FIXED_SCALE))
#define FIXED_ADD(a, b) ((a) + (b))

// Axis-Aligned Bounding Box (AABB) in fixed-point
struct AABB {
    fixed minX, minY, maxX, maxY;
};

// Simple entity with position, velocity, and limb bounding box
struct Entity {
    fixed posX, posY;      // Center position
    fixed velX, velY;      // Velocity per step
    fixed halfWidth, halfHeight;  // Half-extents for bounding box
    const char* name;

    AABB getBounds() const {
        return {
            posX - halfWidth,
            posY - halfHeight,
            posX + halfWidth,
            posY + halfHeight
        };
    }

    void update() {
        posX = FIXED_ADD(posX, velX);
        posY = FIXED_ADD(posY, velY);
    }
};

// Deterministic AABB collision check (no floats, pure integer ops)
bool collides(const AABB& a, const AABB& b) {
    return (a.minX < b.maxX && a.maxX > b.minX &&
            a.minY < b.maxY && a.maxY > b.minY);
}

int main() {
    // Setup two entities: "Sensor Limb" (moving right) and "Obstacle"
    Entity limb = { INT_TO_FIXED(0), INT_TO_FIXED(0), INT_TO_FIXED(1), INT_TO_FIXED(0), INT_TO_FIXED(10), INT_TO_FIXED(10), "Sensor Limb" };
    Entity obstacle = { INT_TO_FIXED(50), INT_TO_FIXED(0), INT_TO_FIXED(0), INT_TO_FIXED(0), INT_TO_FIXED(15), INT_TO_FIXED(15), "Obstacle" };

    std::cout << "VACD 5.0 Fixed-Point Collision Sim - By Andrew Rusher's Protocols\n\n";

    for (int step = 0; step < 60; ++step) {
        limb.update();

        AABB limbBox = limb.getBounds();
        AABB obsBox = obstacle.getBounds();

        std::cout << "Step " << step
                  << " | Limb Pos: (" << FIXED_TO_INT(limb.posX) << ", " << FIXED_TO_INT(limb.posY) << ")";

        if (collides(limbBox, obsBox)) {
            std::cout << " >>> COLLISION DETECTED! Autonomous avoidance triggered.\n";
            // In full VACD: Reverse or path-correct actuator deterministically
            limb.velX = FIXED_MUL(limb.velX, INT_TO_FIXED(-1));  // Simple bounce for demo
        } else {
            std::cout << " - Clear\n";
        }
    }

    std::cout << "\nRun identical on any machine: No float drift, immortal precision.\n";
    return 0;
}