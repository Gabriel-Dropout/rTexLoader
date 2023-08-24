#include "raylib.h"
#include "raymath.h"

#define RTEXLOADER_IMPLEMENTATION
#include "rtexloader.hpp"

const int screenWidth = 640;
const int screenHeight = 480;

int main() {
    /// Initialization ///
    InitWindow(screenWidth, screenHeight, "atlas loading test");
    SetTargetFPS(60);
    
    initAtlas("resources/atlas.png");
    int globalTimer = 0;

    /// Main game loop ///
    while (!WindowShouldClose()) {
        /// Update ///
        globalTimer++;

        /// Draw ///
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawSpriteAtlas(atlas, getSprite("big_zombie_idle_anim_f", globalTimer/8%4), 20, 240 + 50*sinf(globalTimer*10*DEG2RAD), 4, 4, 0.0f, WHITE);
            DrawSpriteAtlas(atlas, getSprite("dwarf_m_run_anim_f", globalTimer/8%4), 220, 180, 4 + sinf(globalTimer*10*DEG2RAD), 4 + sinf(globalTimer*10*DEG2RAD), (float)globalTimer, WHITE);
            DrawSpriteAtlas(atlas, getSprite(globalTimer/60%2 ? "doors_leaf_closed" : "doors_leaf_open"), 450, 50, 4, 4, 0.0f, WHITE);

        EndDrawing();
    }
    unloadAtlas();
}