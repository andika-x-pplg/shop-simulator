#include "Customer.hpp"
#include "Shop.hpp"
#include "PriceManager.hpp"
#include "MarketSystem.hpp"
#include <cmath>
#include <algorithm>
#include <map>

Customer::Customer()
    : id(0),
      name("Customer"),
      type(CustomerType::NORMAL),
      position{ 0.0f, 0.0f, 16.0f },
      velocity{ 0.0f, 0.0f, 0.0f },
      rotationY(PI),
      moveSpeed(2.8f),
      state(CustomerState::ENTERING),
      currentShoppingItemIndex(0),
      currentWaypointIndex(0),
      targetRackPos{ 0.0f, 0.0f, 0.0f },
      targetRackId(-1),
      currentTargetProduct(ProductType::NONE),
      waitTimer(0.0f),
      maxWaitDuration(2.0f),
      patienceMultiplier(1.0f),
      maxQueuePatience(25.0f),
      payTimer(0.0f),
      hasPaid(false),
      speechBubbleText(""),
      speechBubbleTimer(0.0f),
      speechCooldown(0.0f),
      speechBubbleColor{ 245, 245, 250, 245 },
      speechTextColor{ 20, 25, 35, 255 },
      satisfaction(100),
      totalQueueWaitTime(0.0f),
      penaltyWaitShortApplied(false),
      penaltyWaitMidApplied(false),
      penaltyWaitLongApplied(false),
      penaltyNoStockApplied(false),
      penaltyPriceTooHighApplied(false),
      bonusProductAcquired(false),
      bonusPaymentSuccess(false),
      hasGivenRating(false),
      specificFeedback(""),
      bodyColor{ 255, 218, 185, 255 },
      shirtColor{ 70, 130, 180, 255 },
      pantsColor{ 45, 52, 54, 255 },
      heightScale(1.0f),
      bobbingTimer(0.0f)
{
    GenerateShoppingList();
    BuildEntryWaypoints();
}

Customer::Customer(int id, const std::string& name, CustomerType type, Vector3 spawnPos, Color bodyColor, Color shirtColor)
    : id(id),
      name(name),
      type(type),
      position(spawnPos),
      velocity{ 0.0f, 0.0f, 0.0f },
      rotationY(PI), // Face north towards shop door
      moveSpeed(2.8f),
      state(CustomerState::ENTERING),
      currentShoppingItemIndex(0),
      currentWaypointIndex(0),
      targetRackPos{ 0.0f, 0.0f, 0.0f },
      targetRackId(-1),
      currentTargetProduct(ProductType::NONE),
      waitTimer(0.0f),
      maxWaitDuration(2.0f + (id % 3) * 0.5f),
      patienceMultiplier(1.0f),
      maxQueuePatience(25.0f),
      payTimer(0.0f),
      hasPaid(false),
      speechBubbleText(""),
      speechBubbleTimer(0.0f),
      speechCooldown(0.0f),
      speechBubbleColor{ 245, 245, 250, 245 },
      speechTextColor{ 20, 25, 35, 255 },
      satisfaction(100),
      totalQueueWaitTime(0.0f),
      penaltyWaitShortApplied(false),
      penaltyWaitMidApplied(false),
      penaltyWaitLongApplied(false),
      penaltyNoStockApplied(false),
      penaltyPriceTooHighApplied(false),
      bonusProductAcquired(false),
      bonusPaymentSuccess(false),
      hasGivenRating(false),
      specificFeedback(""),
      bodyColor(bodyColor),
      shirtColor(shirtColor),
      pantsColor{ 50, 55, 60, 255 },
      heightScale(0.94f + ((id % 7) * 0.02f)),
      bobbingTimer(0.0f)
{
    // Apply personality type configuration (Tahap 14)
    switch (type) {
        case CustomerType::IMPATIENT:
            moveSpeed = 3.2f;
            patienceMultiplier = 0.65f;
            maxQueuePatience = 14.0f; // Leaves if waiting >14s
            maxWaitDuration = 1.2f;
            break;
        case CustomerType::PATIENT:
            moveSpeed = 2.4f;
            patienceMultiplier = 1.45f;
            maxQueuePatience = 40.0f; // Very patient
            maxWaitDuration = 2.8f;
            break;
        case CustomerType::BIG_SHOPPER:
            moveSpeed = 2.6f;
            patienceMultiplier = 1.15f;
            maxQueuePatience = 30.0f;
            maxWaitDuration = 2.2f;
            break;
        case CustomerType::PRICE_SENSITIVE:
            moveSpeed = 2.7f;
            patienceMultiplier = 0.9f;
            maxQueuePatience = 22.0f;
            maxWaitDuration = 2.5f; // Spends extra time inspecting price
            break;
        case CustomerType::NORMAL:
        default:
            moveSpeed = 2.8f;
            patienceMultiplier = 1.0f;
            maxQueuePatience = 25.0f;
            maxWaitDuration = 1.8f;
            break;
    }

    GenerateShoppingList();
    BuildEntryWaypoints();
}

void Customer::Say(const std::string& text, float duration, Color bubbleColor, Color textColor) {
    speechBubbleText = text;
    speechBubbleTimer = duration;
    speechBubbleColor = bubbleColor;
    speechTextColor = textColor;
    speechCooldown = 2.0f;
}

void Customer::GenerateShoppingList(ProductType popularProductPreference) {
    shoppingList.clear();
    carriedItems.clear();
    currentShoppingItemIndex = 0;

    const auto& allProducts = GetAllProductTypes();

    switch (type) {
        case CustomerType::BIG_SHOPPER: {
            // Big Shoppers buy 2 to 4 different product types (total 2 to 5 items)
            int itemTypes = 2 + (id % 3); // 2 to 4 types
            for (int i = 0; i < itemTypes && i < (int)allProducts.size(); ++i) {
                ProductType p = allProducts[(id + i) % allProducts.size()];
                int qty = 1 + (id % 2); // 1 or 2 items each
                shoppingList.push_back({ p, qty, 0 });
            }
            break;
        }
        case CustomerType::IMPATIENT: {
            // Impatient buys quickly 1 item
            ProductType p = (popularProductPreference != ProductType::NONE && (id % 2 == 0)) ? popularProductPreference : allProducts[id % allProducts.size()];
            shoppingList.push_back({ p, 1, 0 });
            break;
        }
        case CustomerType::PRICE_SENSITIVE: {
            // Price sensitive prefers essential drinks, bread, or soap
            std::vector<ProductType> affordable = { ProductType::BEVERAGE, ProductType::INSTANT_NOODLE, ProductType::SOAP_BAR, ProductType::BREAD };
            ProductType p = affordable[id % affordable.size()];
            shoppingList.push_back({ p, 1, 0 });
            if (id % 3 == 0) {
                shoppingList.push_back({ ProductType::SNACK_BISCUIT, 1, 0 });
            }
            break;
        }
        case CustomerType::PATIENT: {
            // Patient customer buys 1 to 3 items
            ProductType p1 = (popularProductPreference != ProductType::NONE && (id % 3 == 0)) ? popularProductPreference : allProducts[id % allProducts.size()];
            shoppingList.push_back({ p1, 1, 0 });
            if (id % 2 == 0) {
                ProductType p2 = allProducts[(id + 2) % allProducts.size()];
                shoppingList.push_back({ p2, 1, 0 });
            }
            break;
        }
        case CustomerType::NORMAL:
        default: {
            ProductType p1 = (popularProductPreference != ProductType::NONE && (id % 2 == 0)) ? popularProductPreference : allProducts[id % allProducts.size()];
            shoppingList.push_back({ p1, 1, 0 });
            if (id % 3 == 0) {
                ProductType p2 = allProducts[(id + 3) % allProducts.size()];
                shoppingList.push_back({ p2, 1, 0 });
            }
            break;
        }
    }

    // If popular product event is active and not already on list, high chance to add it
    if (popularProductPreference != ProductType::NONE && (id % 2 == 0)) {
        bool alreadyHas = false;
        for (const auto& item : shoppingList) {
            if (item.product == popularProductPreference) {
                alreadyHas = true;
                break;
            }
        }
        if (!alreadyHas) {
            shoppingList.push_back({ popularProductPreference, 1, 0 });
        }
    }
}

std::string Customer::GetCustomerTypeString() const {
    switch (type) {
        case CustomerType::NORMAL: return "Normal";
        case CustomerType::IMPATIENT: return "Tidak Sabaran (Impatient)";
        case CustomerType::PATIENT: return "Penyabar (Patient)";
        case CustomerType::BIG_SHOPPER: return "Borongan (Big Shopper)";
        case CustomerType::PRICE_SENSITIVE: return "Sensitif Harga (Price Sensitive)";
        default: return "Normal";
    }
}

std::string Customer::GetStateString() const {
    switch (state) {
        case CustomerState::ENTERING: return "Masuk ke Toko";
        case CustomerState::BROWSING: return "Memilih Daftar Belanja";
        case CustomerState::SEARCHING_PRODUCT: return "Mencari " + GetProductInfo(currentTargetProduct).name;
        case CustomerState::WALKING_TO_SHELF: return "Menuju Rak " + GetProductInfo(currentTargetProduct).name;
        case CustomerState::AT_SHELF: return "Memeriksa " + GetProductInfo(currentTargetProduct).name;
        case CustomerState::TAKING_PRODUCT: return "Mengambil " + GetProductInfo(currentTargetProduct).name;
        case CustomerState::GOING_TO_CASHIER: return "Menuju Kasir";
        case CustomerState::WAITING_FOR_CASHIER: return "Mengantre di Kasir";
        case CustomerState::PAYING: return "Membayar di Kasir";
        case CustomerState::LEAVING: return "Menuju Pintu Keluar";
        case CustomerState::EXITING: return "Keluar dari Toko";
        case CustomerState::DESPAWNED: return "Selesai (Despawn)";
        default: return "Idle";
    }
}

std::string Customer::GetCarriedSummaryString() const {
    if (carriedItems.empty()) return "Keranjang Kosong";
    std::map<ProductType, int> counts;
    for (auto p : carriedItems) {
        counts[p]++;
    }
    std::string summary = "";
    bool first = true;
    for (const auto& pair : counts) {
        if (!first) summary += ", ";
        summary += GetProductInfo(pair.first).name + " x" + std::to_string(pair.second);
        first = false;
    }
    return summary;
}

std::string Customer::GetFeedbackMessage() const {
    if (!specificFeedback.empty()) return specificFeedback;

    if (!carriedItems.empty() && hasPaid) {
        if (satisfaction >= 90) return "Pelayanan sangat cepat dan barang lengkap!";
        if (satisfaction >= 75) return "Belanja cukup nyaman dan puas.";
        if (satisfaction >= 50) return "Barang dapat, tapi antrean agak lama.";
        return "Pelayanan kasir lambat.";
    } else {
        if (penaltyPriceTooHighApplied) return "Harga barang terlalu mahal!";
        if (penaltyNoStockApplied) return "Produk yang saya cari habis!";
        if (totalQueueWaitTime >= maxQueuePatience) return "Antrean kasir terlalu lama, saya pergi!";
        return "Tidak menemukan barang yang dicari.";
    }
}

void Customer::BuildEntryWaypoints() {
    waypoints.clear();
    currentWaypointIndex = 0;

    // Waypoint 1: Outside right in front of door entrance (Z = 12.5)
    waypoints.push_back({ 0.0f, 0.0f, 12.5f });
    // Waypoint 2: Step through door frame (Z = 11.5)
    waypoints.push_back({ 0.0f, 0.0f, 11.5f });
    // Waypoint 3: Inside main walkway lobby (Z = 8.5)
    waypoints.push_back({ 0.0f, 0.0f, 8.5f });
}

bool Customer::SearchTargetRack(Shop& shop) {
    if (currentTargetProduct == ProductType::NONE) return false;

    int rackIdx = shop.FindRackWithProduct(currentTargetProduct);
    if (rackIdx >= 0) {
        targetRackId = rackIdx;
        targetRackPos = shop.GetRackFrontPosition((size_t)rackIdx);
        return true;
    }
    return false;
}

void Customer::BuildPathToCashier(Vector3 queueSlot) {
    waypoints.clear();
    currentWaypointIndex = 0;

    // If currently deep in left aisle (X < -2.0) or right aisle (X > 2.0)
    if (fabsf(position.x) > 2.0f && position.z < 2.0f) {
        float aisleX = (position.x > 0.0f) ? 3.2f : -3.2f;
        waypoints.push_back({ aisleX, 0.0f, 4.0f });
    }

    // Cross over through the clear front lobby walkway (Z = 6.0m)
    waypoints.push_back({ 0.0f, 0.0f, 6.0f });

    // Move directly to the assigned queue position in front of cashier
    waypoints.push_back(queueSlot);
}

void Customer::BuildExitWaypoints(Vector3 startPos) {
    waypoints.clear();
    currentWaypointIndex = 0;

    // 1. If currently deep in aisle, move forward along aisle to front walkway first
    if (fabsf(position.x) > 2.0f && position.z < 2.0f) {
        float aisleX = (position.x > 0.0f) ? 3.2f : -3.2f;
        waypoints.push_back({ aisleX, 0.0f, 5.0f });
    }

    // 2. Move to central doorway corridor (X = 0, Z = 8.5)
    waypoints.push_back({ 0.0f, 0.0f, 8.5f });
    // 3. Step out through the door (Z = 11.5)
    waypoints.push_back({ 0.0f, 0.0f, 11.5f });
    // 4. Outside door (Z = 13.0)
    waypoints.push_back({ 0.0f, 0.0f, 13.0f });
    // 5. Outside despawn area
    float exitSide = (id % 2 == 0) ? 8.0f : -8.0f;
    waypoints.push_back({ exitSide, 0.0f, 18.0f });
}

void Customer::MoveTowards(Vector3 target, float deltaTime) {
    Vector3 toTarget = Vector3Subtract(target, position);
    toTarget.y = 0.0f; // Flat floor movement

    float dist = Vector3Length(toTarget);
    if (dist > 0.05f) {
        Vector3 dir = Vector3Normalize(toTarget);
        position = Vector3Add(position, Vector3Scale(dir, moveSpeed * deltaTime));
        rotationY = atan2f(-dir.x, -dir.z); // Face movement direction
        bobbingTimer += deltaTime * (moveSpeed * 2.8f);
    }
}

void Customer::ApplySeparation(const std::vector<Customer>& otherCustomers, float deltaTime) {
    if (state == CustomerState::DESPAWNED || state == CustomerState::PAYING) return;

    Vector3 separation = { 0.0f, 0.0f, 0.0f };
    float minSeparationDist = 0.85f;

    for (const auto& other : otherCustomers) {
        if (other.GetId() == id || other.GetState() == CustomerState::DESPAWNED) continue;

        Vector3 diff = Vector3Subtract(position, other.GetPosition());
        diff.y = 0.0f;
        float dist = Vector3Length(diff);

        if (dist > 0.001f && dist < minSeparationDist) {
            float strength = (minSeparationDist - dist) / minSeparationDist;
            Vector3 pushDir = Vector3Normalize(diff);
            separation = Vector3Add(separation, Vector3Scale(pushDir, strength));
        }
    }

    if (Vector3Length(separation) > 0.01f) {
        Vector3 push = Vector3Scale(Vector3Normalize(separation), 0.8f * deltaTime);
        position = Vector3Add(position, push);
    }
}

void Customer::Update(float deltaTime, Shop& shop, int queueIndex, bool& outDidPay, int& outPaidAmount, std::string& outPaidProductSummary) {
    outDidPay = false;
    outPaidAmount = 0;
    outPaidProductSummary = "";

    if (state == CustomerState::DESPAWNED) return;

    // -------------------------------------------------------------
    // State 1: ENTERING
    // -------------------------------------------------------------
    if (state == CustomerState::ENTERING) {
        if (currentWaypointIndex < waypoints.size()) {
            Vector3 targetWp = waypoints[currentWaypointIndex];
            MoveTowards(targetWp, deltaTime);

            float dist = Vector3Distance(position, targetWp);
            if (dist < 0.35f) {
                currentWaypointIndex++;
            }
        } else {
            // Reached lobby inside shop -> Say greeting and start browsing shopping list
            state = CustomerState::BROWSING;
            const std::vector<std::string> greetings = {
                "Semoga ada yang aku cari.",
                "Mau belanja sebentar.",
                "Aku cuma cari beberapa barang.",
                "Semoga stoknya masih ada.",
                "Hari ini mau belanja kebutuhan."
            };
            Say(greetings[id % greetings.size()], 3.0f, Color{ 240, 245, 255, 245 }, Color{ 25, 45, 75, 255 });
        }
    }
    // -------------------------------------------------------------
    // State 2: BROWSING (Inspect Shopping List)
    // -------------------------------------------------------------
    else if (state == CustomerState::BROWSING) {
        if (speechBubbleTimer > 0.0f) {
            speechBubbleTimer -= deltaTime;
        }
        if (speechCooldown > 0.0f) {
            speechCooldown -= deltaTime;
        }

        // Find next item in shopping list that needs to be acquired
        bool hasNextItem = false;
        while (currentShoppingItemIndex < shoppingList.size()) {
            auto& item = shoppingList[currentShoppingItemIndex];
            if (item.acquiredQuantity < item.desiredQuantity) {
                currentTargetProduct = item.product;
                hasNextItem = true;
                break;
            }
            currentShoppingItemIndex++;
        }

        if (hasNextItem) {
            state = CustomerState::SEARCHING_PRODUCT;
            // Occasional browsing chatter
            if (speechCooldown <= 0.0f && (id + (int)currentShoppingItemIndex) % 3 == 0) {
                const std::vector<std::string> browseChatter = {
                    "Hmm, aku butuh ini.",
                    "Oh, ini yang aku cari.",
                    "Yang ini boleh juga.",
                    "Berapa harganya ya?",
                    "Semoga harganya pas di kantong."
                };
                Say(browseChatter[(id + currentShoppingItemIndex) % browseChatter.size()], 2.5f);
            }
        } else {
            // All shopping list items processed
            if (!carriedItems.empty()) {
                // Head to cashier to pay for acquired items
                state = CustomerState::GOING_TO_CASHIER;
                Vector3 queueSlot = shop.GetCashier().GetQueuePosition(queueIndex);
                BuildPathToCashier(queueSlot);
                if (speechCooldown <= 0.0f) {
                    Say("Semua sudah dapat, sekarang ke kasir.", 2.5f);
                }
            } else {
                // Left without any items -> Penalti & leave
                if (!penaltyNoStockApplied) {
                    satisfaction = std::clamp(satisfaction - 20, 0, 100);
                    penaltyNoStockApplied = true;
                }
                if (specificFeedback.empty()) {
                    specificFeedback = "Semua produk yang saya cari tidak tersedia!";
                }
                state = CustomerState::LEAVING;
                BuildExitWaypoints(position);
                Say("Sayang sekali tidak dapat barang.", 2.8f, Color{ 255, 230, 230, 245 }, Color{ 180, 40, 40, 255 });
            }
        }
    }
    // -------------------------------------------------------------
    // State 3: SEARCHING_PRODUCT
    // -------------------------------------------------------------
    else if (state == CustomerState::SEARCHING_PRODUCT) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        bool found = SearchTargetRack(shop);
        if (found) {
            state = CustomerState::WALKING_TO_SHELF;
            waypoints.clear();
            currentWaypointIndex = 0;
            // Midpoint waypoint along corridor
            waypoints.push_back({ 0.0f, 0.0f, targetRackPos.z });
            waypoints.push_back(targetRackPos);
        } else {
            // Target product out of stock
            if (!penaltyNoStockApplied) {
                satisfaction = std::clamp(satisfaction - 10, 0, 100);
                penaltyNoStockApplied = true;
            }
            Say("Yah, " + GetProductInfo(currentTargetProduct).name + " lagi kosong.", 2.5f, Color{ 255, 240, 230, 245 }, Color{ 180, 80, 40, 255 });
            // Skip this item and move to next item on list
            currentShoppingItemIndex++;
            state = CustomerState::BROWSING;
        }
    }
    // -------------------------------------------------------------
    // State 4: WALKING_TO_SHELF
    // -------------------------------------------------------------
    else if (state == CustomerState::WALKING_TO_SHELF) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        if (currentWaypointIndex < waypoints.size()) {
            Vector3 targetWp = waypoints[currentWaypointIndex];
            MoveTowards(targetWp, deltaTime);

            float dist = Vector3Distance(position, targetWp);
            if (dist < 0.35f) {
                currentWaypointIndex++;
            }
        } else {
            // Reached front of target shelf!
            state = CustomerState::AT_SHELF;
            waitTimer = 0.0f;
        }
    }
    // -------------------------------------------------------------
    // State 5: AT_SHELF (Dynamic Probabilistic Price Decision System)
    // -------------------------------------------------------------
    else if (state == CustomerState::AT_SHELF) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        waitTimer += deltaTime;

        if (waitTimer >= maxWaitDuration) {
            // Stage 20: Evaluate price dynamically against MarketSystem & Demand
            int curPrice = PriceManager::Instance().GetSellPrice(currentTargetProduct);
            int mktPrice = MarketSystem::Instance().GetCurrentMarketPrice(currentTargetProduct);
            int custTypeInt = static_cast<int>(type);

            // Compute buy chance using MarketSystem formula (accounts for persona & demand)
            float buyChance = MarketSystem::Instance().EvaluateCustomerBuyChance(currentTargetProduct, curPrice, custTypeInt, 0.95f);

            // Pseudo-random roll using customer id, item index, price, and time
            int rollSeed = (id * 37 + (int)currentShoppingItemIndex * 19 + curPrice + (int)(GetTime() * 10.0f)) % 100;
            float roll = (float)rollSeed / 100.0f;

            if (roll <= buyChance) {
                // DECISION: BUY
                state = CustomerState::TAKING_PRODUCT;

                // Dynamic contextual buying dialogue (Market, Trend, Demand aware)
                std::vector<std::string> buyDialogues;
                int diffPct = (mktPrice > 0) ? ((curPrice - mktPrice) * 100 / mktPrice) : 0;
                int demandVal = MarketSystem::Instance().GetDemand(currentTargetProduct);

                if (diffPct <= -10) {
                    buyDialogues = {
                        "Harganya murah banget di sini!",
                        "Diskonnya bagus, langsung beli.",
                        "Murah dibanding toko lain.",
                        "Untung dapat harga murah!"
                    };
                } else if (demandVal >= 75) {
                    buyDialogues = {
                        "Produk ini lagi banyak dicari!",
                        "Untung barang ini masih ada stok.",
                        "Ini barang yang lagi tren.",
                        "Oke, aku ambil produk ini."
                    };
                } else {
                    buyDialogues = {
                        "Oke, aku ambil.",
                        "Harganya masih wajar dan masuk akal.",
                        "Kayaknya cocok, beli satu.",
                        "Ini masuk keranjang.",
                        "Bagus, harganya pas."
                    };
                }
                Say(buyDialogues[(id + rollSeed) % buyDialogues.size()], 2.5f, Color{ 235, 255, 240, 245 }, Color{ 20, 100, 40, 255 });

                // Satisfaction adjustments
                if (diffPct < -10) {
                    satisfaction = std::min(100, satisfaction + 5);
                } else if (diffPct > 30) {
                    satisfaction = std::max(20, satisfaction - 5);
                }
            } else {
                // DECISION: REFUSE (Too expensive / above market / low value)
                std::vector<std::string> refuseDialogues;
                int diffPct = (mktPrice > 0) ? ((curPrice - mktPrice) * 100 / mktPrice) : 0;

                if (diffPct >= 40) {
                    refuseDialogues = {
                        "Harganya kemahalan jauh dari pasaran!",
                        "Terlalu mahal, saya cari toko lain.",
                        "Harganya di luar budget sama sekali.",
                        "Gila, mahal banget!"
                    };
                } else {
                    refuseDialogues = {
                        "Harganya agak mahal.",
                        "Kayaknya kemahalan sedikit.",
                        "Aku cari yang lebih murah.",
                        "Kalau segini aku nggak jadi beli.",
                        "Hmm... kemahalan deh."
                    };
                }
                Say(refuseDialogues[(id + rollSeed) % refuseDialogues.size()], 3.0f, Color{ 255, 235, 235, 245 }, Color{ 180, 40, 40, 255 });

                if (!penaltyPriceTooHighApplied) {
                    satisfaction = std::clamp(satisfaction - 15, 0, 100);
                    penaltyPriceTooHighApplied = true;
                    specificFeedback = "Harga " + GetProductInfo(currentTargetProduct).name + " terlalu mahal!";
                }

                // Skip to next item
                currentShoppingItemIndex++;
                state = CustomerState::BROWSING;
            }
        }
    }
    // -------------------------------------------------------------
    // State 6: TAKING_PRODUCT
    // -------------------------------------------------------------
    else if (state == CustomerState::TAKING_PRODUCT) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        auto& racks = shop.GetRacks();
        bool gotProduct = false;

        if (targetRackId >= 0 && (size_t)targetRackId < racks.size() && racks[(size_t)targetRackId].HasStock()) {
            if (racks[(size_t)targetRackId].TakeProduct()) {
                carriedItems.push_back(currentTargetProduct);
                shoppingList[currentShoppingItemIndex].acquiredQuantity++;
                gotProduct = true;

                if (!bonusProductAcquired) {
                    satisfaction = std::clamp(satisfaction + 10, 0, 100);
                    bonusProductAcquired = true;
                }
            }
        }

        if (gotProduct) {
            if (shoppingList[currentShoppingItemIndex].acquiredQuantity >= shoppingList[currentShoppingItemIndex].desiredQuantity) {
                currentShoppingItemIndex++;
            }
            state = CustomerState::BROWSING;
        } else {
            if (!penaltyNoStockApplied) {
                satisfaction = std::clamp(satisfaction - 10, 0, 100);
                penaltyNoStockApplied = true;
            }
            Say("Eh, baru saja habis barangnya.", 2.5f);
            currentShoppingItemIndex++;
            state = CustomerState::BROWSING;
        }
    }
    // -------------------------------------------------------------
    // State 7: GOING_TO_CASHIER
    // -------------------------------------------------------------
    else if (state == CustomerState::GOING_TO_CASHIER) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        Vector3 queueSlot = shop.GetCashier().GetQueuePosition(queueIndex);
        if (!waypoints.empty()) {
            waypoints.back() = queueSlot;
        }

        if (currentWaypointIndex < waypoints.size()) {
            Vector3 targetWp = waypoints[currentWaypointIndex];
            MoveTowards(targetWp, deltaTime);

            float dist = Vector3Distance(position, targetWp);
            if (dist < 0.35f) {
                currentWaypointIndex++;
            }
        } else {
            // Arrived in cashier area
            if (queueIndex == 0) {
                state = CustomerState::PAYING;
                payTimer = 0.0f;
                rotationY = PI / 2.0f; // Face cashier counter
                const std::vector<std::string> payGreetings = {
                    "Totalnya berapa?",
                    "Ini uangnya ya.",
                    "Mau bayar belanjaan ini.",
                    "Tolong hitung ya kasir."
                };
                Say(payGreetings[id % payGreetings.size()], 2.5f);
            } else {
                state = CustomerState::WAITING_FOR_CASHIER;
                rotationY = PI; // Face forward in line
                if (queueIndex >= 2 && speechCooldown <= 0.0f) {
                    Say("Antreannya lumayan ramai ya.", 2.5f);
                }
            }
        }
    }
    // -------------------------------------------------------------
    // State 8: WAITING_FOR_CASHIER
    // -------------------------------------------------------------
    else if (state == CustomerState::WAITING_FOR_CASHIER) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        totalQueueWaitTime += deltaTime;

        // Waiting time penalties scaled with patience multiplier (Tahap 14)
        float scaled5s = 5.0f * patienceMultiplier;
        float scaled10s = 10.0f * patienceMultiplier;
        float scaled20s = 20.0f * patienceMultiplier;

        if (totalQueueWaitTime > scaled20s && !penaltyWaitLongApplied) {
            satisfaction = std::clamp(satisfaction - 15, 0, 100);
            penaltyWaitLongApplied = true;
            Say("Aduh, kasirnya lama sekali...", 3.0f, Color{ 255, 230, 220, 245 }, Color{ 180, 50, 30, 255 });
        } else if (totalQueueWaitTime > scaled10s && !penaltyWaitMidApplied) {
            satisfaction = std::clamp(satisfaction - 10, 0, 100);
            penaltyWaitMidApplied = true;
        } else if (totalQueueWaitTime > scaled5s && !penaltyWaitShortApplied) {
            satisfaction = std::clamp(satisfaction - 5, 0, 100);
            penaltyWaitShortApplied = true;
        }

        // Impatient or extreme wait: Rage-quit if waiting exceeds tolerance (Tahap 14)
        if (totalQueueWaitTime >= maxQueuePatience) {
            satisfaction = std::clamp(satisfaction - 30, 0, 100);
            specificFeedback = "Antrean kasir keterlaluan lambat! Saya batalkan belanja!";
            state = CustomerState::LEAVING;
            BuildExitWaypoints(position);
            Say("Nggak tahan antrenya! Aku pergi saja!", 3.5f, Color{ 255, 220, 220, 245 }, Color{ 200, 30, 30, 255 });
            return;
        }

        // Advance towards assigned queue position
        Vector3 assignedSlot = shop.GetCashier().GetQueuePosition(queueIndex);
        float distToSlot = Vector3Distance(position, assignedSlot);
        if (distToSlot > 0.2f) {
            MoveTowards(assignedSlot, deltaTime);
        }

        // Reached front of line (Slot 0)
        if (queueIndex == 0 && distToSlot <= 0.35f) {
            state = CustomerState::PAYING;
            payTimer = 0.0f;
            rotationY = PI / 2.0f;
            Say("Akhirnya giliran saya.", 2.2f);
        }
    }
    // -------------------------------------------------------------
    // State 9: PAYING (Processes entire cart at register)
    // -------------------------------------------------------------
    else if (state == CustomerState::PAYING) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        payTimer += deltaTime;

        // Payment duration scaled slightly with item count (1.2s + 0.3s per item)
        float requiredPayTime = 1.0f + ((float)carriedItems.size() * 0.4f);

        if (payTimer >= requiredPayTime && !hasPaid) {
            int totalCartAmount = 0;
            for (auto p : carriedItems) {
                int itemPrice = 0;
                if (shop.GetCashier().ProcessPayment(id, name, p, itemPrice)) {
                    totalCartAmount += itemPrice;
                }
            }

            if (totalCartAmount > 0) {
                hasPaid = true;
                outDidPay = true;
                outPaidAmount = totalCartAmount;
                outPaidProductSummary = GetCarriedSummaryString();

                if (!bonusPaymentSuccess) {
                    satisfaction = std::clamp(satisfaction + 10, 0, 100);
                    bonusPaymentSuccess = true;
                }

                // Random thank you checkout dialogue
                const std::vector<std::string> afterPay = {
                    "Terima kasih banyak!",
                    "Sudah pas ya uangnya.",
                    "Terima kasih, sampai jumpa.",
                    "Pelayanan bagus, makasih!",
                    "Belanja selesai, terima kasih!"
                };
                Say(afterPay[(id + totalCartAmount) % afterPay.size()], 3.0f, Color{ 235, 255, 245, 245 }, Color{ 20, 120, 50, 255 });
            }

            state = CustomerState::LEAVING;
            BuildExitWaypoints(position);
        }
    }
    // -------------------------------------------------------------
    // State 10 & 11: LEAVING & EXITING
    // -------------------------------------------------------------
    else if (state == CustomerState::LEAVING || state == CustomerState::EXITING) {
        if (speechBubbleTimer > 0.0f) speechBubbleTimer -= deltaTime;
        if (speechCooldown > 0.0f) speechCooldown -= deltaTime;

        if (currentWaypointIndex < waypoints.size()) {
            Vector3 targetWp = waypoints[currentWaypointIndex];
            MoveTowards(targetWp, deltaTime);

            float dist = Vector3Distance(position, targetWp);
            if (dist < 0.4f) {
                currentWaypointIndex++;
                if (currentWaypointIndex >= 2) {
                    state = CustomerState::EXITING;
                }
            }
        } else {
            state = CustomerState::DESPAWNED;
        }
    }
}

void Customer::RenderCarriedItems() {
    if (carriedItems.empty()) return;

    Vector3 forward = { -sinf(rotationY), 0.0f, -cosf(rotationY) };
    Vector3 right = { cosf(rotationY), 0.0f, -sinf(rotationY) };

    // Render up to 3 carried items in customer basket / hands
    for (size_t i = 0; i < carriedItems.size() && i < 3; ++i) {
        ProductType pType = carriedItems[i];
        ProductInfo info = GetProductInfo(pType);

        float stackY = (float)i * 0.18f;
        Vector3 itemPos = {
            position.x + forward.x * 0.28f + right.x * 0.26f,
            position.y + (0.75f * heightScale) + stackY,
            position.z + forward.z * 0.28f + right.z * 0.26f
        };

        if (pType == ProductType::BEVERAGE) {
            DrawCube(itemPos, info.modelDimensions.x * 0.8f, info.modelDimensions.y * 0.8f, info.modelDimensions.z * 0.8f, info.primaryColor);
            DrawCubeWires(itemPos, info.modelDimensions.x * 0.8f, info.modelDimensions.y * 0.8f, info.modelDimensions.z * 0.8f, { 20, 80, 160, 255 });
        } else if (pType == ProductType::BREAD) {
            DrawCube(itemPos, info.modelDimensions.x * 0.8f, info.modelDimensions.y * 0.8f, info.modelDimensions.z * 0.8f, info.primaryColor);
            DrawCubeWires(itemPos, info.modelDimensions.x * 0.8f, info.modelDimensions.y * 0.8f, info.modelDimensions.z * 0.8f, { 140, 90, 40, 255 });
        } else if (pType == ProductType::CANNED_FOOD) {
            DrawCube(itemPos, info.modelDimensions.x * 0.8f, info.modelDimensions.y * 0.8f, info.modelDimensions.z * 0.8f, info.primaryColor);
            DrawCubeWires(itemPos, info.modelDimensions.x * 0.8f, info.modelDimensions.y * 0.8f, info.modelDimensions.z * 0.8f, { 150, 30, 30, 255 });
        }
    }
}

void Customer::Render() {
    if (state == CustomerState::DESPAWNED) return;

    bool isStill = (state == CustomerState::AT_SHELF || state == CustomerState::PAYING || state == CustomerState::WAITING_FOR_CASHIER);
    float bobOffset = isStill ? 0.0f : sinf(bobbingTimer) * 0.04f;
    float legAngle = isStill ? 0.0f : sinf(bobbingTimer) * 0.15f;

    Vector3 basePos = { position.x, position.y + bobOffset, position.z };

    // 1. Legs
    float legHeight = 0.65f * heightScale;
    Vector3 leftLegPos = { basePos.x - 0.14f, basePos.y + legHeight / 2.0f, basePos.z - legAngle };
    Vector3 rightLegPos = { basePos.x + 0.14f, basePos.y + legHeight / 2.0f, basePos.z + legAngle };

    DrawCube(leftLegPos, 0.18f, legHeight, 0.22f, pantsColor);
    DrawCubeWires(leftLegPos, 0.18f, legHeight, 0.22f, { 25, 25, 30, 255 });

    DrawCube(rightLegPos, 0.18f, legHeight, 0.22f, pantsColor);
    DrawCubeWires(rightLegPos, 0.18f, legHeight, 0.22f, { 25, 25, 30, 255 });

    // 2. Torso / Body
    float bodyHeight = 0.75f * heightScale;
    Vector3 torsoPos = { basePos.x, basePos.y + legHeight + bodyHeight / 2.0f, basePos.z };
    DrawCube(torsoPos, 0.55f, bodyHeight, 0.35f, shirtColor);
    DrawCubeWires(torsoPos, 0.55f, bodyHeight, 0.35f, { 30, 30, 35, 255 });

    // 3. Head
    Vector3 headPos = { basePos.x, basePos.y + legHeight + bodyHeight + 0.22f, basePos.z };
    DrawSphere(headPos, 0.22f, bodyColor);
    DrawSphereWires(headPos, 0.22f, 8, 8, { 180, 140, 120, 255 });

    // 4. Hair / Hat accessory based on CustomerType (Tahap 14 Visual Variation)
    Vector3 hairPos = { basePos.x, headPos.y + 0.14f, basePos.z };
    Color hatColor = (type == CustomerType::BIG_SHOPPER) ? Color{ 230, 126, 34, 255 } :
                     (type == CustomerType::IMPATIENT) ? Color{ 192, 57, 43, 255 } :
                     (type == CustomerType::PATIENT) ? Color{ 46, 204, 113, 255 } :
                     (type == CustomerType::PRICE_SENSITIVE) ? Color{ 142, 68, 173, 255 } : Color{ 52, 73, 94, 255 };
    DrawCube(hairPos, 0.38f, 0.12f, 0.38f, hatColor);

    // 5. Render Carried Products (Multi-item stack)
    RenderCarriedItems();

    // 6. Overhead Status Marker (Indicates State / Satisfaction mood)
    Vector3 tagPos = { basePos.x, headPos.y + 0.55f, basePos.z };
    Color statusMarkerColor = (state == CustomerState::PAYING) ? Color{ 0, 255, 128, 255 } :
                              (state == CustomerState::WAITING_FOR_CASHIER || state == CustomerState::GOING_TO_CASHIER) ? Color{ 255, 165, 0, 255 } :
                              (state == CustomerState::AT_SHELF) ? Color{ 255, 215, 0, 255 } :
                              (satisfaction < 50) ? Color{ 231, 76, 60, 255 } :
                              hasPaid ? Color{ 50, 205, 50, 255 } : Color{ 100, 180, 255, 255 };
    DrawCube(tagPos, 0.12f, 0.12f, 0.12f, statusMarkerColor);
}

void Customer::RenderSpeechBubble() {
    // 3D Billboard stub if needed
}

void Customer::RenderSpeechBubble2D(Camera3D camera, int screenWidth, int screenHeight) {
    if (state == CustomerState::DESPAWNED || speechBubbleTimer <= 0.0f || speechBubbleText.empty()) {
        return;
    }

    // Position above NPC head
    Vector3 headWorldPos = { position.x, position.y + (1.75f * heightScale) + 0.45f, position.z };

    // Check if NPC is in front of the camera (forward dot product)
    Vector3 camForward = Vector3Subtract(camera.target, camera.position);
    Vector3 toNpc = Vector3Subtract(headWorldPos, camera.position);
    float dot = (camForward.x * toNpc.x) + (camForward.y * toNpc.y) + (camForward.z * toNpc.z);
    if (dot <= 0.2f) {
        // NPC is behind the camera or directly beside -> DO NOT RENDER
        return;
    }

    Vector2 screenPos = GetWorldToScreen(headWorldPos, camera);

    // Only render if within visible screen bounds
    if (screenPos.x < 0 || screenPos.x > screenWidth || screenPos.y < 0 || screenPos.y > screenHeight) {
        return;
    }

    // Measure text
    int fontSize = 13;
    int textWidth = MeasureText(speechBubbleText.c_str(), fontSize);
    int bubbleWidth = textWidth + 24;
    int bubbleHeight = 28;

    int bx = (int)screenPos.x - (bubbleWidth / 2);
    int by = (int)screenPos.y - bubbleHeight - 8;

    // Fade out during last 0.5 seconds
    float alpha = (speechBubbleTimer < 0.5f) ? (speechBubbleTimer / 0.5f) : 1.0f;
    Color bg = speechBubbleColor;
    bg.a = (unsigned char)(240 * alpha);
    Color border = speechTextColor;
    border.a = (unsigned char)(220 * alpha);
    Color textCol = speechTextColor;
    textCol.a = (unsigned char)(255 * alpha);

    // Draw speech bubble background & outline
    DrawRectangle(bx, by, bubbleWidth, bubbleHeight, bg);
    DrawRectangleLines(bx, by, bubbleWidth, bubbleHeight, border);

    // Draw bubble pointer arrow pointing down to NPC
    Vector2 p1 = { (float)screenPos.x - 5, (float)(by + bubbleHeight) };
    Vector2 p2 = { (float)screenPos.x + 5, (float)(by + bubbleHeight) };
    Vector2 p3 = { (float)screenPos.x, (float)(by + bubbleHeight + 7) };
    DrawTriangle(p1, p2, p3, bg);
    DrawLine((int)p1.x, (int)p1.y, (int)p3.x, (int)p3.y, border);
    DrawLine((int)p2.x, (int)p2.y, (int)p3.x, (int)p3.y, border);

    // Draw dialogue text
    DrawText(speechBubbleText.c_str(), bx + 12, by + 7, fontSize, textCol);
}

