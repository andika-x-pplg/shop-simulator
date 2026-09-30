#include "raylib.h"
#include "Player.hpp"
#include "Shop.hpp"
#include "Customer.hpp"
#include "Cashier.hpp"
#include "Storage.hpp"
#include "Supplier.hpp"
#include "PriceManager.hpp"
#include "Finance.hpp"
#include <string>
#include <vector>
#include <cstdlib>

int main() {
    // 1. Window Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "3D Shop Simulator - Tahap 7: Sistem Ekonomi & Manajemen Harga");

    SetTargetFPS(60);

    // Disable cursor for smooth first-person mouse controls
    DisableCursor();

    // 2. Game Entities Initialization
    Shop shop;
    shop.Init();

    Supplier supplier;
    supplier.Init();

    // Single source of truth for Store Economy (Saldo Awal: Rp100.000)
    Finance finance(100000);

    // Dynamic Price Management System
    PriceManager& priceMgr = PriceManager::Instance();
    priceMgr.Init();

    Player player;
    // Spawn player in front of shop entrance
    player.Init({ 0.0f, 0.0f, 10.0f });

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
        bool anyModalOpen = supplier.IsMenuOpen() || priceMgr.IsMenuOpen() || finance.IsMenuOpen();

        // Exit or close modals on ESC
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (anyModalOpen) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                DisableCursor();
            } else {
                break;
            }
        }

        float deltaTime = GetFrameTime();

        // -------------------------------------------------------------
        // Modal Toggles (TAB: Supplier, P: Price Manager, F: Finance)
        // -------------------------------------------------------------
        if (IsKeyPressed(KEY_TAB)) {
            bool nextState = !supplier.IsMenuOpen();
            supplier.SetMenuOpen(nextState);
            if (nextState) {
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_P)) {
            bool nextState = !priceMgr.IsMenuOpen();
            priceMgr.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_F)) {
            bool nextState = !finance.IsMenuOpen();
            finance.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        anyModalOpen = supplier.IsMenuOpen() || priceMgr.IsMenuOpen() || finance.IsMenuOpen();

        // -------------------------------------------------------------
        // Supplier Modal Inputs
        // -------------------------------------------------------------
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
                int costBefore = GetProductInfo(selType).buyPrice * qty;
                if (supplier.PlaceOrder(selType, qty, finance, errorMsg)) {
                    topNotice = "-Rp" + std::to_string(costBefore) + " Pengadaan (" + std::to_string(qty) + "x " + GetProductInfo(selType).name + ")";
                    topNoticeColor = { 210, 50, 50, 235 };
                    topNoticeTimer = 3.5f;
                } else {
                    player.SetFeedbackMessage(errorMsg, 2.5f);
                }
            }
        }
        // -------------------------------------------------------------
        // Price Management Modal Inputs (P)
        // -------------------------------------------------------------
        else if (priceMgr.IsMenuOpen()) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                priceMgr.PreviousProduct();
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                priceMgr.NextProduct();
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                ProductType curType = priceMgr.GetSelectedProductType();
                priceMgr.AdjustSellPrice(curType, 500);
                std::string fb;
                priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                topNotice = fb;
                topNoticeColor = { 40, 120, 200, 235 };
                topNoticeTimer = 3.0f;
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                ProductType curType = priceMgr.GetSelectedProductType();
                priceMgr.AdjustSellPrice(curType, -500);
                std::string fb;
                priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                topNotice = fb;
                topNoticeColor = { 40, 120, 200, 235 };
                topNoticeTimer = 3.0f;
            }
        }

        // Update Top Notification Timer
        if (topNoticeTimer > 0.0f) {
            topNoticeTimer -= deltaTime;
            if (topNoticeTimer <= 0.0f) {
                topNotice = "";
            }
        }

        // Update Game Logic (Player movement active when no modal open)
        if (!anyModalOpen) {
            player.Update(deltaTime, shop.GetColliders());
        }
        shop.Update(deltaTime);

        // Update Supplier Deliveries -> Delivery to Storage (Only adds stock, does not deduct money)
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
        if (IsKeyPressed(KEY_E) && !anyModalOpen) {
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

            // Record customer revenue exactly once through single Finance system
            if (didPay && paidAmount > 0) {
                finance.RecordRevenue(paidAmount, "Penjualan " + paidProduct + " ke " + cust.GetName());
                topNotice = "+Rp" + std::to_string(paidAmount) + " Pendapatan (" + cust.GetName() + " - " + paidProduct + ")";
                topNoticeColor = { 20, 130, 60, 235 };
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
            DrawRectangle(15, 15, 340, 290, { 15, 20, 25, 225 });
            DrawRectangleLines(15, 15, 340, 290, { 70, 85, 100, 255 });

            DrawText("SHOP SIMULATOR 3D (Tahap 7)", 25, 23, 16, { 255, 215, 0, 255 });
            DrawText("WASD     : Bergerak", 25, 44, 13, RAYWHITE);
            DrawText("Mouse    : Kontrol Kamera", 25, 62, 13, RAYWHITE);
            DrawText("E        : Interaksi Rak / Storage", 25, 80, 13, { 100, 230, 100, 255 });
            DrawText("TAB      : Menu Supplier & Order", 25, 98, 13, { 255, 180, 50, 255 });
            DrawText("P        : Manajemen Harga Jual", 25, 116, 13, { 100, 200, 255, 255 });
            DrawText("F        : Ringkasan Keuangan Toko", 25, 134, 13, { 255, 220, 80, 255 });
            DrawText("ESC      : Keluar Game / Tutup Menu", 25, 152, 13, { 255, 100, 100, 255 });

            // Carried Product Status
            std::string carriedText = "Membawa: " + player.GetHeldProductName();
            Color carriedColor = player.IsHoldingProduct() ? Color{ 255, 220, 50, 255 } : Color{ 180, 190, 200, 255 };
            DrawText(carriedText.c_str(), 25, 174, 14, carriedColor);

            // Treasury / Money Balance (HUD focused on Saldo Toko)
            std::string moneyText = "Uang Toko: Rp" + std::to_string(finance.GetCurrentBalance());
            DrawText(moneyText.c_str(), 25, 196, 17, { 50, 255, 120, 255 });

            // Storage Stock Summary
            std::string storageInfo = "Storage: Minuman " + std::to_string(shop.GetStorage().GetBeverageStock()) +
                                      " | Roti " + std::to_string(shop.GetStorage().GetBreadStock()) +
                                      " | Kaleng " + std::to_string(shop.GetStorage().GetCannedFoodStock());
            DrawText(storageInfo.c_str(), 25, 224, 12, { 255, 200, 120, 255 });

            // Customer / Cashier Status Debug
            std::string custCountText = "Customer: " + std::to_string(customers.size()) + "/" + std::to_string(maxActiveCustomers) +
                                        " | Antrian Kasir: " + std::to_string(cashierQueueCount);
            DrawText(custCountText.c_str(), 25, 244, 12, { 100, 220, 255, 255 });

            if (!customers.empty()) {
                const auto& activeCust = customers.front();
                std::string custInfo = activeCust.GetName() + " -> " + activeCust.GetStateString();
                DrawText(custInfo.c_str(), 25, 264, 12, { 200, 230, 250, 255 });
            } else {
                DrawText("Menunggu customer baru...", 25, 264, 12, { 140, 150, 160, 255 });
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

                int curSellPrice = priceMgr.GetSellPrice(targetedRack->GetProductType());

                if (!player.IsHoldingProduct()) {
                    if (targetedRack->HasStock()) {
                        promptText = "Tekan E untuk mengambil " + targetedRack->GetProductName() + 
                                     " (Stok: " + std::to_string(targetedRack->GetStock()) + "/" + std::to_string(targetedRack->GetMaxStock()) + 
                                     " | Harga Jual: Rp" + std::to_string(curSellPrice) + ")";
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
                std::string cashierInfo = "Meja Kasir Toko (NPC Kasir Aktif)";
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

            // Crosshair in screen center (when no modal open)
            if (!anyModalOpen) {
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

                int modalW = 620;
                int modalH = 470;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 38, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 52, 152, 219, 255 });

                DrawText("MENU PENGADAAN BARANG (SUPPLIER)", modalX + 30, modalY + 22, 20, { 255, 215, 0, 255 });
                DrawText("Pilih produk, tentukan jumlah, lalu tekan ENTER untuk order", modalX + 30, modalY + 48, 13, { 180, 190, 200, 255 });

                // Product items list
                const auto& prods = priceMgr.GetManagedProducts();
                int listY = modalY + 78;

                for (size_t i = 0; i < prods.size(); ++i) {
                    ProductInfo info = GetProductInfo(prods[i]);
                    bool isSelected = (supplier.GetSelectedProductIndex() == (int)i);

                    Color itemBg = isSelected ? Color{ 40, 70, 110, 240 } : Color{ 35, 40, 48, 200 };
                    Color itemBorder = isSelected ? Color{ 0, 200, 255, 255 } : Color{ 60, 70, 80, 255 };

                    DrawRectangle(modalX + 30, listY, modalW - 60, 62, itemBg);
                    DrawRectangleLines(modalX + 30, listY, modalW - 60, 62, itemBorder);

                    // Product Icon Color Cube
                    DrawRectangle(modalX + 45, listY + 16, 30, 30, info.primaryColor);
                    DrawRectangleLines(modalX + 45, listY + 16, 30, 30, RAYWHITE);

                    // Name and Prices (Supplier buy price remains constant, shop sell price is dynamic)
                    std::string pTitle = info.name + (isSelected ? "  <-- TERPILIH" : "");
                    DrawText(pTitle.c_str(), modalX + 90, listY + 12, 16, isSelected ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                    std::string priceLine = "Harga Beli Supplier: Rp" + std::to_string(info.buyPrice) + 
                                            "  |  Harga Jual Toko: Rp" + std::to_string(priceMgr.GetSellPrice(prods[i])) +
                                            "  |  Storage: " + std::to_string(shop.GetStorage().GetStock(prods[i]));
                    DrawText(priceLine.c_str(), modalX + 90, listY + 36, 13, { 180, 200, 220, 255 });

                    listY += 70;
                }

                // Quantity selector
                ProductInfo selectedInfo = GetProductInfo(supplier.GetSelectedProductType());
                int curQty = supplier.GetOrderQuantity();
                int curTotal = selectedInfo.buyPrice * curQty;

                int qtyBoxY = modalY + 305;
                DrawRectangle(modalX + 30, qtyBoxY, modalW - 60, 70, { 20, 25, 32, 230 });
                DrawRectangleLines(modalX + 30, qtyBoxY, modalW - 60, 70, { 100, 110, 120, 255 });

                DrawText("Jumlah Pesanan:", modalX + 50, qtyBoxY + 14, 15, RAYWHITE);
                DrawText(("[ < A / D > ]  " + std::to_string(curQty) + " Unit").c_str(), modalX + 185, qtyBoxY + 12, 18, { 255, 215, 0, 255 });

                DrawText("Total Biaya:", modalX + 50, qtyBoxY + 40, 15, RAYWHITE);
                DrawText(("Rp" + std::to_string(curTotal)).c_str(), modalX + 185, qtyBoxY + 40, 16, { 255, 100, 100, 255 });

                std::string treasuryHint = "Saldo Toko: Rp" + std::to_string(finance.GetCurrentBalance());
                DrawText(treasuryHint.c_str(), modalX + 370, qtyBoxY + 40, 14, { 50, 255, 120, 255 });

                // Footer Controls Instruction
                DrawText("[W / S / Panah] Pilih Produk    [A / D] Ubah Jumlah (+-5)    [ENTER] Beli    [TAB / ESC] Tutup",
                         modalX + 35, modalY + 415, 13, { 255, 220, 120, 255 });
            }

            // ==========================================
            // PRICE MANAGEMENT MODAL MENU (P)
            // ==========================================
            if (priceMgr.IsMenuOpen()) {
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 640;
                int modalH = 470;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 42, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 41, 128, 185, 255 });

                DrawText("MANAJEMEN HARGA JUAL TOKO", modalX + 30, modalY + 22, 20, { 100, 220, 255, 255 });
                DrawText("Atur harga jual ke customer. Margin dihitung otomatis (Jual - Beli)", modalX + 30, modalY + 48, 13, { 180, 195, 210, 255 });

                const auto& prods = priceMgr.GetManagedProducts();
                int listY = modalY + 80;

                for (size_t i = 0; i < prods.size(); ++i) {
                    ProductType pType = prods[i];
                    ProductInfo info = GetProductInfo(pType);
                    int sellPrice = priceMgr.GetSellPrice(pType);
                    int buyPrice = priceMgr.GetBuyPrice(pType);
                    int margin = priceMgr.GetUnitMargin(pType);

                    bool isSelected = (priceMgr.GetSelectedProductIndex() == (int)i);

                    Color itemBg = isSelected ? Color{ 35, 65, 95, 240 } : Color{ 30, 36, 45, 200 };
                    Color itemBorder = isSelected ? Color{ 0, 220, 255, 255 } : Color{ 55, 65, 75, 255 };

                    DrawRectangle(modalX + 30, listY, modalW - 60, 85, itemBg);
                    DrawRectangleLines(modalX + 30, listY, modalW - 60, 85, itemBorder);

                    // Product Color Box
                    DrawRectangle(modalX + 45, listY + 25, 35, 35, info.primaryColor);
                    DrawRectangleLines(modalX + 45, listY + 25, 35, 35, RAYWHITE);

                    // Title
                    std::string pTitle = info.name + (isSelected ? "  [Sedang Dipilih]" : "");
                    DrawText(pTitle.c_str(), modalX + 95, listY + 12, 16, isSelected ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                    // Prices row
                    std::string modalLine = "Modal: Rp" + std::to_string(buyPrice) +
                                            "    |    Jual: Rp" + std::to_string(sellPrice);
                    DrawText(modalLine.c_str(), modalX + 95, listY + 36, 14, { 220, 230, 240, 255 });

                    // Profit / Unit & Warning
                    Color marginColor = (margin >= 0) ? Color{ 50, 255, 120, 255 } : Color{ 255, 80, 80, 255 };
                    std::string marginText = "Profit/unit: " + (margin >= 0 ? ("+Rp" + std::to_string(margin)) : ("-Rp" + std::to_string(-margin)));
                    DrawText(marginText.c_str(), modalX + 95, listY + 58, 14, marginColor);

                    if (sellPrice < buyPrice) {
                        DrawText("(Peringatan: Jual di bawah modal!)", modalX + 280, listY + 58, 12, { 255, 100, 100, 255 });
                    }

                    // Adjustment controls indicator on selected item
                    if (isSelected) {
                        DrawText("[ < A / D >  +-Rp500 ]", modalX + modalW - 235, listY + 36, 13, { 255, 215, 0, 255 });
                    }

                    listY += 95;
                }

                // Footer
                DrawText("[W / S / Panah] Pilih Produk    [A / D] Ubah Harga (+-Rp500)    [P / ESC] Tutup",
                         modalX + 45, modalY + 415, 13, { 255, 220, 120, 255 });
            }

            // ==========================================
            // FINANCIAL SUMMARY MODAL MENU (F)
            // ==========================================
            if (finance.IsMenuOpen()) {
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 620;
                int modalH = 470;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 22, 28, 36, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 46, 204, 113, 255 });

                DrawText("RINGKASAN KEUANGAN TOKO", modalX + 30, modalY + 22, 20, { 255, 215, 0, 255 });
                DrawText("Data finansial real-time: Pendapatan, Pengeluaran & Profit / Loss", modalX + 30, modalY + 48, 13, { 180, 195, 210, 255 });

                // 4 Main Financial Metrics Cards in 2x2 grid
                int cardW = 265;
                int cardH = 65;

                // 1. Saldo Toko
                int c1X = modalX + 35;
                int c1Y = modalY + 80;
                DrawRectangle(c1X, c1Y, cardW, cardH, { 30, 38, 48, 240 });
                DrawRectangleLines(c1X, c1Y, cardW, cardH, { 70, 85, 100, 255 });
                DrawText("Saldo Toko Saat Ini:", c1X + 15, c1Y + 12, 13, { 180, 195, 210, 255 });
                std::string bStr = "Rp" + std::to_string(finance.GetCurrentBalance());
                DrawText(bStr.c_str(), c1X + 15, c1Y + 34, 18, { 50, 255, 120, 255 });

                // 2. Total Pendapatan (Revenue)
                int c2X = modalX + 320;
                int c2Y = modalY + 80;
                DrawRectangle(c2X, c2Y, cardW, cardH, { 30, 38, 48, 240 });
                DrawRectangleLines(c2X, c2Y, cardW, cardH, { 70, 85, 100, 255 });
                DrawText("Total Pendapatan (Revenue):", c2X + 15, c2Y + 12, 13, { 180, 195, 210, 255 });
                std::string rStr = "Rp" + std::to_string(finance.GetTotalRevenue());
                DrawText(rStr.c_str(), c2X + 15, c2Y + 34, 18, { 100, 220, 255, 255 });

                // 3. Total Pengeluaran (Expenses)
                int c3X = modalX + 35;
                int c3Y = modalY + 155;
                DrawRectangle(c3X, c3Y, cardW, cardH, { 30, 38, 48, 240 });
                DrawRectangleLines(c3X, c3Y, cardW, cardH, { 70, 85, 100, 255 });
                DrawText("Total Pengeluaran (Expenses):", c3X + 15, c3Y + 12, 13, { 180, 195, 210, 255 });
                std::string eStr = "Rp" + std::to_string(finance.GetTotalExpenses());
                DrawText(eStr.c_str(), c3X + 15, c3Y + 34, 18, { 255, 100, 100, 255 });

                // 4. Keuntungan / Kerugian (Profit = Revenue - Expenses)
                int c4X = modalX + 320;
                int c4Y = modalY + 155;
                int profit = finance.GetTotalProfit();
                DrawRectangle(c4X, c4Y, cardW, cardH, { 30, 38, 48, 240 });
                Color profitBorder = (profit >= 0) ? Color{ 46, 204, 113, 255 } : Color{ 231, 76, 60, 255 };
                DrawRectangleLines(c4X, c4Y, cardW, cardH, profitBorder);

                std::string pLabel = (profit >= 0) ? "Keuntungan Bersih (Profit):" : "Kerugian Toko (Loss):";
                DrawText(pLabel.c_str(), c4X + 15, c4Y + 12, 13, { 180, 195, 210, 255 });

                std::string pStr = (profit >= 0) ? ("+Rp" + std::to_string(profit)) : ("-Rp" + std::to_string(-profit));
                Color profitColor = (profit >= 0) ? Color{ 50, 255, 120, 255 } : Color{ 255, 80, 80, 255 };
                DrawText(pStr.c_str(), c4X + 15, c4Y + 34, 18, profitColor);

                // Recent Transactions List
                DrawText("Riwayat Transaksi Terbaru:", modalX + 35, modalY + 235, 14, { 255, 215, 0, 255 });
                int tBoxY = modalY + 258;
                DrawRectangle(modalX + 35, tBoxY, modalW - 70, 145, { 18, 22, 28, 240 });
                DrawRectangleLines(modalX + 35, tBoxY, modalW - 70, 145, { 60, 70, 80, 255 });

                const auto& txs = finance.GetRecentTransactions();
                if (txs.empty()) {
                    DrawText("Belum ada transaksi tercatat.", modalX + 50, tBoxY + 60, 13, { 140, 150, 160, 255 });
                } else {
                    int txRowY = tBoxY + 10;
                    for (size_t i = 0; i < txs.size() && i < 4; ++i) {
                        const auto& tx = txs[i];
                        bool isRev = (tx.type == "PENDAPATAN");
                        Color tCol = isRev ? Color{ 50, 255, 120, 255 } : Color{ 255, 100, 100, 255 };
                        std::string sign = isRev ? "+" : "-";

                        DrawText(tx.description.c_str(), modalX + 50, txRowY, 13, RAYWHITE);
                        std::string amtStr = sign + "Rp" + std::to_string(tx.amount);
                        DrawText(amtStr.c_str(), modalX + modalW - 200, txRowY, 13, tCol);

                        txRowY += 32;
                    }
                }

                // Footer
                DrawText("Rumus: Profit = Total Pendapatan - Total Pengeluaran    |    [F / ESC] Tutup",
                         modalX + 45, modalY + 420, 13, { 255, 220, 120, 255 });
            }

        EndDrawing();
    }

    // 6. Cleanup
    CloseWindow();

    return 0;
}

