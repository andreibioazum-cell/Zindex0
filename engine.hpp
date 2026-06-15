#include "raylib.h"
#include <vector>

struct Block { Vector3 pos; Color color; };

class VoxelEngine {
public:
    std::vector<Block> blocks;
    void Generate() {
        for(int x = -10; x < 10; x++) {
            for(int z = -10; z < 10; z++) {
                blocks.push_back({(Vector3){(float)x, 0, (float)z}, LIME});
            }
        }
    }
    void Draw() {
        for(auto& b : blocks) DrawCube(b.pos, 1, 1, 1, b.color);
    }
};
