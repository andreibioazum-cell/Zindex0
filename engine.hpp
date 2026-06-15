#include "raylib.h"
#include <vector>

struct Voxel { Vector3 pos; Color col; };

class Core {
public:
    std::vector<Voxel> map;
    void Init() {
        for(int x = -WORLD_SIZE; x < WORLD_SIZE; x++) {
            for(int z = -WORLD_SIZE; z < WORLD_SIZE; z++) {
                map.push_back({{(float)x, 0, (float)z}, LIME});
            }
        }
    }
    void Render() {
        for(auto& v : map) DrawCube(v.pos, 1, 1, 1, v.col);
    }
};
