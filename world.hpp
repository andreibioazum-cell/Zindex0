#include "raylib.h"
#include <vector>
#include <math.h>

struct Block { Vector3 pos; };

class World {
public:
    std::vector<Block> blocks;
    void Generate() {
        for (int x = -WORLD_SIZE; x < WORLD_SIZE; x++) {
            for (int z = -WORLD_SIZE; z < WORLD_SIZE; z++) {
                float h = (sinf(x * 0.3f) + cosf(z * 0.3f)) * 1.5f;
                blocks.push_back({(Vector3){(float)x, (float)h, (float)z}});
            }
        }
    }
    void Draw() {
        for (auto& b : blocks) {
            DrawCube(b.pos, 1.0f, 1.0f, 1.0f, LIME);
            DrawCubeWires(b.pos, 1.0f, 1.0f, 1.0f, DARKGREEN);
        }
    }
};
