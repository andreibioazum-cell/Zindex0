#include "raylib.h"
#include "raymath.h"
#include "config.hpp"

class Player {
public:
    Vector3 pos = {0, 5, 0};
    Vector2 angle = {0, 0};
    float velY = 0;
    bool grounded = false;
    Camera3D camera = {0};

    void Init() {
        camera.up = (Vector3){0, 1, 0};
        camera.fovy = 60.0f;
        camera.projection = CAMERA_PERSPECTIVE;
    }

    void Update() {
        int w = GetScreenWidth();
        int h = GetScreenHeight();

        for (int i = 0; i < GetTouchPointCount(); i++) {
            Vector2 t = GetTouchPosition(i);
            
            if (t.x < w / 3) angle.x -= TOUCH_SPEED;
            else if (t.x > w * 2 / 3 && t.y > h / 2) velY = (grounded) ? JUMP_FORCE : velY;
            else if (t.x > w / 3 && t.x < w * 2 / 3 && t.y > h / 2) {
                pos.x += cosf(angle.x) * MOVE_SPEED;
                pos.z += sinf(angle.x) * MOVE_SPEED;
            }
            if (t.x > w * 2 / 3 && t.y < h / 2) angle.x += TOUCH_SPEED;
        }

        velY -= GRAVITY;
        pos.y += velY;

        if (pos.y < 2.0f) {
            pos.y = 2.0f;
            velY = 0;
            grounded = true;
        } else grounded = false;

        camera.position = pos;
        camera.target = (Vector3){pos.x + cosf(angle.x), pos.y, pos.z + sinf(angle.x)};
    }
};
