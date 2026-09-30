#include "raylib.h"
#include "Player.hpp"
#include "Shop.hpp"
#include "Customer.hpp"
#include "Cashier.hpp"
#include "Storage.hpp"
#include "Supplier.hpp"
#include "PriceManager.hpp"
#include "Finance.hpp"
#include "Reputation.hpp"
#include "ShopUpgrade.hpp"
#include "Furniture.hpp"
#include "GameTime.hpp"
#include "DailyStats.hpp"
#include "SaveSystem.hpp"
#include "AudioManager.hpp"
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>

int main() {
    // 1. Window Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "3D Shop Simulator - Final Release");

    SetTargetFPS(60);

    // Initialize Audio Manager (Tahap 12)
    AudioManager& audioMgr = AudioManager::Instance();
    audioMgr.Init();

    // Disable cursor for smooth first-person mouse controls
    DisableCursor();

    // 2. Game Systems & Entities Initialization
    Shop shop;
    shop.Init();

    Supplier supplier;
    supplier.Init();

    // Single source of truth for Store Economy (Starting Balance: Rp100.000)
    Finance finance(100000);

    // Dynamic Price Management System
    PriceManager& priceMgr = PriceManager::Instance();
    priceMgr.Init();

    // Store Reputation & Customer Satisfaction Rating System
    Reputation reputation;
    reputation.Init(50); // Reputasi awal: 50 / 100

    // Shop Upgrade System (Size, Shelf Cap, Storage Cap, Customer Cap)
    ShopUpgrade shopUpgrade;
    shopUpgrade.Init();

    // Furniture & Equipment System (3D Models & Bonuses)
    Furniture furniture;
    furniture.Init();

    // Day & Time System (Tahap 10)
    GameTime gameTime;
    gameTime.Init();

    // Daily Statistics System (Tahap 10)
    DailyStats dailyStats;
    dailyStats.Init();

    // Save / Load System (Tahap 11)
    SaveSystem saveSystem;
    saveSystem.Init();
    const std::string defaultSaveFile = "save/savegame.json";

    Player player;
    // Spawn player in front of shop entrance
    player.Init({ 0.0f, 0.0f, 10.0f });

    // Transaction & Delivery Notification Banner
    std::string topNotice = "Toko dibuka! Selamat melayani pelanggan.";
    float topNoticeTimer = 4.0f;
    Color topNoticeColor = { 46, 204, 113, 235 };

    // 4. Customer NPC System Management (Tahap 14: Advanced Customer AI)
    std::vector<Customer> customers;
    float spawnTimer = 2.0f; // First customer arrives in 2 seconds
    int customerCounter = 1;
    bool showAiDebug = false; // Toggleable AI Debug Mode (F3)

    // Pre-defined customer profiles (name, skin tone, shirt color)
    const std::vector<std::string> customerNames = { "Budi", "Siti", "Andi", "Dewi", "Rian", "Maya", "Doni", "Putri", "Ahmad", "Citra" };
    const std::vector<Color> shirtColors = {
        { 52, 152, 219, 255 },  // Blue
        { 231, 76, 60, 255 },   // Red
        { 46, 204, 113, 255 },  // Green
        { 155, 89, 182, 255 },  // Purple
        { 241, 196, 15, 255 },  // Yellow
        { 230, 126, 34, 255 },  // Orange
        { 26, 188, 156, 255 },  // Turquoise
        { 236, 240, 241, 255 }   // Light Gray
    };
    const std::vector<Color> skinColors = {
        { 255, 220, 185, 255 },
        { 240, 200, 160, 255 },
        { 210, 165, 130, 255 },
        { 180, 135, 100, 255 }
    };

    // Helper for resetting to clean new game state without deleting file
    auto ResetToNewGame = [&]() {
        shop.Init();
        supplier.Init();
        finance.Init(100000);
        priceMgr.Init();
        reputation.Init(50);
        shopUpgrade.Init();
        furniture.Init();
        gameTime.Init();
        dailyStats.Init();
        player.Init({ 0.0f, 0.0f, 10.0f });
        customers.clear();
        spawnTimer = 2.0f;
        customerCounter = 1;
        topNotice = "Permainan Baru Dimulai! (Day 1 - 08:00)";
        topNoticeColor = { 46, 204, 113, 235 };
        topNoticeTimer = 4.0f;
        audioMgr.PlayEvent(SoundEvent::NOTIFICATION);
    };

    // 5. Main Game Loop
    while (!WindowShouldClose()) {
        bool anyModalOpen = supplier.IsMenuOpen() || priceMgr.IsMenuOpen() || 
                             finance.IsMenuOpen() || reputation.IsMenuOpen() ||
                             shopUpgrade.IsMenuOpen() || furniture.IsMenuOpen() ||
                             gameTime.IsDaySummaryOpen() || saveSystem.IsMenuOpen();

        // Exit or close modals on ESC (or open Game Menu if in normal play)
        if (IsKeyPressed(KEY_ESCAPE)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            if (saveSystem.IsMenuOpen()) {
                saveSystem.SetMenuOpen(false);
                DisableCursor();
            } else if (gameTime.IsDaySummaryOpen()) {
                gameTime.SetDaySummaryOpen(false);
                DisableCursor();
            } else if (anyModalOpen) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                reputation.SetMenuOpen(false);
                shopUpgrade.SetMenuOpen(false);
                furniture.SetMenuOpen(false);
                DisableCursor();
            } else {
                // Open Game Menu / Save Modal on ESC during gameplay
                saveSystem.SetMenuOpen(true);
                EnableCursor();
            }
        }

        float deltaTime = GetFrameTime();

        // Update Game Time & Check Warnings / Auto-close (Tahap 10)
        std::string timeNotice = "";
        Color timeNoticeCol = RAYWHITE;
        gameTime.Update(deltaTime, timeNotice, timeNoticeCol);
        if (!timeNotice.empty()) {
            topNotice = timeNotice;
            topNoticeColor = timeNoticeCol;
            topNoticeTimer = 4.0f;
            audioMgr.PlayEvent(SoundEvent::NOTIFICATION);

            if (gameTime.IsDaySummaryOpen()) {
                // Trigger Auto-save on day end if closing
                std::string autoSaveMsg;
                saveSystem.SaveGame(defaultSaveFile, player, shop, finance, priceMgr, reputation, shopUpgrade, furniture, supplier, gameTime, dailyStats, autoSaveMsg);
                EnableCursor();
            }
        }

        // -------------------------------------------------------------
        // Daily Summary Modal Handling (Tahap 10)
        // -------------------------------------------------------------
        if (gameTime.IsDaySummaryOpen()) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                audioMgr.PlayEvent(SoundEvent::DOOR);
                // Advance to Next Day
                std::string nextDayNotice;
                Color nextDayCol;
                gameTime.StartNextDay(nextDayNotice, nextDayCol);
                dailyStats.ResetDaily();
                priceMgr.ResetDailyStats();

                // Trigger Autosave on new day start
                std::string autoSaveMsg;
                saveSystem.SaveGame(defaultSaveFile, player, shop, finance, priceMgr, reputation, shopUpgrade, furniture, supplier, gameTime, dailyStats, autoSaveMsg);

                topNotice = nextDayNotice;
                topNoticeColor = nextDayCol;
                topNoticeTimer = 4.0f;

                // Reset customer spawn timer for the fresh day
                spawnTimer = 2.0f;
                DisableCursor();
            }
        }

        // -------------------------------------------------------------
        // Game Menu & Save/Load Modal (F5: Quick Save, F9: Quick Load, M: Game Menu)
        // -------------------------------------------------------------
        if (IsKeyPressed(KEY_M)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            bool nextState = !saveSystem.IsMenuOpen();
            saveSystem.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                reputation.SetMenuOpen(false);
                shopUpgrade.SetMenuOpen(false);
                furniture.SetMenuOpen(false);
                gameTime.SetDaySummaryOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        // Quick Save (F5)
        if (IsKeyPressed(KEY_F5)) {
            std::string msg;
            if (saveSystem.SaveGame(defaultSaveFile, player, shop, finance, priceMgr, reputation, shopUpgrade, furniture, supplier, gameTime, dailyStats, msg)) {
                topNotice = "Quick Save: " + msg;
                topNoticeColor = { 46, 204, 113, 235 };
                audioMgr.PlayEvent(SoundEvent::NOTIFICATION);
            } else {
                topNotice = "Quick Save Gagal!";
                topNoticeColor = { 231, 76, 60, 235 };
            }
            topNoticeTimer = 3.5f;
        }

        // Quick Load (F9)
        if (IsKeyPressed(KEY_F9)) {
            std::string msg;
            if (saveSystem.LoadGame(defaultSaveFile, player, shop, finance, priceMgr, reputation, shopUpgrade, furniture, supplier, gameTime, dailyStats, msg)) {
                customers.clear(); // Despawn active customer safely
                spawnTimer = 2.0f;
                topNotice = "Quick Load: " + msg;
                topNoticeColor = { 52, 152, 219, 235 };
                audioMgr.PlayEvent(SoundEvent::NOTIFICATION);
            } else {
                topNotice = msg;
                topNoticeColor = { 231, 76, 60, 235 };
            }
            topNoticeTimer = 3.5f;
        }

        // Toggle Customer AI Debug Overlay (F3) (Tahap 14)
        if (IsKeyPressed(KEY_F3)) {
            showAiDebug = !showAiDebug;
            topNotice = showAiDebug ? "AI Debug Overlay: AKTIF" : "AI Debug Overlay: NON-AKTIF";
            topNoticeColor = { 100, 200, 255, 235 };
            topNoticeTimer = 2.0f;
            audioMgr.PlayEvent(SoundEvent::CLICK);
        }

        // -------------------------------------------------------------
        // Modal Toggles (TAB: Supplier, P: Price, F: Finance, R: Reputation, U: Upgrade, B: Furniture)
        // -------------------------------------------------------------
        if (IsKeyPressed(KEY_TAB)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            bool nextState = !supplier.IsMenuOpen();
            supplier.SetMenuOpen(nextState);
            if (nextState) {
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                reputation.SetMenuOpen(false);
                shopUpgrade.SetMenuOpen(false);
                furniture.SetMenuOpen(false);
                gameTime.SetDaySummaryOpen(false);
                saveSystem.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_P)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            bool nextState = !priceMgr.IsMenuOpen();
            priceMgr.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                reputation.SetMenuOpen(false);
                shopUpgrade.SetMenuOpen(false);
                furniture.SetMenuOpen(false);
                gameTime.SetDaySummaryOpen(false);
                saveSystem.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_F)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            bool nextState = !finance.IsMenuOpen();
            finance.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                reputation.SetMenuOpen(false);
                shopUpgrade.SetMenuOpen(false);
                furniture.SetMenuOpen(false);
                gameTime.SetDaySummaryOpen(false);
                saveSystem.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_R)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            bool nextState = !reputation.IsMenuOpen();
            reputation.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                shopUpgrade.SetMenuOpen(false);
                furniture.SetMenuOpen(false);
                gameTime.SetDaySummaryOpen(false);
                saveSystem.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_U)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            bool nextState = !shopUpgrade.IsMenuOpen();
            shopUpgrade.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                reputation.SetMenuOpen(false);
                furniture.SetMenuOpen(false);
                gameTime.SetDaySummaryOpen(false);
                saveSystem.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        if (IsKeyPressed(KEY_B)) {
            audioMgr.PlayEvent(SoundEvent::CLICK);
            bool nextState = !furniture.IsMenuOpen();
            furniture.SetMenuOpen(nextState);
            if (nextState) {
                supplier.SetMenuOpen(false);
                priceMgr.SetMenuOpen(false);
                finance.SetMenuOpen(false);
                reputation.SetMenuOpen(false);
                shopUpgrade.SetMenuOpen(false);
                gameTime.SetDaySummaryOpen(false);
                saveSystem.SetMenuOpen(false);
                EnableCursor();
            } else {
                DisableCursor();
            }
        }

        anyModalOpen = supplier.IsMenuOpen() || priceMgr.IsMenuOpen() || 
                       finance.IsMenuOpen() || reputation.IsMenuOpen() ||
                       shopUpgrade.IsMenuOpen() || furniture.IsMenuOpen() ||
                       gameTime.IsDaySummaryOpen() || saveSystem.IsMenuOpen();

        // -------------------------------------------------------------
        // Save System Modal Inputs
        // -------------------------------------------------------------
        if (saveSystem.IsMenuOpen()) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                saveSystem.PreviousAction();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                saveSystem.NextAction();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                int action = saveSystem.GetSelectedActionIndex();
                if (action == 0) {
                    // Continue
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                    saveSystem.SetMenuOpen(false);
                    DisableCursor();
                } else if (action == 1) {
                    // Save Game
                    std::string msg;
                    if (saveSystem.SaveGame(defaultSaveFile, player, shop, finance, priceMgr, reputation, shopUpgrade, furniture, supplier, gameTime, dailyStats, msg)) {
                        topNotice = msg;
                        topNoticeColor = { 46, 204, 113, 235 };
                        audioMgr.PlayEvent(SoundEvent::NOTIFICATION);
                    } else {
                        topNotice = msg;
                        topNoticeColor = { 231, 76, 60, 235 };
                    }
                    topNoticeTimer = 3.5f;
                    saveSystem.SetMenuOpen(false);
                    DisableCursor();
                } else if (action == 2) {
                    // Load Game
                    std::string msg;
                    if (saveSystem.LoadGame(defaultSaveFile, player, shop, finance, priceMgr, reputation, shopUpgrade, furniture, supplier, gameTime, dailyStats, msg)) {
                        customers.clear(); // Safely reset current customers
                        spawnTimer = 2.0f;
                        topNotice = msg;
                        topNoticeColor = { 52, 152, 219, 235 };
                        audioMgr.PlayEvent(SoundEvent::NOTIFICATION);
                    } else {
                        topNotice = msg;
                        topNoticeColor = { 231, 76, 60, 235 };
                    }
                    topNoticeTimer = 3.5f;
                    saveSystem.SetMenuOpen(false);
                    DisableCursor();
                } else if (action == 3) {
                    // New Game
                    ResetToNewGame();
                    saveSystem.SetMenuOpen(false);
                    DisableCursor();
                } else if (action == 4) {
                    // Exit
                    break;
                }
            }
        }
        // -------------------------------------------------------------
        // Supplier Modal Inputs
        // -------------------------------------------------------------
        else if (supplier.IsMenuOpen()) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                supplier.PreviousProduct();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                supplier.NextProduct();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                supplier.IncreaseQuantity(5);
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                supplier.DecreaseQuantity(5);
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                ProductType selType = supplier.GetSelectedProductType();
                int qty = supplier.GetOrderQuantity();
                std::string errorMsg = "";
                int costBefore = GetProductInfo(selType).buyPrice * qty;
                if (supplier.PlaceOrder(selType, qty, finance, errorMsg)) {
                    dailyStats.RecordExpense(costBefore); // Record daily stats
                    topNotice = "-Rp" + std::to_string(costBefore) + " Pengadaan (" + std::to_string(qty) + "x " + GetProductInfo(selType).name + ")";
                    topNoticeColor = { 210, 50, 50, 235 };
                    topNoticeTimer = 3.5f;
                    audioMgr.PlayEvent(SoundEvent::PURCHASE);
                } else {
                    player.SetFeedbackMessage(errorMsg, 2.5f);
                }
            }
        }
        // -------------------------------------------------------------
        // Price Management Modal Inputs (P)
        // -------------------------------------------------------------
        else if (priceMgr.IsMenuOpen()) {
            if (priceMgr.IsEditingPrice()) {
                // Number keys 0-9 (Main keyboard & Numpad)
                for (int key = KEY_ZERO; key <= KEY_NINE; ++key) {
                    if (IsKeyPressed(key)) {
                        priceMgr.AppendCharToInput('0' + (key - KEY_ZERO));
                        audioMgr.PlayEvent(SoundEvent::CLICK);
                    }
                }
                for (int key = KEY_KP_0; key <= KEY_KP_9; ++key) {
                    if (IsKeyPressed(key)) {
                        priceMgr.AppendCharToInput('0' + (key - KEY_KP_0));
                        audioMgr.PlayEvent(SoundEvent::CLICK);
                    }
                }

                // Backspace
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    priceMgr.BackspaceInput();
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }

                // Confirm Input
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                    std::string fb;
                    if (priceMgr.ConfirmEditingPrice(fb)) {
                        topNotice = fb;
                        topNoticeColor = { 46, 204, 113, 235 };
                        topNoticeTimer = 3.5f;
                        audioMgr.PlayEvent(SoundEvent::PURCHASE);
                    } else {
                        player.SetFeedbackMessage(fb, 2.5f);
                        audioMgr.PlayEvent(SoundEvent::CLICK);
                    }
                }

                // Cancel direct typing
                if (IsKeyPressed(KEY_ESCAPE)) {
                    priceMgr.CancelEditingPrice();
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
            } else {
                // Navigation mode
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                    priceMgr.PreviousProduct();
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                    priceMgr.NextProduct();
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }

                // Quick Increment / Decrement (Fixed +-Rp500)
                if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.AdjustSellPrice(curType, 500);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 40, 120, 200, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
                if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.AdjustSellPrice(curType, -500);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 40, 120, 200, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }

                // Percentage Markup Adjust (+-5% with Q / E)
                if (IsKeyPressed(KEY_E)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.AdjustMarkupPercent(curType, 5.0f);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 46, 204, 113, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
                if (IsKeyPressed(KEY_Q)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.AdjustMarkupPercent(curType, -5.0f);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 241, 196, 15, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }

                // Preset Markup jumps with 1, 2, 3, 4, 5 (0%, +10%, +25%, +50%, +100%)
                if (IsKeyPressed(KEY_ONE)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.SetMarkupPercent(curType, 0.0f);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 40, 120, 200, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
                if (IsKeyPressed(KEY_TWO)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.SetMarkupPercent(curType, 10.0f);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 40, 120, 200, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
                if (IsKeyPressed(KEY_THREE)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.SetMarkupPercent(curType, 25.0f);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 40, 120, 200, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
                if (IsKeyPressed(KEY_FOUR)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.SetMarkupPercent(curType, 50.0f);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 40, 120, 200, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
                if (IsKeyPressed(KEY_FIVE)) {
                    ProductType curType = priceMgr.GetSelectedProductType();
                    priceMgr.SetMarkupPercent(curType, 100.0f);
                    std::string fb;
                    priceMgr.SetSellPrice(curType, priceMgr.GetSellPrice(curType), fb);
                    topNotice = fb;
                    topNoticeColor = { 231, 76, 60, 235 };
                    topNoticeTimer = 3.0f;
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }

                // Enter Direct Price Input Mode
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    priceMgr.StartEditingPrice();
                    audioMgr.PlayEvent(SoundEvent::CLICK);
                }
            }
        }
        // -------------------------------------------------------------
        // Shop Upgrade Modal Inputs (U)
        // -------------------------------------------------------------
        else if (shopUpgrade.IsMenuOpen()) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                shopUpgrade.PreviousUpgrade();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                shopUpgrade.NextUpgrade();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                UpgradeType uType = shopUpgrade.GetSelectedUpgradeType();
                int cost = shopUpgrade.GetNextUpgradeCost(uType);
                std::string fb;
                if (shopUpgrade.PurchaseUpgrade(uType, finance, shop, fb)) {
                    dailyStats.RecordExpense(cost); // Record daily stats
                    topNotice = fb;
                    topNoticeColor = { 46, 204, 113, 235 };
                    topNoticeTimer = 3.5f;
                    audioMgr.PlayEvent(SoundEvent::UPGRADE);

                    // Re-apply equipment bonuses if shelf/storage upgraded
                    int baseShelf = shopUpgrade.GetShelfCapacity() + furniture.GetEquipmentShelfBonus();
                    for (auto& r : shop.GetRacks()) {
                        r.SetMaxStock(baseShelf);
                    }
                    int baseStorage = shopUpgrade.GetStorageCapacity() + furniture.GetEquipmentStorageBonus();
                    shop.GetStorage().SetMaxCapacity(baseStorage);
                } else {
                    player.SetFeedbackMessage(fb, 2.5f);
                }
            }
        }
        // -------------------------------------------------------------
        // Furniture & Equipment Modal Inputs (B)
        // -------------------------------------------------------------
        else if (furniture.IsMenuOpen()) {
            if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E)) {
                furniture.SwitchTab();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                furniture.PreviousItem();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                furniture.NextItem();
                audioMgr.PlayEvent(SoundEvent::CLICK);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                std::string fb;
                bool success = false;
                if (furniture.GetSelectedTabIndex() == 0) {
                    int idx = furniture.GetSelectedIndex();
                    int price = furniture.GetFurnitureList()[idx].price;
                    success = furniture.PurchaseFurniture(idx, finance, fb);
                    if (success) {
                        dailyStats.RecordExpense(price);
                    }
                } else {
                    int idx = furniture.GetSelectedIndex();
                    int price = furniture.GetEquipmentList()[idx].price;
                    success = furniture.PurchaseEquipment(idx, finance, fb);
                    if (success) {
                        dailyStats.RecordExpense(price);
                        // Apply equipment bonuses
                        int baseShelf = shopUpgrade.GetShelfCapacity() + furniture.GetEquipmentShelfBonus();
                        for (auto& r : shop.GetRacks()) {
                            r.SetMaxStock(baseShelf);
                        }
                        int baseStorage = shopUpgrade.GetStorageCapacity() + furniture.GetEquipmentStorageBonus();
                        shop.GetStorage().SetMaxCapacity(baseStorage);
                    }
                }

                if (success) {
                    topNotice = fb;
                    topNoticeColor = { 46, 204, 113, 235 };
                    topNoticeTimer = 3.5f;
                    audioMgr.PlayEvent(SoundEvent::PURCHASE);
                } else {
                    player.SetFeedbackMessage(fb, 2.5f);
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

        // Update Game Logic (Player movement active when no modal open)
        if (!anyModalOpen) {
            std::vector<AABB> combinedColliders = shop.GetColliders();
            auto furnColliders = furniture.GetColliders();
            combinedColliders.insert(combinedColliders.end(), furnColliders.begin(), furnColliders.end());

            player.Update(deltaTime, combinedColliders);
        }
        shop.Update(deltaTime);

        // Update Supplier Deliveries -> Delivery to Storage (Active whether shop is open or closed)
        std::vector<std::string> deliveredNotices;
        supplier.Update(deltaTime, shop.GetStorage(), deliveredNotices);
        for (const auto& notice : deliveredNotices) {
            topNotice = notice;
            topNoticeColor = { 20, 140, 60, 230 };
            topNoticeTimer = 4.0f;
            audioMgr.PlayEvent(SoundEvent::NOTIFICATION);
        }

        // Check Interaction Targets (Look ray to Rack or Storage Pallets within 3.5m)
        Rack* targetedRack = shop.GetTargetedRack(player.GetEyePosition(), player.GetLookDirection(), 3.5f);
        ProductType targetedStorageProduct = shop.GetStorage().GetTargetedProduct(player.GetPosition(), player.GetLookDirection(), 3.5f);

        // Interaction Key 'E' Handling (Active even when shop is closed, e.g. for restocking)
        if (IsKeyPressed(KEY_E) && !anyModalOpen) {
            // 1. Interaction with Storage Pallets (Taking stock from Storage to Restock)
            if (targetedStorageProduct != ProductType::NONE) {
                if (!player.IsHoldingProduct()) {
                    if (shop.GetStorage().TakeStock(targetedStorageProduct)) {
                        player.PickUpProduct(targetedStorageProduct);
                        player.SetFeedbackMessage("Mengambil " + GetProductInfo(targetedStorageProduct).name + " dari Storage", 1.5f);
                        audioMgr.PlayEvent(SoundEvent::PICKUP);
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
                        audioMgr.PlayEvent(SoundEvent::PUTDOWN);
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
                            audioMgr.PlayEvent(SoundEvent::PICKUP);
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
                                audioMgr.PlayEvent(SoundEvent::PUTDOWN);
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

        // Customer Spawner & State Management (Tahap 14: Advanced Customer AI)
        // ONLY SPAWN NEW CUSTOMERS IF SHOP IS OPEN (Tahap 10)
        size_t maxCustCapacity = shopUpgrade.GetMaxActiveCustomers();
        if (gameTime.IsShopOpen()) {
            spawnTimer -= deltaTime;
            if (spawnTimer <= 0.0f && customers.size() < maxCustCapacity) {
                // Spawn a new customer outside
                float spawnX = (customerCounter % 2 == 0) ? 3.0f : -3.0f;
                Vector3 spawnPos = { spawnX, 0.0f, 16.0f + (customerCounter % 3) * 1.5f };

                std::string cName = customerNames[customerCounter % customerNames.size()];
                Color cShirt = shirtColors[customerCounter % shirtColors.size()];
                Color cSkin = skinColors[customerCounter % skinColors.size()];

                // Determine customer personality type (Tahap 14)
                CustomerType cType = CustomerType::NORMAL;
                int typeRoll = customerCounter % 5;
                if (typeRoll == 1) cType = CustomerType::IMPATIENT;
                else if (typeRoll == 2) cType = CustomerType::PATIENT;
                else if (typeRoll == 3) cType = CustomerType::BIG_SHOPPER;
                else if (typeRoll == 4) cType = CustomerType::PRICE_SENSITIVE;

                Customer newCust(customerCounter, cName, cType, spawnPos, cSkin, cShirt);
                customers.push_back(newCust);
                customerCounter++;
                
                // Spawn frequency influenced by store reputation (Higher reputation = faster customer flow)
                float repFactor = (float)reputation.GetReputation() / 100.0f; // 0.0 to 1.0
                spawnTimer = (5.5f - (repFactor * 2.2f)) + (customerCounter % 3) * 0.8f;
            }
        }

        // Apply separation avoidance to prevent severe clustering (Tahap 14)
        for (auto& cust : customers) {
            cust.ApplySeparation(customers, deltaTime);
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
            std::string paidProductSummary = "";

            cust.Update(deltaTime, shop, qIndex, didPay, paidAmount, paidProductSummary);

            // Record customer revenue exactly once through single Finance system & Daily Stats
            if (didPay && paidAmount > 0) {
                finance.RecordRevenue(paidAmount, "Penjualan [" + paidProductSummary + "] ke " + cust.GetName());
                dailyStats.RecordRevenue(paidAmount);
                dailyStats.IncrementCustomerServed();

                // Track individual product sales statistics in PriceManager (Tahap 15)
                for (auto pType : cust.GetCarriedItems()) {
                    int unitPrice = priceMgr.GetSellPrice(pType);
                    priceMgr.RecordSale(pType, 1, unitPrice);
                }

                topNotice = "+Rp" + std::to_string(paidAmount) + " Pendapatan (" + cust.GetName() + ": " + paidProductSummary + ")";
                topNoticeColor = { 20, 130, 60, 235 };
                topNoticeTimer = 3.5f;

                // Cash Register Audio Effect (Tahap 12)
                audioMgr.PlayEvent(SoundEvent::CASH_REGISTER);
            }

            // Customer leaves / exits shop -> Record rating exactly once
            if ((cust.GetState() == CustomerState::LEAVING || cust.GetState() == CustomerState::EXITING || cust.GetState() == CustomerState::DESPAWNED) && !cust.HasGivenRating()) {
                cust.MarkRatingGiven();
                int stars = 0;
                std::string feedback = "";
                
                // Extra satisfaction bonus if better cashier equipment owned (+5%)
                int finalSatisfaction = cust.GetSatisfaction();
                if (furniture.HasBetterCashierEquipment() && cust.DidSuccessfullyBuy()) {
                    finalSatisfaction = std::min(100, finalSatisfaction + 5);
                }

                // Use custom feedback from Advanced AI (Tahap 14)
                std::string customFb = cust.GetFeedbackMessage();
                reputation.RecordRating(cust.GetId(), cust.GetName(), finalSatisfaction, cust.GetCarriedSummaryString(), cust.DidSuccessfullyBuy(), stars, feedback);
                if (!customFb.empty()) {
                    feedback = customFb;
                }
                dailyStats.RecordRating(cust.GetName(), stars, finalSatisfaction, feedback);

                // Rating & feedback notification
                topNotice = cust.GetName() + " [" + cust.GetCustomerTypeString() + "] rating " + std::to_string(stars) + "/5 (" + feedback + ")";
                topNoticeColor = (stars >= 4) ? Color{ 46, 204, 113, 235 } : (stars == 3) ? Color{ 243, 156, 18, 235 } : Color{ 231, 76, 60, 235 };
                topNoticeTimer = 3.8f;
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
            ClearBackground(gameTime.GetSkyColor()); // Dynamic Sky color based on morning/day/evening

            // 3D Rendering Mode
            BeginMode3D(player.GetCamera());
                shop.Render();
                furniture.Render(); // 3D Furniture & Decorations
                for (auto& cust : customers) {
                    cust.Render();
                }
                player.RenderHeldItem();
            EndMode3D();

            // Render 2D Overhead Speech Bubbles for Customers
            for (auto& cust : customers) {
                cust.RenderSpeechBubble2D(player.GetCamera(), screenWidth, screenHeight);
            }

            // 2D HUD / UI Rendering
            // Top-Left Controls & Status Box
            DrawRectangle(15, 15, 345, 410, { 15, 20, 25, 230 });
            DrawRectangleLines(15, 15, 345, 410, { 70, 85, 100, 255 });

            DrawText("SHOP SIMULATOR 3D (Final Release)", 25, 23, 16, { 255, 215, 0, 255 });
            
            // Time & Shop Open/Closed Banner (Tahap 10)
            std::string timeHud = gameTime.GetDayString() + "  |  " + gameTime.GetFormattedTime();
            DrawText(timeHud.c_str(), 25, 45, 16, { 255, 255, 255, 255 });

            bool isShopOpen = gameTime.IsShopOpen();
            std::string shopStatusText = isShopOpen ? "STATUS TOKO: OPEN (08:00 - 21:00)" : "STATUS TOKO: CLOSED";
            Color shopStatusCol = isShopOpen ? Color{ 50, 255, 120, 255 } : Color{ 255, 80, 80, 255 };
            DrawText(shopStatusText.c_str(), 25, 65, 13, shopStatusCol);

            DrawText("WASD     : Bergerak", 25, 85, 13, RAYWHITE);
            DrawText("Mouse    : Kontrol Kamera", 25, 103, 13, RAYWHITE);
            DrawText("E        : Interaksi Rak / Storage", 25, 121, 13, { 100, 230, 100, 255 });
            DrawText("M / ESC  : Game Menu & Save/Load", 25, 139, 13, { 255, 215, 0, 255 });
            DrawText("F5 / F9  : Quick Save / Quick Load", 25, 157, 13, { 100, 220, 255, 255 });
            DrawText("TAB      : Menu Supplier & Order", 25, 175, 13, { 255, 180, 50, 255 });
            DrawText("P        : Manajemen Harga Jual", 25, 193, 13, { 100, 200, 255, 255 });
            DrawText("F        : Ringkasan Keuangan Toko", 25, 211, 13, { 255, 220, 80, 255 });
            DrawText("R        : Reputasi & Rating Toko", 25, 229, 13, { 241, 196, 15, 255 });
            DrawText("U        : Upgrade Toko (Shop Upgrade)", 25, 247, 13, { 52, 152, 219, 255 });
            DrawText("B        : Beli Furniture & Equipment", 25, 265, 13, { 230, 126, 34, 255 });

            // Carried Product Status
            std::string carriedText = "Membawa: " + player.GetHeldProductName();
            Color carriedColor = player.IsHoldingProduct() ? Color{ 255, 220, 50, 255 } : Color{ 180, 190, 200, 255 };
            DrawText(carriedText.c_str(), 25, 287, 14, carriedColor);

            // Treasury / Money Balance & Levels HUD
            std::string moneyText = "Uang Toko: Rp" + std::to_string(finance.GetCurrentBalance());
            DrawText(moneyText.c_str(), 25, 307, 16, { 50, 255, 120, 255 });

            // Shop & Upgrade Levels Summary on HUD
            std::string levelSummary = "Toko: Lvl " + std::to_string(shopUpgrade.GetShopSizeLevel()) +
                                        " | Rak: Lvl " + std::to_string(shopUpgrade.GetLevel(UpgradeType::SHELF_CAPACITY)) +
                                        " | Gudang: Lvl " + std::to_string(shopUpgrade.GetLevel(UpgradeType::STORAGE_CAPACITY));
            DrawText(levelSummary.c_str(), 25, 329, 12, { 100, 220, 255, 255 });

            // Rating & Reputation Indicators on HUD
            std::string repHudText = "";
            if (reputation.HasRatings()) {
                repHudText = "Rating: " + std::string(TextFormat("%.1f/5", reputation.GetAverageRating())) +
                             " | Reputasi: " + std::to_string(reputation.GetReputation()) + "/100";
            } else {
                repHudText = "Rating: Belum ada | Reputasi: " + std::to_string(reputation.GetReputation()) + "/100";
            }
            DrawText(repHudText.c_str(), 25, 349, 12, { 255, 215, 0, 255 });

            // Storage Stock Summary
            std::string storageInfo = "Storage: Total " + std::to_string(shop.GetStorage().GetTotalStock()) +
                                      " / " + std::to_string(shop.GetStorage().GetMaxCapacity()) + " unit (TAB: Pengadaan)";
            DrawText(storageInfo.c_str(), 25, 367, 12, { 255, 200, 120, 255 });

            // Customer / Cashier Status Debug
            std::string custCountText = "Customer: " + std::to_string(customers.size()) + "/" + std::to_string(maxCustCapacity) +
                                        " | Antrian Kasir: " + std::to_string(cashierQueueCount);
            DrawText(custCountText.c_str(), 25, 385, 12, { 200, 230, 250, 255 });

            // Center Interaction Prompt (When player aims at rack or storage pallet)
            if (targetedStorageProduct != ProductType::NONE) {
                std::string stPrompt = "";
                Color stColor = { 255, 200, 100, 255 };
                std::string pName = GetProductInfo(targetedStorageProduct).name;
                int stStock = shop.GetStorage().GetStock(targetedStorageProduct);

                if (!player.IsHoldingProduct()) {
                    if (stStock > 0) {
                        stPrompt = "[E] Ambil " + pName + " dari Storage (Tersedia: " + std::to_string(stStock) + ")";
                        stColor = { 100, 255, 120, 255 };
                    } else {
                        stPrompt = "[Storage " + pName + " Kosong - Beli di Supplier (TAB)]";
                        stColor = { 255, 140, 100, 255 };
                    }
                } else {
                    if (player.GetHeldProduct() == targetedStorageProduct) {
                        stPrompt = "[E] Taruh kembali " + pName + " ke Storage";
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
                        promptText = "[E] Ambil " + targetedRack->GetProductName() + 
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
                            promptText = "[E] Restock " + player.GetHeldProductName() + 
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
                std::string cashierInfo = "[Kasir] Meja Kasir Toko (NPC Kasir Siaga)";
                int textWidth = MeasureText(cashierInfo.c_str(), 16);
                int boxX = (screenWidth - textWidth) / 2 - 15;
                int boxY = screenHeight / 2 + 50;
                DrawRectangle(boxX, boxY, textWidth + 30, 32, { 20, 25, 30, 200 });
                DrawRectangleLines(boxX, boxY, textWidth + 30, 32, { 100, 180, 255, 255 });
                DrawText(cashierInfo.c_str(), boxX + 15, boxY + 8, 16, { 150, 210, 255, 255 });
            }

            // Top Notification Banner (Polished with subtle alpha fade)
            if (topNoticeTimer > 0.0f) {
                float alphaFactor = std::min(1.0f, topNoticeTimer * 2.0f);
                Color fadedBg = topNoticeColor;
                fadedBg.a = (unsigned char)(topNoticeColor.a * alphaFactor);
                Color fadedBorder = RAYWHITE;
                fadedBorder.a = (unsigned char)(255 * alphaFactor);

                int noticeWidth = MeasureText(topNotice.c_str(), 18);
                int nBoxX = (screenWidth - noticeWidth) / 2 - 25;
                int nBoxY = 25;

                DrawRectangle(nBoxX, nBoxY, noticeWidth + 50, 42, fadedBg);
                DrawRectangleLines(nBoxX, nBoxY, noticeWidth + 50, 42, fadedBorder);
                DrawText(topNotice.c_str(), nBoxX + 25, nBoxY + 12, 18, fadedBorder);
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

            // Polished Crosshair in screen center (Dynamic color feedback)
            if (!anyModalOpen) {
                int centerX = screenWidth / 2;
                int centerY = screenHeight / 2;
                bool isTargeting = (targetedRack != nullptr || targetedStorageProduct != ProductType::NONE);
                Color crosshairColor = isTargeting ? Color{ 46, 204, 113, 240 } : Color{ 255, 255, 255, 190 };
                
                // Crosshair reticle
                DrawCircle(centerX, centerY, isTargeting ? 3.5f : 2.5f, crosshairColor);
                DrawCircleLines(centerX, centerY, isTargeting ? 8.0f : 6.0f, crosshairColor);
                if (isTargeting) {
                    DrawLine(centerX - 12, centerY, centerX - 5, centerY, crosshairColor);
                    DrawLine(centerX + 5, centerY, centerX + 12, centerY, crosshairColor);
                    DrawLine(centerX, centerY - 12, centerX, centerY - 5, crosshairColor);
                    DrawLine(centerX, centerY + 5, centerX, centerY + 12, crosshairColor);
                }
            }

            // FPS Counter in top right
            DrawFPS(screenWidth - 90, 15);

            // ==========================================
            // SUPPLIER ORDER MODAL MENU (TAB)
            // ==========================================
            if (supplier.IsMenuOpen()) {
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 680;
                int modalH = 500;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 38, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 52, 152, 219, 255 });

                DrawText("MENU PENGADAAN BARANG (SUPPLIER)", modalX + 30, modalY + 18, 20, { 255, 215, 0, 255 });
                DrawText("Katalog Multi-Kategori: Minuman, Makanan, Camilan & Kebutuhan Rumah", modalX + 30, modalY + 42, 12, { 180, 190, 200, 255 });

                const auto& prods = priceMgr.GetManagedProducts();
                int curSel = supplier.GetSelectedProductIndex();
                int visibleCount = 3;
                int startIdx = std::max(0, std::min((int)prods.size() - visibleCount, curSel - 1));

                int listY = modalY + 68;
                for (int i = startIdx; i < startIdx + visibleCount && i < (int)prods.size(); ++i) {
                    ProductType pType = prods[i];
                    ProductInfo info = GetProductInfo(pType);
                    bool isSelected = (curSel == i);

                    Color itemBg = isSelected ? Color{ 40, 70, 110, 240 } : Color{ 35, 40, 48, 200 };
                    Color itemBorder = isSelected ? Color{ 0, 200, 255, 255 } : Color{ 60, 70, 80, 255 };

                    DrawRectangle(modalX + 30, listY, modalW - 60, 72, itemBg);
                    DrawRectangleLines(modalX + 30, listY, modalW - 60, 72, itemBorder);

                    DrawRectangle(modalX + 45, listY + 14, 44, 44, info.primaryColor);
                    DrawRectangleLines(modalX + 45, listY + 14, 44, 44, RAYWHITE);
                    DrawText(info.sku.c_str(), modalX + 47, listY + 30, 9, RAYWHITE);

                    std::string pTitle = info.name + " (" + GetCategoryName(info.category) + ")" + (isSelected ? "  <-- DIPILIH" : "");
                    DrawText(pTitle.c_str(), modalX + 100, listY + 12, 15, isSelected ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                    std::string priceLine = "Beli: Rp" + std::to_string(info.buyPrice) + 
                                            "  |  Jual: Rp" + std::to_string(priceMgr.GetSellPrice(pType)) +
                                            "  |  Storage: " + std::to_string(shop.GetStorage().GetStock(pType)) +
                                            "  |  SKU: " + info.sku;
                    DrawText(priceLine.c_str(), modalX + 100, listY + 34, 12, { 180, 200, 220, 255 });

                    std::string popText = "Popularitas: " + priceMgr.GetPopularityLevel(pType) + " (Terjual: " + std::to_string(priceMgr.GetProductStats(pType).totalSold) + ")";
                    DrawText(popText.c_str(), modalX + 100, listY + 52, 11, { 255, 215, 0, 255 });

                    listY += 78;
                }

                // Scroll Indicator
                std::string pageStr = "Item " + std::to_string(curSel + 1) + " / " + std::to_string(prods.size()) + " (Gunakan Panah Atas/Bawah untuk Scroll)";
                DrawText(pageStr.c_str(), modalX + 35, modalY + 308, 11, { 150, 170, 190, 255 });

                ProductInfo selectedInfo = GetProductInfo(supplier.GetSelectedProductType());
                int curQty = supplier.GetOrderQuantity();
                int curTotal = selectedInfo.buyPrice * curQty;

                int qtyBoxY = modalY + 328;
                DrawRectangle(modalX + 30, qtyBoxY, modalW - 60, 96, { 20, 25, 32, 230 });
                DrawRectangleLines(modalX + 30, qtyBoxY, modalW - 60, 96, { 100, 110, 120, 255 });

                std::string selOrdSummary = "Order: [" + selectedInfo.sku + "] " + selectedInfo.name + " (" + GetCategoryName(selectedInfo.category) + ")";
                DrawText(selOrdSummary.c_str(), modalX + 45, qtyBoxY + 10, 13, { 100, 220, 255, 255 });

                DrawText("Jumlah:", modalX + 45, qtyBoxY + 36, 14, RAYWHITE);
                DrawText(("[ < A / D > ]  " + std::to_string(curQty) + " Unit").c_str(), modalX + 115, qtyBoxY + 34, 16, { 255, 215, 0, 255 });

                DrawText("Total Biaya:", modalX + 330, qtyBoxY + 36, 14, RAYWHITE);
                DrawText(("Rp" + std::to_string(curTotal)).c_str(), modalX + 430, qtyBoxY + 34, 16, { 255, 100, 100, 255 });

                std::string treasuryHint = "Saldo Toko: Rp" + std::to_string(finance.GetCurrentBalance()) + 
                                           "  |  Gudang: " + std::to_string(shop.GetStorage().GetTotalStock()) + "/" + std::to_string(shop.GetStorage().GetMaxCapacity());
                DrawText(treasuryHint.c_str(), modalX + 45, qtyBoxY + 68, 13, { 50, 255, 120, 255 });

                DrawText("[W / S / Panah] Pilih Produk    [A / D] Ubah Jumlah (+-5)    [ENTER] Beli    [TAB / ESC] Tutup",
                         modalX + 35, modalY + 468, 12, { 255, 220, 120, 255 });
            }

            // ==========================================
            // PRICE & PRODUCT MANAGEMENT MODAL MENU (P)
            // ==========================================
            if (priceMgr.IsMenuOpen()) {
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 760;
                int modalH = 530;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 25, 30, 42, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 41, 128, 185, 255 });

                DrawText("MANAJEMEN HARGA JUAL & RESPON PELANGGAN", modalX + 30, modalY + 16, 19, { 100, 220, 255, 255 });
                DrawText("Atur harga berdasarkan persentase markup terhadap harga pasar wajar (Mempengaruhi probabilitas beli NPC)", modalX + 30, modalY + 38, 11, { 180, 195, 210, 255 });

                const auto& prods = priceMgr.GetManagedProducts();
                int curSel = priceMgr.GetSelectedProductIndex();
                int visibleCount = 3;
                int startIdx = std::max(0, std::min((int)prods.size() - visibleCount, curSel - 1));

                int listY = modalY + 60;
                for (int i = startIdx; i < startIdx + visibleCount && i < (int)prods.size(); ++i) {
                    ProductType pType = prods[i];
                    ProductInfo info = GetProductInfo(pType);
                    int sellPrice = priceMgr.GetSellPrice(pType);
                    int buyPrice = priceMgr.GetBuyPrice(pType);
                    int refPrice = priceMgr.GetReferencePrice(pType);
                    float markup = priceMgr.GetMarkupPercent(pType);
                    int margin = priceMgr.GetUnitMargin(pType);
                    const auto& stats = priceMgr.GetProductStats(pType);

                    bool isSelected = (curSel == i);

                    Color itemBg = isSelected ? Color{ 35, 65, 95, 240 } : Color{ 30, 36, 45, 200 };
                    Color itemBorder = isSelected ? Color{ 0, 220, 255, 255 } : Color{ 55, 65, 75, 255 };

                    DrawRectangle(modalX + 25, listY, modalW - 50, 96, itemBg);
                    DrawRectangleLines(modalX + 25, listY, modalW - 50, 96, itemBorder);

                    DrawRectangle(modalX + 38, listY + 14, 46, 46, info.primaryColor);
                    DrawRectangleLines(modalX + 38, listY + 14, 46, 46, RAYWHITE);
                    DrawText(info.sku.c_str(), modalX + 40, listY + 30, 9, RAYWHITE);

                    std::string pTitle = "[" + info.sku + "] " + info.name + " (" + GetCategoryName(info.category) + ")" + (isSelected ? "  [DIPILIH]" : "");
                    DrawText(pTitle.c_str(), modalX + 95, listY + 10, 14, isSelected ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                    // Pricing breakdown: Modal, Pasar/Ref, Jual, Markup %, Status
                    std::string priceLine = "Modal: Rp" + std::to_string(buyPrice) +
                                            "  |  Pasar: Rp" + std::to_string(refPrice) +
                                            "  |  Jual: Rp" + std::to_string(sellPrice);
                    DrawText(priceLine.c_str(), modalX + 95, listY + 28, 12, { 220, 230, 240, 255 });

                    // Markup badge & Profit
                    char markupStr[64];
                    std::snprintf(markupStr, sizeof(markupStr), "Markup: %+.1f%%", markup);
                    Color statusCol = priceMgr.GetPriceStatusColor(pType);
                    std::string statusTag = "[" + priceMgr.GetPriceStatusLabel(pType) + "]";

                    std::string marginText = "Profit/Unit: " + (margin >= 0 ? ("+Rp" + std::to_string(margin)) : ("-Rp" + std::to_string(-margin)));
                    DrawText(marginText.c_str(), modalX + 95, listY + 46, 12, (margin >= 0 ? Color{ 50, 255, 120, 255 } : Color{ 255, 80, 80, 255 }));
                    DrawText(markupStr, modalX + 275, listY + 46, 12, statusCol);
                    DrawText(statusTag.c_str(), modalX + 395, listY + 46, 12, statusCol);

                    // Sales Stats
                    std::string statLine = "Terjual: " + std::to_string(stats.totalSold) + " unit | Omset: Rp" + std::to_string(stats.totalRevenue) +
                                           " | Laba: Rp" + std::to_string(stats.totalProfit) + " | Pop: " + priceMgr.GetPopularityLevel(pType);
                    DrawText(statLine.c_str(), modalX + 95, listY + 66, 11, { 255, 215, 0, 255 });

                    if (isSelected) {
                        if (priceMgr.IsEditingPrice()) {
                            DrawRectangle(modalX + modalW - 235, listY + 10, 195, 34, { 30, 45, 65, 255 });
                            DrawRectangleLines(modalX + modalW - 235, listY + 10, 195, 34, { 0, 255, 200, 255 });
                            std::string inputShow = "Rp" + priceMgr.GetInputBuffer() + (((int)(GetTime() * 2.5f) % 2 == 0) ? "_" : " ");
                            DrawText(inputShow.c_str(), modalX + modalW - 225, listY + 18, 14, { 50, 255, 150, 255 });
                            DrawText("[ENTER] Simpan [ESC] Batal", modalX + modalW - 235, listY + 48, 10, { 200, 230, 255, 255 });
                        } else {
                            DrawText("[Q / E] Markup +-5%", modalX + modalW - 220, listY + 10, 12, { 50, 255, 180, 255 });
                            DrawText("[A / D] Harga +-Rp500", modalX + modalW - 220, listY + 28, 12, { 255, 215, 0, 255 });
                            DrawText("[ENTER] Ketik Manual", modalX + modalW - 220, listY + 46, 11, { 180, 210, 240, 255 });
                        }
                    }

                    listY += 102;
                }

                // Preset Buttons hint & summary
                int statBoxY = modalY + 372;
                DrawRectangle(modalX + 25, statBoxY, modalW - 50, 95, { 18, 22, 28, 240 });
                DrawRectangleLines(modalX + 25, statBoxY, modalW - 50, 95, { 60, 75, 90, 255 });

                ProductType bestType = priceMgr.GetBestSeller();
                ProductType slowType = priceMgr.GetSlowSeller();

                std::string bestName = (bestType != ProductType::NONE) ? 
                    (GetProductInfo(bestType).name + " (" + std::to_string(priceMgr.GetProductStats(bestType).totalSold) + " unit)") : "Belum ada";
                std::string slowName = (slowType != ProductType::NONE) ? 
                    (GetProductInfo(slowType).name + " (" + std::to_string(priceMgr.GetProductStats(slowType).totalSold) + " unit)") : "Belum ada";

                DrawText("PRESET CEPAT HARGA: [1] 0% (Wajar) | [2] +10% | [3] +25% | [4] +50% | [5] +100%", modalX + 40, statBoxY + 10, 12, { 100, 220, 255, 255 });
                DrawText(("BEST SELLER : " + bestName).c_str(), modalX + 40, statBoxY + 32, 12, { 50, 255, 120, 255 });
                DrawText(("SLOW SELLER : " + slowName).c_str(), modalX + 40, statBoxY + 52, 12, { 255, 160, 100, 255 });
                DrawText(("Item terpilih: " + std::to_string(curSel + 1) + " / " + std::to_string(prods.size())).c_str(), modalX + 40, statBoxY + 72, 11, { 180, 190, 200, 255 });

                if (priceMgr.IsEditingPrice()) {
                    DrawText("MODE KETIK HARGA: Ketik Angka [0-9] | [BACKSPACE] Hapus | [ENTER] Simpan | [ESC] Batal",
                             modalX + 40, modalY + 495, 12, { 50, 255, 150, 255 });
                } else {
                    DrawText("[W / S] Pilih    [Q / E] +-5% Markup    [A / D] +-Rp500    [1-5] Preset    [P / ESC] Tutup",
                             modalX + 40, modalY + 495, 12, { 255, 220, 120, 255 });
                }
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

                int cardW = 265;
                int cardH = 65;

                int c1X = modalX + 35;
                int c1Y = modalY + 80;
                DrawRectangle(c1X, c1Y, cardW, cardH, { 30, 38, 48, 240 });
                DrawRectangleLines(c1X, c1Y, cardW, cardH, { 70, 85, 100, 255 });
                DrawText("Saldo Toko Saat Ini:", c1X + 15, c1Y + 12, 13, { 180, 195, 210, 255 });
                std::string bStr = "Rp" + std::to_string(finance.GetCurrentBalance());
                DrawText(bStr.c_str(), c1X + 15, c1Y + 34, 18, { 50, 255, 120, 255 });

                int c2X = modalX + 320;
                int c2Y = modalY + 80;
                DrawRectangle(c2X, c2Y, cardW, cardH, { 30, 38, 48, 240 });
                DrawRectangleLines(c2X, c2Y, cardW, cardH, { 70, 85, 100, 255 });
                DrawText("Total Pendapatan (Revenue):", c2X + 15, c2Y + 12, 13, { 180, 195, 210, 255 });
                std::string rStr = "Rp" + std::to_string(finance.GetTotalRevenue());
                DrawText(rStr.c_str(), c2X + 15, c2Y + 34, 18, { 100, 220, 255, 255 });

                int c3X = modalX + 35;
                int c3Y = modalY + 155;
                DrawRectangle(c3X, c3Y, cardW, cardH, { 30, 38, 48, 240 });
                DrawRectangleLines(c3X, c3Y, cardW, cardH, { 70, 85, 100, 255 });
                DrawText("Total Pengeluaran (Expenses):", c3X + 15, c3Y + 12, 13, { 180, 195, 210, 255 });
                std::string eStr = "Rp" + std::to_string(finance.GetTotalExpenses());
                DrawText(eStr.c_str(), c3X + 15, c3Y + 34, 18, { 255, 100, 100, 255 });

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

                DrawText("Rumus: Profit = Total Pendapatan - Total Pengeluaran    |    [F / ESC] Tutup",
                         modalX + 45, modalY + 420, 13, { 255, 220, 120, 255 });
            }

            // ==========================================
            // STORE REPUTATION & RATING MODAL MENU (R)
            // ==========================================
            if (reputation.IsMenuOpen()) {
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 620;
                int modalH = 470;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 26, 28, 38, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 241, 196, 15, 255 });

                DrawText("REPUTASI DAN RATING TOKO", modalX + 30, modalY + 22, 20, { 255, 215, 0, 255 });
                DrawText("Evaluasi kepuasan customer, skor rating bintang, dan reputasi bisnis", modalX + 30, modalY + 48, 13, { 180, 195, 210, 255 });

                int cardW = 175;
                int cardH = 80;

                int c1X = modalX + 30;
                int c1Y = modalY + 75;
                DrawRectangle(c1X, c1Y, cardW, cardH, { 35, 38, 50, 240 });
                DrawRectangleLines(c1X, c1Y, cardW, cardH, { 241, 196, 15, 255 });
                DrawText("Reputasi Toko:", c1X + 12, c1Y + 12, 13, { 200, 210, 225, 255 });
                std::string repValStr = std::to_string(reputation.GetReputation()) + " / 100";
                Color repColor = (reputation.GetReputation() >= 75) ? Color{ 46, 204, 113, 255 } :
                                 (reputation.GetReputation() >= 50) ? Color{ 241, 196, 15, 255 } : Color{ 231, 76, 60, 255 };
                DrawText(repValStr.c_str(), c1X + 12, c1Y + 38, 20, repColor);

                int c2X = modalX + 222;
                int c2Y = modalY + 75;
                DrawRectangle(c2X, c2Y, cardW, cardH, { 35, 38, 50, 240 });
                DrawRectangleLines(c2X, c2Y, cardW, cardH, { 52, 152, 219, 255 });
                DrawText("Rata-rata Rating:", c2X + 12, c2Y + 12, 13, { 200, 210, 225, 255 });
                std::string avgStr = reputation.HasRatings() ? TextFormat("%.1f / 5.0", reputation.GetAverageRating()) : "Belum ada";
                DrawText(avgStr.c_str(), c2X + 12, c2Y + 38, 20, { 100, 220, 255, 255 });

                int c3X = modalX + 415;
                int c3Y = modalY + 75;
                DrawRectangle(c3X, c3Y, cardW, cardH, { 35, 38, 50, 240 });
                DrawRectangleLines(c3X, c3Y, cardW, cardH, { 46, 204, 113, 255 });
                DrawText("Jumlah Rating:", c3X + 12, c3Y + 12, 13, { 200, 210, 225, 255 });
                std::string countStr = std::to_string(reputation.GetTotalRatings()) + " Ulasan";
                DrawText(countStr.c_str(), c3X + 12, c3Y + 38, 20, { 50, 255, 120, 255 });

                DrawText("Ulasan & Kepuasan Customer Terbaru:", modalX + 30, modalY + 172, 14, { 255, 215, 0, 255 });
                int rBoxY = modalY + 195;
                DrawRectangle(modalX + 30, rBoxY, modalW - 60, 210, { 20, 22, 30, 240 });
                DrawRectangleLines(modalX + 30, rBoxY, modalW - 60, 210, { 60, 70, 80, 255 });

                const auto& ratings = reputation.GetRecentRatings();
                if (ratings.empty()) {
                    DrawText("Belum ada rating customer tercatat.", modalX + 50, rBoxY + 90, 13, { 140, 150, 160, 255 });
                    DrawText("Customer yang selesai berbelanja akan memberikan rating & feedback.", modalX + 50, rBoxY + 115, 12, { 110, 120, 130, 255 });
                } else {
                    int rRowY = rBoxY + 12;
                    for (size_t i = 0; i < ratings.size() && i < 4; ++i) {
                        const auto& r = ratings[i];
                        Color starColor = (r.stars >= 4) ? Color{ 255, 215, 0, 255 } : (r.stars == 3) ? Color{ 243, 156, 18, 255 } : Color{ 231, 76, 60, 255 };

                        std::string starsIcon = "";
                        for (int s = 0; s < r.stars; ++s) starsIcon += "* ";

                        DrawText(r.customerName.c_str(), modalX + 45, rRowY, 14, RAYWHITE);
                        DrawText(starsIcon.c_str(), modalX + 150, rRowY, 14, starColor);
                        DrawText(("(" + std::to_string(r.stars) + "/5)").c_str(), modalX + 220, rRowY, 12, { 180, 190, 200, 255 });
                        
                        std::string fbText = "\"" + r.feedback + "\" (" + std::to_string(r.satisfaction) + "% kepuasan)";
                        DrawText(fbText.c_str(), modalX + 280, rRowY, 12, starColor);

                        rRowY += 48;
                    }
                }

                DrawText("5 Bintang: +3 Reputasi | 4 Bintang: +1 | 3 Bintang: 0 | 2 Bintang: -2 | 1 Bintang: -3    [R / ESC] Tutup",
                         modalX + 35, modalY + 420, 12, { 255, 220, 120, 255 });
            }

            // ==========================================
            // SHOP UPGRADE MODAL MENU (U)
            // ==========================================
            if (shopUpgrade.IsMenuOpen()) {
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 680;
                int modalH = 490;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 22, 28, 38, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 52, 152, 219, 255 });

                DrawText("UPGRADE FASILITAS TOKO (SHOP UPGRADES)", modalX + 30, modalY + 20, 20, { 100, 220, 255, 255 });
                DrawText("Tingkatkan kapasitas toko, ukuran ruangan, rak & gudang menggunakan saldo toko", modalX + 30, modalY + 46, 13, { 180, 195, 210, 255 });

                UpgradeType uTypes[4] = {
                    UpgradeType::SHOP_SIZE,
                    UpgradeType::SHELF_CAPACITY,
                    UpgradeType::STORAGE_CAPACITY,
                    UpgradeType::CUSTOMER_CAPACITY
                };

                int listY = modalY + 74;
                for (int i = 0; i < 4; ++i) {
                    UpgradeType type = uTypes[i];
                    const auto& upInfo = shopUpgrade.GetUpgradeInfo(type);
                    bool isSelected = (shopUpgrade.GetSelectedUpgradeIndex() == i);
                    bool isMax = shopUpgrade.IsMaxLevel(type);
                    int nextCost = shopUpgrade.GetNextUpgradeCost(type);

                    Color itemBg = isSelected ? Color{ 35, 65, 100, 240 } : Color{ 28, 34, 45, 200 };
                    Color itemBorder = isSelected ? Color{ 0, 220, 255, 255 } : Color{ 55, 65, 78, 255 };

                    DrawRectangle(modalX + 30, listY, modalW - 60, 72, itemBg);
                    DrawRectangleLines(modalX + 30, listY, modalW - 60, 72, itemBorder);

                    // Upgrade Name & Level
                    std::string title = upInfo.name + (isSelected ? "  [Dipilih]" : "");
                    DrawText(title.c_str(), modalX + 45, listY + 10, 15, isSelected ? Color{ 255, 230, 100, 255 } : RAYWHITE);

                    std::string lvlStr = "Level: " + std::to_string(upInfo.currentLevel) + " / " + std::to_string(upInfo.maxLevel);
                    DrawText(lvlStr.c_str(), modalX + 320, listY + 10, 14, { 52, 152, 219, 255 });

                    // Current vs Next Benefit
                    std::string benefitStr = "Saat Ini: " + shopUpgrade.GetCurrentBenefitString(type) + 
                                             (isMax ? " (Maksimum)" : (" -> Upgrade: " + shopUpgrade.GetNextBenefitString(type)));
                    DrawText(benefitStr.c_str(), modalX + 45, listY + 32, 13, { 180, 200, 220, 255 });

                    // Cost / Max indicator
                    if (isMax) {
                        DrawRectangle(modalX + modalW - 165, listY + 14, 120, 44, { 40, 45, 55, 255 });
                        DrawRectangleLines(modalX + modalW - 165, listY + 14, 120, 44, { 80, 90, 100, 255 });
                        DrawText("MAX LEVEL", modalX + modalW - 150, listY + 28, 14, { 241, 196, 15, 255 });
                    } else {
                        Color costBg = (finance.GetCurrentBalance() >= nextCost) ? Color{ 30, 100, 50, 240 } : Color{ 110, 35, 35, 240 };
                        DrawRectangle(modalX + modalW - 165, listY + 14, 120, 44, costBg);
                        DrawRectangleLines(modalX + modalW - 165, listY + 14, 120, 44, RAYWHITE);
                        DrawText("Biaya:", modalX + modalW - 155, listY + 18, 11, RAYWHITE);
                        std::string costStr = "Rp" + std::to_string(nextCost);
                        DrawText(costStr.c_str(), modalX + modalW - 155, listY + 34, 13, { 255, 255, 120, 255 });
                    }

                    listY += 80;
                }

                // Balance & Footer Controls
                int footerY = modalY + 398;
                DrawRectangle(modalX + 30, footerY, modalW - 60, 40, { 18, 22, 28, 240 });
                DrawRectangleLines(modalX + 30, footerY, modalW - 60, 40, { 70, 80, 90, 255 });

                std::string balHint = "Saldo Toko: Rp" + std::to_string(finance.GetCurrentBalance());
                DrawText(balHint.c_str(), modalX + 45, footerY + 11, 15, { 50, 255, 120, 255 });

                DrawText("[W / S / Panah] Pilih Upgrade    [ENTER] Beli Upgrade    [U / ESC] Tutup",
                         modalX + 45, modalY + 452, 13, { 255, 220, 120, 255 });
            }

            // ==========================================
            // FURNITURE & EQUIPMENT MODAL MENU (B)
            // ==========================================
            if (furniture.IsMenuOpen()) {
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

                int modalW = 700;
                int modalH = 500;
                int modalX = (screenWidth - modalW) / 2;
                int modalY = (screenHeight - modalH) / 2;

                DrawRectangle(modalX, modalY, modalW, modalH, { 24, 28, 38, 250 });
                DrawRectangleLines(modalX, modalY, modalW, modalH, { 230, 126, 34, 255 });

                DrawText("KATALOG FURNITURE & EQUIPMENT TOKO", modalX + 30, modalY + 18, 20, { 255, 180, 50, 255 });

                // Tabs: [Furniture 3D] vs [Equipment Bonus]
                int tabY = modalY + 46;
                bool isFurnTab = (furniture.GetSelectedTabIndex() == 0);

                Color furnTabBg = isFurnTab ? Color{ 230, 126, 34, 255 } : Color{ 40, 45, 55, 255 };
                Color equipTabBg = !isFurnTab ? Color{ 52, 152, 219, 255 } : Color{ 40, 45, 55, 255 };

                DrawRectangle(modalX + 30, tabY, 180, 32, furnTabBg);
                DrawRectangleLines(modalX + 30, tabY, 180, 32, RAYWHITE);
                DrawText("1. Furniture 3D (TAB)", modalX + 45, tabY + 8, 13, RAYWHITE);

                DrawRectangle(modalX + 220, tabY, 195, 32, equipTabBg);
                DrawRectangleLines(modalX + 220, tabY, 195, 32, RAYWHITE);
                DrawText("2. Peralatan / Equipment", modalX + 232, tabY + 8, 13, RAYWHITE);

                // Items list
                int listY = modalY + 90;

                if (isFurnTab) {
                    const auto& fList = furniture.GetFurnitureList();
                    for (size_t i = 0; i < fList.size(); ++i) {
                        const auto& item = fList[i];
                        bool isSel = (furniture.GetSelectedIndex() == (int)i);

                        Color itemBg = isSel ? Color{ 60, 45, 35, 240 } : Color{ 30, 35, 45, 200 };
                        Color itemBorder = isSel ? Color{ 255, 180, 50, 255 } : Color{ 55, 65, 75, 255 };

                        DrawRectangle(modalX + 30, listY, modalW - 60, 55, itemBg);
                        DrawRectangleLines(modalX + 30, listY, modalW - 60, 55, itemBorder);

                        // Icon Color Box
                        DrawRectangle(modalX + 42, listY + 12, 30, 30, item.primaryColor);
                        DrawRectangleLines(modalX + 42, listY + 12, 30, 30, RAYWHITE);

                        // Name and description
                        std::string title = item.name + (isSel ? "  [Dipilih]" : "");
                        DrawText(title.c_str(), modalX + 85, listY + 8, 14, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);
                        DrawText(item.description.c_str(), modalX + 85, listY + 28, 12, { 180, 195, 210, 255 });

                        // Status / Price
                        if (item.isOwned) {
                            DrawRectangle(modalX + modalW - 145, listY + 10, 100, 34, { 30, 90, 45, 255 });
                            DrawRectangleLines(modalX + modalW - 145, listY + 10, 100, 34, { 100, 255, 120, 255 });
                            DrawText("OWNED", modalX + modalW - 122, listY + 18, 13, { 100, 255, 120, 255 });
                        } else {
                            DrawText(("Rp" + std::to_string(item.price)).c_str(), modalX + modalW - 145, listY + 18, 14, { 255, 215, 0, 255 });
                        }

                        listY += 60;
                    }
                } else {
                    const auto& eList = furniture.GetEquipmentList();
                    for (size_t i = 0; i < eList.size(); ++i) {
                        const auto& item = eList[i];
                        bool isSel = (furniture.GetSelectedIndex() == (int)i);

                        Color itemBg = isSel ? Color{ 35, 60, 95, 240 } : Color{ 30, 35, 45, 200 };
                        Color itemBorder = isSel ? Color{ 52, 152, 219, 255 } : Color{ 55, 65, 75, 255 };

                        DrawRectangle(modalX + 30, listY, modalW - 60, 68, itemBg);
                        DrawRectangleLines(modalX + 30, listY, modalW - 60, 68, itemBorder);

                        // Name and bonus description
                        std::string title = item.name + (isSel ? "  [Dipilih]" : "");
                        DrawText(title.c_str(), modalX + 45, listY + 10, 14, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);
                        DrawText(item.bonusDescription.c_str(), modalX + 45, listY + 34, 12, { 100, 220, 255, 255 });

                        // Status / Price
                        if (item.isOwned) {
                            DrawRectangle(modalX + modalW - 145, listY + 16, 100, 36, { 30, 90, 45, 255 });
                            DrawRectangleLines(modalX + modalW - 145, listY + 16, 100, 36, { 100, 255, 120, 255 });
                            DrawText("OWNED", modalX + modalW - 122, listY + 25, 13, { 100, 255, 120, 255 });
                        } else {
                            DrawText(("Rp" + std::to_string(item.price)).c_str(), modalX + modalW - 145, listY + 25, 14, { 255, 215, 0, 255 });
                        }

                        listY += 75;
                    }
                }

                // Balance & Footer Controls
                int footerY = modalY + 410;
                DrawRectangle(modalX + 30, footerY, modalW - 60, 38, { 18, 22, 28, 240 });
                DrawRectangleLines(modalX + 30, footerY, modalW - 60, 38, { 70, 80, 90, 255 });

                std::string balHint = "Saldo Toko: Rp" + std::to_string(finance.GetCurrentBalance());
                DrawText(balHint.c_str(), modalX + 45, footerY + 10, 15, { 50, 255, 120, 255 });

                DrawText("[Q / E / TAB] Ganti Tab    [W / S / Panah] Pilih    [ENTER] Beli Item    [B / ESC] Tutup",
                         modalX + 45, modalY + 460, 13, { 255, 220, 120, 255 });
            }

            // ==========================================
            // DAILY SUMMARY MODAL OVERLAY (Tahap 10)
            // ==========================================
            if (gameTime.IsDaySummaryOpen()) {
                dailyStats.RenderSummaryModal(screenWidth, screenHeight, gameTime.GetCurrentDay(), finance.GetCurrentBalance());
            }

            // ==========================================
            // GAME MENU & SAVE / LOAD MODAL (Tahap 11)
            // ==========================================
            if (saveSystem.IsMenuOpen()) {
                saveSystem.RenderMenu(screenWidth, screenHeight, defaultSaveFile, gameTime.GetCurrentDay(), gameTime.GetFormattedTime(), finance.GetCurrentBalance());
            }

            // ==========================================
            // CUSTOMER AI DEBUG OVERLAY (Tahap 14 - F3)
            // ==========================================
            if (showAiDebug && !anyModalOpen) {
                int dbgW = 440;
                int dbgH = 35 + std::min(6, (int)customers.size()) * 42;
                int dbgX = screenWidth - dbgW - 15;
                int dbgY = 80;

                DrawRectangle(dbgX, dbgY, dbgW, dbgH, { 15, 20, 30, 235 });
                DrawRectangleLines(dbgX, dbgY, dbgW, dbgH, { 0, 200, 255, 255 });
                DrawText("CUSTOMER AI DEBUG MONITOR [F3: Tutup]", dbgX + 15, dbgY + 10, 14, { 0, 255, 255, 255 });

                int rowY = dbgY + 32;
                for (size_t i = 0; i < customers.size() && i < 6; ++i) {
                    const auto& c = customers[i];
                    std::string line1 = "#" + std::to_string(c.GetId()) + " " + c.GetName() + 
                                        " [" + c.GetCustomerTypeString() + "] Sat:" + std::to_string(c.GetSatisfaction()) + "%";
                    std::string line2 = "  State: " + c.GetStateString() + " | Item: " + c.GetCarriedSummaryString();
                    
                    DrawText(line1.c_str(), dbgX + 15, rowY, 12, { 255, 220, 100, 255 });
                    DrawText(line2.c_str(), dbgX + 15, rowY + 16, 11, { 180, 220, 255, 255 });
                    rowY += 40;
                }
            }

        EndDrawing();
    }

    // 6. Cleanup
    audioMgr.Close();
    CloseWindow();

    return 0;
}
