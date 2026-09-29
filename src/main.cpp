#include "raylib.h"
#include "Player.hpp"
#include "Shop.hpp"
#include <string>

int main() {
    // 1. Window Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "3D Shop Simulator - Tahap 2: Racks, Products & Interaction");

    SetTargetFPS(60);

    // Disable cursor for smooth first-person mouse controls
    DisableCursor();

    // 2. Game Entities Initialization
    Shop shop;
    shop.Init();

    Player player;
    // Spawn player in front of shop entrance
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

        // Check Interaction Target (Look ray to Rack within 3.5m)
        Rack* targetedRack = shop.GetTargetedRack(player.GetEyePosition(), player.GetLookDirection(), 3.5f);

        // Interaction Key 'E' Handling
        if (IsKeyPressed(KEY_E)) {
            if (targetedRack != nullptr) {
                if (!player.IsHoldingProduct()) {
                    // Player has empty hands -> Try taking product
                    if (targetedRack->HasStock()) {
                        ProductType pType = targetedRack->GetProductType();
                        if (targetedRack->TakeProduct()) {
                            player.PickUpProduct(pType);
                        }
                    } else {
                        player.SetFeedbackMessage("Stok di rak ini habis!", 2.0f);
                    }
                } else {
                    // Player is holding a product
                    ProductType held = player.GetHeldProduct();
                    if (targetedRack->GetProductType() == held) {
                        // Matching rack type
                        if (!targetedRack->IsFull()) {
                            if (targetedRack->PlaceProduct(held)) {
                                player.DropOrPlaceProduct();
                            }
                        } else {
                            player.SetFeedbackMessage("Rak ini sudah penuh!", 2.0f);
                        }
                    } else {
                        // Mismatched rack type
                        player.SetFeedbackMessage("Rak ini bukan untuk " + player.GetHeldProductName() + "!", 2.0f);
                    }
                }
            } else {
                // Pressed E in empty space or holding item without looking at rack
                if (player.IsHoldingProduct()) {
                    // Holding item reminder
                }
            }
        }

        // Render Frame
        BeginDrawing();
            ClearBackground({ 135, 206, 235, 255 }); // Sky blue background

            // 3D Rendering Mode
            BeginMode3D(player.GetCamera());
                shop.Render();
                player.RenderHeldItem();
            EndMode3D();

            // 2D HUD / UI Rendering
            // Top-Left Controls & Status Box
            DrawRectangle(15, 15, 300, 160, { 15, 20, 25, 210 });
            DrawRectangleLines(15, 15, 300, 160, { 70, 85, 100, 255 });

            DrawText("SHOP SIMULATOR 3D", 25, 25, 16, { 255, 215, 0, 255 });
            DrawText("WASD     : Bergerak", 25, 48, 14, RAYWHITE);
            DrawText("Mouse    : Kontrol Kamera", 25, 68, 14, RAYWHITE);
            DrawText("E        : Interaksi Rak/Produk", 25, 88, 14, { 100, 230, 100, 255 });
            DrawText("ESC      : Keluar Game", 25, 108, 14, { 255, 100, 100, 255 });

            // Carried Product Status
            std::string carriedText = "Membawa: " + player.GetHeldProductName();
            Color carriedColor = player.IsHoldingProduct() ? Color{ 255, 220, 50, 255 } : Color{ 180, 190, 200, 255 };
            DrawText(carriedText.c_str(), 25, 138, 15, carriedColor);

            // Center Interaction Prompt (When player aims at rack)
            if (targetedRack != nullptr) {
                std::string promptText = "";
                Color promptBg = { 20, 25, 30, 220 };
                Color promptTextColor = RAYWHITE;

                if (!player.IsHoldingProduct()) {
                    if (targetedRack->HasStock()) {
                        promptText = "Tekan E untuk mengambil " + targetedRack->GetProductName() + 
                                     " (Stok: " + std::to_string(targetedRack->GetStock()) + ")";
                        promptTextColor = { 100, 255, 100, 255 };
                    } else {
                        promptText = "[Stok " + targetedRack->GetProductName() + " Kosong]";
                        promptTextColor = { 255, 120, 120, 255 };
                    }
                } else {
                    if (targetedRack->GetProductType() == player.GetHeldProduct()) {
                        if (!targetedRack->IsFull()) {
                            promptText = "Tekan E untuk menaruh " + player.GetHeldProductName() + 
                                         " (Stok: " + std::to_string(targetedRack->GetStock()) + "/" + std::to_string(targetedRack->GetMaxStock()) + ")";
                            promptTextColor = { 100, 220, 255, 255 };
                        } else {
                            promptText = "[Rak " + targetedRack->GetProductName() + " Penuh]";
                            promptTextColor = { 255, 120, 120, 255 };
                        }
                    } else {
                        promptText = "[Rak " + targetedRack->GetProductName() + " - Bukan untuk " + player.GetHeldProductName() + "]";
                        promptTextColor = { 255, 180, 100, 255 };
                    }
                }

                if (!promptText.empty()) {
                    int textWidth = MeasureText(promptText.c_str(), 16);
                    int boxX = (screenWidth - textWidth) / 2 - 20;
                    int boxY = screenHeight / 2 + 50;

                    DrawRectangle(boxX, boxY, textWidth + 40, 36, promptBg);
                    DrawRectangleLines(boxX, boxY, textWidth + 40, 36, promptTextColor);
                    DrawText(promptText.c_str(), boxX + 20, boxY + 10, 16, promptTextColor);
                }
            }

            // Temporary Warning / Notification Banner
            if (player.HasFeedbackMessage()) {
                std::string msg = player.GetFeedbackMessage();
                int msgWidth = MeasureText(msg.c_str(), 18);
                int msgBoxX = (screenWidth - msgWidth) / 2 - 25;
                int msgBoxY = screenHeight / 2 - 80;

                DrawRectangle(msgBoxX, msgBoxY, msgWidth + 50, 40, { 180, 40, 40, 230 });
                DrawRectangleLines(msgBoxX, msgBoxY, msgWidth + 50, 40, { 255, 220, 220, 255 });
                DrawText(msg.c_str(), msgBoxX + 25, msgBoxY + 11, 18, RAYWHITE);
            }

            // Crosshair in screen center
            int centerX = screenWidth / 2;
            int centerY = screenHeight / 2;
            Color crosshairColor = (targetedRack != nullptr) ? Color{ 50, 255, 100, 230 } : Color{ 255, 255, 255, 180 };
            DrawCircle(centerX, centerY, 3.0f, crosshairColor);
            DrawCircleLines(centerX, centerY, 7.0f, crosshairColor);

            // FPS Counter in top right
            DrawFPS(screenWidth - 90, 15);

        EndDrawing();
    }

    // 4. Cleanup
    CloseWindow();

    return 0;
}
