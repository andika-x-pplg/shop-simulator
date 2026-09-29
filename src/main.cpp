#include "raylib.h"
#include "Player.hpp"
#include "Shop.hpp"

int main() {
    // 1. Window Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "3D Shop Simulator - Prototype Tahap 1");

    SetTargetFPS(60);

    // Disable cursor for smooth first-person mouse controls
    DisableCursor();

    // 2. Game Entities Initialization
    Shop shop;
    shop.Init();

    Player player;
    // Spawn player in front of the shop entrance
    player.Init({ 0.0f, 0.0f, 10.0f });

    // 3. Main Game Loop
    while (!WindowShouldClose()) {
        // Exit on ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        float deltaTime = GetFrameTime();

        // Update Game Logic
        player.Update(deltaTime, shop.GetColliders());

        // Render Frame
        BeginDrawing();
            ClearBackground({ 135, 206, 235, 255 }); // Sky blue background

            // 3D Rendering Mode
            BeginMode3D(player.GetCamera());
                shop.Render();
            EndMode3D();

            // 2D HUD / UI Rendering
            // Semi-transparent controls overlay box
            DrawRectangle(15, 15, 280, 115, { 15, 20, 25, 200 });
            DrawRectangleLines(15, 15, 280, 115, { 80, 90, 100, 255 });

            DrawText("SHOP SIMULATOR 3D (Tahap 1)", 25, 25, 16, { 255, 215, 0, 255 });
            DrawText("W A S D  : Bergerak", 25, 50, 14, RAYWHITE);
            DrawText("Mouse    : Kontrol Kamera", 25, 70, 14, RAYWHITE);
            DrawText("ESC      : Keluar Game", 25, 90, 14, { 255, 100, 100, 255 });

            // Crosshair in screen center
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            DrawCircle(centerX, centerY, 2.5f, { 255, 255, 255, 200 });
            DrawCircleLines(centerX, centerY, 6.0f, { 255, 255, 255, 150 });

            // FPS Counter
            DrawFPS(screenWidth - 90, 15);

        EndDrawing();
    }

    // 4. Cleanup
    CloseWindow();

    return 0;
}
