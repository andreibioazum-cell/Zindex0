#include "raylib.h"
#include "config.hpp"
#include "world.hpp"
#include "player.hpp"

int main() {
    InitWindow(0, 0, "Cubic Mobile");
    World world;
    world.Generate();
    Player player;
    player.Init();
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        player.Update();
        BeginDrawing();
            ClearBackground(SKYBLUE);
            BeginMode3D(player.camera);
                world.Draw();
            EndMode3D();
            DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 5, RED);
            DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
