#include "raylib.h"
#include "Player.hpp"
#include "Shop.hpp"
#include "Customer.hpp"
#include "Cashier.hpp"
#include <string>
#include <vector>
#include <cstdlib>

int main() {
    // 1. Window Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "3D Shop Simulator - Tahap 5: Cashier & Economy System");

    SetTargetFPS(60);

    // Disable cursor for smooth first-person mouse controls
    DisableCursor();

    // 2. Game Entities Initialization
    Shop shop;
    shop.Init();

    Player player;
    // Spawn player in front of shop entrance
    player.Init({ 0.0f, 0.0f, 10.0f });

    // 3. Economy & Shop Treasury (Saldo Awal: Rp100.000)
    int shopMoney = 100000;

    // Transaction Notification Banner
    std::string transactionNotice = "";
    float transactionNoticeTimer = 0.0f;

    // 4. Customer NPC System Management
    std::vector<Customer> customers;
    const size_t maxActiveCustomers = 3;
    float spawnTimer = 2.0f; // First customer arrives in 2 seconds
    int customerCounter = 1;

    // Pre-defined customer profiles (name, skin tone, shirt color)
    const std::vector<std::string> customerNames = { "Budi", "Siti", "Andi", "Dewi", "Rian", "Maya" };
    const std::vector<Color> shirtColors = {
        { 52, 152, 219, 255 },  // Blue
        { 231, 76, 60, 255 },   // Red
        { 46, 204, 113, 255 },  // Green
        { 155, 89, 182, 255 },  // Purple
        { 241, 196, 15, 255 },  // Yellow
        { 230, 126, 34, 255 }   // Orange
    };
    const std::vector<Color> skinColors = {
        { 255, 220, 185, 255 },
        { 240, 200, 160, 255 },
        { 210, 165, 130, 255 },
        { 180, 135, 100, 255 }
    };

    // 5. Main Game Loop
    while (!WindowShouldClose()) {
        // Exit on ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        float deltaTime = GetFrameTime();

        // Update Transaction Notice Timer
        if (transactionNoticeTimer > 0.0f) {
            transactionNoticeTimer -= deltaTime;
            if (transactionNoticeTimer <= 0.0f) {
                transactionNotice = "";
            }
        }

        // Update Game Logic
        player.Update(deltaTime, shop.GetColliders());
        shop.Update(deltaTime);

        // Check Interaction Target (Look ray to Rack within 3.5m)
        Rack* targetedRack = shop.GetTargetedRack(player.GetEyePosition(), player.GetLookDirection(), 3.5f);

        // Interaction Key 'E' Handling (Player stage 2 feature)
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
            }
        }

        // Customer Spawner & State Management
        spawnTimer -= deltaTime;
        if (spawnTimer <= 0.0f && customers.size() < maxActiveCustomers) {
            // Spawn a new customer outside
            float spawnX = (customerCounter % 2 == 0) ? 3.0f : -3.0f;
            Vector3 spawnPos = { spawnX, 0.0f, 16.0f + (customerCounter % 3) * 1.5f };

            std::string cName = customerNames[customerCounter % customerNames.size()];
            Color cShirt = shirtColors[customerCounter % shirtColors.size()];
            Color cSkin = skinColors[customerCounter % skinColors.size()];

            Customer newCust(customerCounter, cName, spawnPos, cSkin, cShirt);
            customers.push_back(newCust);
            customerCounter++;
            
            // Interval for next customer spawn (5s - 8s)
            spawnTimer = 6.0f + (customerCounter % 3) * 1.5f;
        }

        // Count and assign queue indexes for customers heading to or at the cashier
        int cashierQueueCount = 0;
        for (auto& cust : customers) {
            int qIndex = -1;
            if (cust.IsInCashierQueue()) {
                qIndex = cashierQueueCount;
                cashierQueueCount++;
            }

            bool didPay = false;
            int paidAmount = 0;
            std::string paidProduct = "";

            cust.Update(deltaTime, shop, qIndex, didPay, paidAmount, paidProduct);

            if (didPay && paidAmount > 0) {
                shopMoney += paidAmount;
                transactionNotice = "Pembayaran Berhasil! " + cust.GetName() + " membeli " + paidProduct + " (+Rp" + std::to_string(paidAmount) + ")";
                transactionNoticeTimer = 3.5f;
            }
        }
        shop.GetCashier().SetQueueCount(cashierQueueCount);

        // Remove despawned customers
        for (auto it = customers.begin(); it != customers.end();) {
            if (it->IsDespawned()) {
                it = customers.erase(it);
            } else {
                ++it;
            }
        }

        // Check if player is close to the Cashier Counter (Informative HUD prompt)
        float distToCashier = Vector3Distance(player.GetPosition(), shop.GetCashier().GetPosition());
        bool playerNearCashier = (distToCashier < 3.8f);

        // Render Frame
        BeginDrawing();
            ClearBackground({ 135, 206, 235, 255 }); // Sky blue background

            // 3D Rendering Mode
            BeginMode3D(player.GetCamera());
                shop.Render();
                for (auto& cust : customers) {
                    cust.Render();
                }
                player.RenderHeldItem();
            EndMode3D();

            // 2D HUD / UI Rendering
            // Top-Left Controls & Status Box
            DrawRectangle(15, 15, 320, 240, { 15, 20, 25, 220 });
            DrawRectangleLines(15, 15, 320, 240, { 70, 85, 100, 255 });

            DrawText("SHOP SIMULATOR 3D (Tahap 5)", 25, 25, 16, { 255, 215, 0, 255 });
            DrawText("WASD     : Bergerak", 25, 48, 14, RAYWHITE);
            DrawText("Mouse    : Kontrol Kamera", 25, 68, 14, RAYWHITE);
            DrawText("E        : Interaksi Rak/Produk", 25, 88, 14, { 100, 230, 100, 255 });
            DrawText("ESC      : Keluar Game", 25, 108, 14, { 255, 100, 100, 255 });

            // Carried Product Status
            std::string carriedText = "Membawa: " + player.GetHeldProductName();
            Color carriedColor = player.IsHoldingProduct() ? Color{ 255, 220, 50, 255 } : Color{ 180, 190, 200, 255 };
            DrawText(carriedText.c_str(), 25, 138, 15, carriedColor);

            // Treasury / Money Balance
            std::string moneyText = "Uang Toko: Rp" + std::to_string(shopMoney);
            DrawText(moneyText.c_str(), 25, 165, 16, { 50, 255, 120, 255 });

            // Customer / Cashier Status Debug
            std::string custCountText = "Customer: " + std::to_string(customers.size()) + "/" + std::to_string(maxActiveCustomers) +
                                        " | Antrian Kasir: " + std::to_string(cashierQueueCount);
            DrawText(custCountText.c_str(), 25, 192, 13, { 100, 220, 255, 255 });

            if (!customers.empty()) {
                const auto& activeCust = customers.front();
                std::string custInfo = activeCust.GetName() + " -> " + activeCust.GetStateString();
                DrawText(custInfo.c_str(), 25, 212, 12, { 200, 230, 250, 255 });
            } else {
                DrawText("Menunggu customer baru...", 25, 212, 12, { 140, 150, 160, 255 });
            }

            // Center Interaction Prompt (When player aims at rack)
            if (targetedRack != nullptr) {
                std::string promptText = "";
                Color promptBg = { 20, 25, 30, 220 };
                Color promptTextColor = RAYWHITE;

                if (!player.IsHoldingProduct()) {
                    if (targetedRack->HasStock()) {
                        promptText = "Tekan E untuk mengambil " + targetedRack->GetProductName() + 
                                     " (Stok: " + std::to_string(targetedRack->GetStock()) + 
                                     " | Rp" + std::to_string(GetProductInfo(targetedRack->GetProductType()).price) + ")";
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
            } else if (playerNearCashier) {
                // Info prompt when player approaches cashier counter
                std::string cashierInfo = "Meja Kasir Toko";
                int textWidth = MeasureText(cashierInfo.c_str(), 16);
                int boxX = (screenWidth - textWidth) / 2 - 15;
                int boxY = screenHeight / 2 + 50;
                DrawRectangle(boxX, boxY, textWidth + 30, 32, { 20, 25, 30, 200 });
                DrawRectangleLines(boxX, boxY, textWidth + 30, 32, { 100, 180, 255, 255 });
                DrawText(cashierInfo.c_str(), boxX + 15, boxY + 8, 16, { 150, 210, 255, 255 });
            }

            // Successful Transaction Notification Banner (Center Top)
            if (transactionNoticeTimer > 0.0f) {
                int noticeWidth = MeasureText(transactionNotice.c_str(), 18);
                int nBoxX = (screenWidth - noticeWidth) / 2 - 25;
                int nBoxY = 25;

                DrawRectangle(nBoxX, nBoxY, noticeWidth + 50, 42, { 20, 120, 50, 230 });
                DrawRectangleLines(nBoxX, nBoxY, noticeWidth + 50, 42, { 100, 255, 150, 255 });
                DrawText(transactionNotice.c_str(), nBoxX + 25, nBoxY + 12, 18, RAYWHITE);
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

    // 6. Cleanup
    CloseWindow();

    return 0;
}
