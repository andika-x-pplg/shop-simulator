#include "raylib.h"
#include "Player.hpp"
#include "Shop.hpp"
#include "Customer.hpp"
#include "Cashier.hpp"
#include "Storage.hpp"
#include "Supplier.hpp"
#include <string>
#include <vector>
#include <cstdlib>

int main() {
    // 1. Window Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "3D Shop Simulator - Tahap 6: Supplier, Delivery & Restock");

    SetTargetFPS(60);

    // Disable cursor for smooth first-person mouse controls
    DisableCursor();

    // 2. Game Entities Initialization
    Shop shop;
    shop.Init();

    Supplier supplier;
    supplier.Init();

    Player player;
    // Spawn player in front of shop entrance
    player.Init({ 0.0f, 0.0f, 10.0f });

    // 3. Economy & Shop Treasury (Saldo Awal: Rp100.000)
    int shopMoney = 100000;

    // Transaction & Delivery Notification Banner
    std::string topNotice = "";
    float topNoticeTimer = 0.0f;
    Color topNoticeColor = { 20, 120, 50, 230 };

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
            if (supplier.IsMenuOpen()) {
                supplier.SetMenuOpen(false);
                DisableCursor();
            } else {
                break;
            }
        }

        float deltaTime = GetFrameTime();

        // Toggle Supplier Menu with TAB
        if (IsKeyPressed(KEY_TAB)) {
            supplier.ToggleMenu();
            if (supplier.IsMenuOpen()) {
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        // Supplier Menu Inputs when Menu is Open
        if (supplier.IsMenuOpen()) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                supplier.PreviousProduct();
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                supplier.NextProduct();
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                supplier.IncreaseQuantity(5);
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                supplier.DecreaseQuantity(5);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                ProductType selType = supplier.GetSelectedProductType();
                int qty = supplier.GetOrderQuantity();
                std::string errorMsg = "";
                if (supplier.PlaceOrder(selType, qty, shopMoney, errorMsg)) {
                    topNotice = "Pesanan Supplier Berhasil Dibuat! (" + std::to_string(qty) + "x " + GetProductInfo(selType).name + ")";
                    topNoticeColor = { 30, 100, 180, 230 };
                    topNoticeTimer = 3.5f;
                } else {
                    player.SetFeedbackMessage(errorMsg, 2.5f);
                }
            }
        }

        // Update Top Notification Timer
        if (topNoticeTimer > 0.0f) {
            topNoticeTimer -= deltaTime;
            if (topNoticeTimer <= 0.0f) {
                topNotice = "";
            }
        }

        // Update Game Logic (Player movement active when supplier menu closed)
        if (!supplier.IsMenuOpen()) {
            player.Update(deltaTime, shop.GetColliders());
        }
        shop.Update(deltaTime);

        // Update Supplier Deliveries -> Delivery to Storage
        std::vector<std::string> deliveredNotices;
        supplier.Update(deltaTime, shop.GetStorage(), deliveredNotices);
        for (const auto& notice : deliveredNotices) {
            topNotice = notice;
            topNoticeColor = { 20, 140, 60, 230 };
            topNoticeTimer = 4.0f;
        }

        // Check Interaction Targets (Look ray to Rack or Storage Pallets within 3.5m)
        Rack* targetedRack = shop.GetTargetedRack(player.GetEyePosition(), player.GetLookDirection(), 3.5f);
        ProductType targetedStorageProduct = shop.GetStorage().GetTargetedProduct(player.GetPosition(), player.GetLookDirection(), 3.5f);

        // Interaction Key 'E' Handling
        if (IsKeyPressed(KEY_E) && !supplier.IsMenuOpen()) {
            // 1. Interaction with Storage Pallets (Taking stock from Storage to Restock)
            if (targetedStorageProduct != ProductType::NONE) {
                if (!player.IsHoldingProduct()) {
                    if (shop.GetStorage().TakeStock(targetedStorageProduct)) {
                        player.PickUpProduct(targetedStorageProduct);
                        player.SetFeedbackMessage("Mengambil " + GetProductInfo(targetedStorageProduct).name + " dari Storage", 1.5f);
                    } else {
                        player.SetFeedbackMessage("Stok " + GetProductInfo(targetedStorageProduct).name + " di Storage kosong! Beli di Supplier (TAB)", 2.5f);
                    }
                } else {
                    // Putting held item back into storage
                    ProductType held = player.GetHeldProduct();
                    if (held == targetedStorageProduct) {
                        shop.GetStorage().AddStock(held, 1);
                        player.DropOrPlaceProduct();
                        player.SetFeedbackMessage("Mengembalikan " + GetProductInfo(held).name + " ke Storage", 1.5f);
                    } else {
                        player.SetFeedbackMessage("Pallet ini untuk " + GetProductInfo(targetedStorageProduct).name + "!", 2.0f);
                    }
                }
            }
            // 2. Interaction with Shop Racks (Taking or Restocking Racks)
            else if (targetedRack != nullptr) {
                if (!player.IsHoldingProduct()) {
                    // Player takes product from shelf
                    if (targetedRack->HasStock()) {
                        ProductType pType = targetedRack->GetProductType();
                        if (targetedRack->TakeProduct()) {
                            player.PickUpProduct(pType);
                        }
                    } else {
                        player.SetFeedbackMessage("Stok di rak ini habis! Ambil dari Storage atau beli di Supplier (TAB)", 2.5f);
                    }
                } else {
                    // Player restocks / places product on rack
                    ProductType held = player.GetHeldProduct();
                    if (targetedRack->GetProductType() == held) {
                        if (!targetedRack->IsFull()) {
                            if (targetedRack->PlaceProduct(held)) {
                                player.DropOrPlaceProduct();
                                player.SetFeedbackMessage("Restock " + GetProductInfo(held).name + " Berhasil! (+1)", 1.5f);
                            }
                        } else {
                            player.SetFeedbackMessage("Rak sudah penuh! (Maks: " + std::to_string(targetedRack->GetMaxStock()) + ")", 2.0f);
                        }
                    } else {
                        player.SetFeedbackMessage("Produk tidak sesuai dengan rak! (Rak ini untuk " + targetedRack->GetProductName() + ")", 2.0f);
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
                topNotice = "Pembayaran Berhasil! " + cust.GetName() + " membeli " + paidProduct + " (+Rp" + std::to_string(paidAmount) + ")";
                topNoticeColor = { 20, 120, 50, 230 };
                topNoticeTimer = 3.5f;
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

        // Check proximity to Cashier
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
            DrawRectangle(15, 15, 330, 265, { 15, 20, 25, 225 });
            DrawRectangleLines(15, 15, 330, 265, { 70, 85, 100, 255 });

            DrawText("SHOP SIMULATOR 3D (Tahap 6)", 25, 25, 16, { 255, 215, 0, 255 });
            DrawText("WASD     : Bergerak", 25, 48, 14, RAYWHITE);
            DrawText("Mouse    : Kontrol Kamera", 25, 68, 14, RAYWHITE);
            DrawText("E        : Interaksi Rak / Storage", 25, 88, 14, { 100, 230, 100, 255 });
            DrawText("TAB      : Menu Supplier & Order", 25, 108, 14, { 255, 180, 50, 255 });
            DrawText("ESC      : Keluar Game / Tutup Menu", 25, 128, 14, { 255, 100, 100, 255 });

            // Carried Product Status
            std::string carriedText = "Membawa: " + player.GetHeldProductName();
            Color carriedColor = player.IsHoldingProduct() ? Color{ 255, 220, 50, 255 } : Color{ 180, 190, 200, 255 };
            DrawText(carriedText.c_str(), 25, 152, 15, carriedColor);

            // Treasury / Money Balance
            std::string moneyText = "Uang Toko: Rp" + std::to_string(shopMoney);
            DrawText(moneyText.c_str(), 25, 175, 16, { 50, 255, 120, 255 });

            // Storage Stock Summary
            std::string storageInfo = "Storage: Minuman " + std::to_string(shop.GetStorage().GetBeverageStock()) +
                                      " | Roti " + std::to_string(shop.GetStorage().GetBreadStock()) +
                                      " | Kaleng " + std::to_string(shop.GetStorage().GetCannedFoodStock());
            DrawText(storageInfo.c_str(), 25, 202, 13, { 255, 200, 120, 255 });

            // Customer / Cashier Status Debug
            std::string custCountText = "Customer: " + std::to_string(customers.size()) + "/" + std::to_string(maxActiveCustomers) +
                                        " | Antrian Kasir: " + std::to_string(cashierQueueCount);
            DrawText(custCountText.c_str(), 25, 222, 13, { 100, 220, 255, 255 });

            if (!customers.empty()) {
                const auto& activeCust = customers.front();
                std::string custInfo = activeCust.GetName() + " -> " + activeCust.GetStateString();
                DrawText(custInfo.c_str(), 25, 242, 12, { 200, 230, 250, 255 });
            } else {
                DrawText("Menunggu customer baru...", 25, 242, 12, { 140, 150, 160, 255 });
            }

            // Center Interaction Prompt (When player aims at rack or storage pallet)
            if (targetedStorageProduct != ProductType::NONE) {
                std::string stPrompt = "";
                Color stColor = { 255, 200, 100, 255 };
                std::string pName = GetProductInfo(targetedStorageProduct).name;
                int stStock = shop.GetStorage().GetStock(targetedStorageProduct);

                if (!player.IsHoldingProduct()) {
                    if (stStock > 0) {
                        stPrompt = "Tekan E untuk mengambil " + pName + " dari Storage (Tersedia: " + std::to_string(stStock) + ")";
                        stColor = { 100, 255, 120, 255 };
                    } else {
                        stPrompt = "[Storage " + pName + " Kosong - Beli di Supplier (TAB)]";
                        stColor = { 255, 140, 100, 255 };
                    }
                } else {
                    if (player.GetHeldProduct() == targetedStorageProduct) {
                        stPrompt = "Tekan E untuk menaruh kembali " + pName + " ke Storage";
                    } else {
                        stPrompt = "[Pallet Storage " + pName + " - Bukan untuk " + player.GetHeldProductName() + "]";
                    }
                }

                int textWidth = MeasureText(stPrompt.c_str(), 16);
                int boxX = (screenWidth - textWidth) / 2 - 20;
                int boxY = screenHeight / 2 + 50;
                DrawRectangle(boxX, boxY, textWidth + 40, 36, { 20, 25, 30, 220 });
                DrawRectangleLines(boxX, boxY, textWidth + 40, 36, stColor);
                DrawText(stPrompt.c_str(), boxX + 20, boxY + 10, 16, stColor);
            }
            else if (targetedRack != nullptr) {
                std::string promptText = "";
                Color promptBg = { 20, 25, 30, 220 };
                Color promptTextColor = RAYWHITE;

                if (!player.IsHoldingProduct()) {
                    if (targetedRack->HasStock()) {
                        promptText = "Tekan E untuk mengambil " + targetedRack->GetProductName() + 
                                     " (Stok: " + std::to_string(targetedRack->GetStock()) + "/" + std::to_string(targetedRack->GetMaxStock()) + 
                                     " | Rp" + std::to_string(GetProductInfo(targetedRack->GetProductType()).sellPrice) + ")";
                        promptTextColor = { 100, 255, 100, 255 };
                    } else {
                        promptText = "[Stok Rak " + targetedRack->GetProductName() + " Kosong - Ambil dari Storage/Supplier]";
                        promptTextColor = { 255, 120, 120, 255 };
                    }
                } else {
                    if (targetedRack->GetProductType() == player.GetHeldProduct()) {
                        if (!targetedRack->IsFull()) {
                            promptText = "Tekan E untuk Restock " + player.GetHeldProductName() + 
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
                std::string cashierInfo = "Meja Kasir Toko";
                int textWidth = MeasureText(cashierInfo.c_str(), 16);
                int boxX = (screenWidth - textWidth) / 2 - 15;
                int boxY = screenHeight / 2 + 50;
                DrawRectangle(boxX, boxY, textWidth + 30, 32, { 20, 25, 30, 200 });
                DrawRectangleLines(boxX, boxY, textWidth + 30, 32, { 100, 180, 255, 255 });
                DrawText(cashierInfo.c_str(), boxX + 15, boxY + 8, 16, { 150, 210, 255, 255 });
            }

            // Top Notification Banner
            if (topNoticeTimer > 0.0f) {
                int noticeWidth = MeasureText(topNotice.c_str(), 18);
                int nBoxX = (screenWidth - noticeWidth) / 2 - 25;
                int nBoxY = 25;

                DrawRectangle(nBoxX, nBoxY, noticeWidth + 50, 42, topNoticeColor);
                DrawRectangleLines(nBoxX, nBoxY, noticeWidth + 50, 42, RAYWHITE);
                DrawText(topNotice.c_str(), nBoxX + 25, nBoxY + 12, 18, RAYWHITE);
            }

            // Temporary Warning / Feedback Banner (Center)
            if (player.HasFeedbackMessage()) {
                std::string msg = player.GetFeedbackMessage();
                int msgWidth = MeasureText(msg.c_str(), 18);
                int msgBoxX = (screenWidth - msgWidth) / 2 - 25;
                int msgBoxY = screenHeight / 2 - 80;

                DrawRectangle(msgBoxX, msgBoxY, msgWidth + 50, 40, { 180, 40, 40, 230 });
                DrawRectangleLines(msgBoxX, msgBoxY, msgWidth + 50, 40, { 255, 220, 220, 255 });
                DrawText(msg.c_str(), msgBoxX + 25, msgBoxY + 11, 18, RAYWHITE);
            }

            // Active Deliveries HUD Widget (Top Right)
            if (supplier.HasActiveOrders()) {
                const auto& orders = supplier.GetActiveOrders();
                int oBoxY = 45;
                DrawRectangle(screenWidth - 280, oBoxY, 265, 30 + (int)orders.size() * 30, { 20, 30, 40, 220 });
                DrawRectangleLines(screenWidth - 280, oBoxY, 265, 30 + (int)orders.size() * 30, { 52, 152, 219, 255 });
                DrawText("STATUS PENGIRIMAN SUPPLIER", screenWidth - 270, oBoxY + 8, 13, { 100, 200, 255, 255 });

                int lineY = oBoxY + 30;
                for (const auto& ord : orders) {
                    ProductInfo info = GetProductInfo(ord.productType);
                    std::string ordText = info.name + " (" + std::to_string(ord.quantity) + "x): " +
                                          TextFormat("%.1fs", ord.deliveryTimer);
                    DrawText(ordText.c_str(), screenWidth - 270, lineY, 13, RAYWHITE);
                    lineY += 28;
                }
            }

            // Crosshair in screen center (when menu closed)
            if (!supplier.IsMenuOpen()) {
                int centerX = screenWidth / 2;
                int centerY = screenHeight / 2;
                bool isTargeting = (targetedRack != nullptr || targetedStorageProduct != ProductType::NONE);
                Color crosshairColor = isTargeting ? Color{ 50, 255, 100, 230 } : Color{ 255, 255, 255, 180 };
                DrawCircle(centerX, centerY, 3.0f, crosshairColor);
                DrawCircleLines(centerX, centerY, 7.0f, crosshairColor);
            }

            // FPS Counter in top right
            DrawFPS(screenWidth - 90, 15);

            // ==========================================
            // SUPPLIER ORDER MODAL MENU (TAB)
            // ==========================================
            if (supplier.IsMenuOpen()) {
                // Dimmed overlay background
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 600;
                int modalH = 460;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 38, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 52, 152, 219, 255 });

                DrawText("MENU PENGADAAN BARANG (SUPPLIER)", modalX + 30, modalY + 25, 20, { 255, 215, 0, 255 });
                DrawText("Pilih produk, tentukan jumlah, lalu tekan ENTER untuk order", modalX + 30, modalY + 52, 13, { 180, 190, 200, 255 });

                // Product items list
                ProductType prods[3] = { ProductType::BEVERAGE, ProductType::BREAD, ProductType::CANNED_FOOD };
                int listY = modalY + 85;

                for (int i = 0; i < 3; ++i) {
                    ProductInfo info = GetProductInfo(prods[i]);
                    bool isSelected = (supplier.GetSelectedProductIndex() == i);

                    Color itemBg = isSelected ? Color{ 40, 70, 110, 240 } : Color{ 35, 40, 48, 200 };
                    Color itemBorder = isSelected ? Color{ 0, 200, 255, 255 } : Color{ 60, 70, 80, 255 };

                    DrawRectangle(modalX + 30, listY, modalW - 60, 60, itemBg);
                    DrawRectangleLines(modalX + 30, listY, modalW - 60, 60, itemBorder);

                    // Product Icon Color Cube
                    DrawRectangle(modalX + 45, listY + 15, 30, 30, info.primaryColor);
                    DrawRectangleLines(modalX + 45, listY + 15, 30, 30, RAYWHITE);

                    // Name and Prices
                    std::string pTitle = info.name + (isSelected ? "  <-- TERPILIH" : "");
                    DrawText(pTitle.c_str(), modalX + 90, listY + 12, 16, isSelected ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                    std::string priceLine = "Harga Beli Supplier: Rp" + std::to_string(info.buyPrice) + 
                                            "  |  Harga Jual Toko: Rp" + std::to_string(info.sellPrice) +
                                            "  |  Storage: " + std::to_string(shop.GetStorage().GetStock(prods[i]));
                    DrawText(priceLine.c_str(), modalX + 90, listY + 34, 13, { 180, 200, 220, 255 });

                    listY += 68;
                }

                // Quantity selector
                ProductInfo selectedInfo = GetProductInfo(supplier.GetSelectedProductType());
                int curQty = supplier.GetOrderQuantity();
                int curTotal = selectedInfo.buyPrice * curQty;

                int qtyBoxY = modalY + 300;
                DrawRectangle(modalX + 30, qtyBoxY, modalW - 60, 65, { 20, 25, 32, 230 });
                DrawRectangleLines(modalX + 30, qtyBoxY, modalW - 60, 65, { 100, 110, 120, 255 });

                DrawText("Jumlah Pesanan:", modalX + 50, qtyBoxY + 14, 15, RAYWHITE);
                DrawText(("[ < A / D > ]  " + std::to_string(curQty) + " Unit").c_str(), modalX + 185, qtyBoxY + 12, 18, { 255, 215, 0, 255 });

                DrawText("Total Biaya:", modalX + 50, qtyBoxY + 38, 15, RAYWHITE);
                DrawText(("Rp" + std::to_string(curTotal)).c_str(), modalX + 185, qtyBoxY + 38, 16, { 50, 255, 120, 255 });

                std::string treasuryHint = "Saldo Toko: Rp" + std::to_string(shopMoney);
                DrawText(treasuryHint.c_str(), modalX + 370, qtyBoxY + 38, 14, { 100, 200, 255, 255 });

                // Footer Controls Instruction
                DrawText("[W / S / Panah] Pilih Produk    [A / D] Ubah Jumlah (+-5)    [ENTER] Beli    [TAB / ESC] Tutup",
                         modalX + 35, modalY + 395, 13, { 255, 220, 120, 255 });
            }

        EndDrawing();
    }

    // 6. Cleanup
    CloseWindow();

    return 0;
}
