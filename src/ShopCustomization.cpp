#include "ShopCustomization.hpp"
#include "Finance.hpp"
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>

ShopCustomization& ShopCustomization::Instance() {
    static ShopCustomization instance;
    return instance;
}

ShopCustomization::ShopCustomization()
    : currentFloor(FloorStyle::BASIC_TILE),
      currentWall(WallStyle::BASIC_WHITE),
      currentLightTone(LightTone::WARM_WHITE),
      lightIntensity(1.0f),
      currentSignboard(SignboardStyle::MINIMALIST_WOOD),
      shopName("ANDIKA MART"),
      isEditingPlacement(false),
      heldFurnitureId(0),
      heldDecorId(0),
      heldObjectName(""),
      heldPosition{ 0.0f, 0.0f, 0.0f },
      heldRotationY(0.0f),
      heldSize{ 1.0f, 1.0f, 1.0f },
      originalPosition{ 0.0f, 0.0f, 0.0f },
      originalRotationY(0.0f),
      gridSnapping(true),
      gridSize(0.5f),
      isPlacementValid(true),
      validationMsg("Posisi Valid"),
      menuOpen(false),
      currentTab(0),
      selectedIndex(0)
{
    Init();
}

void ShopCustomization::Init() {
    currentFloor = FloorStyle::BASIC_TILE;
    currentWall = WallStyle::BASIC_WHITE;
    currentLightTone = LightTone::WARM_WHITE;
    lightIntensity = 1.0f;
    currentSignboard = SignboardStyle::MINIMALIST_WOOD;
    shopName = "ANDIKA MART";

    isEditingPlacement = false;
    heldFurnitureId = 0;
    heldDecorId = 0;
    gridSnapping = true;
    gridSize = 0.5f;
    menuOpen = false;
    currentTab = 0;
    selectedIndex = 0;

    // Seed Decoration Catalog
    decorations.clear();
    
    // 1. Monstera Indoor Plant (Tanaman Hias Monstera)
    decorations.push_back({
        1, "Monstera Indoor Pot (Plant)", 1, 20000, false, false,
        { -6.5f, 0.5f, 9.5f }, 0.0f, { 0.7f, 1.1f, 0.7f },
        { 160, 100, 60, 255 }, { 46, 204, 113, 255 },
        "Tanaman monstera pot estetik penyegar sudut toko"
    });

    // 2. Modern Wall Clock (Jam Dinding Modern)
    decorations.push_back({
        2, "Jam Dinding Toko (Wall Clock)", 2, 15000, false, false,
        { 0.0f, 3.8f, -11.9f }, 0.0f, { 0.8f, 0.8f, 0.12f },
        { 45, 52, 60, 255 }, { 241, 196, 15, 255 },
        "Jam dinding elegan untuk melihat waktu belanja"
    });

    // 3. Discount Promotional Poster (Poster Promosi Toko)
    decorations.push_back({
        3, "Poster Promo Spesial (Promo Poster)", 3, 10000, false, false,
        { -9.9f, 2.5f, 0.0f }, 90.0f, { 0.1f, 1.4f, 1.0f },
        { 231, 76, 60, 255 }, { 255, 255, 255, 255 },
        "Poster dinding promosi belanja menarik perhatian customer"
    });

    // 4. Standing Advertising Board (Papan Promosi Berdiri)
    decorations.push_back({
        4, "Papan Iklan Berdiri (A-Frame Sign)", 4, 30000, false, false,
        { -2.0f, 0.6f, 13.5f }, 0.0f, { 0.8f, 1.2f, 0.6f },
        { 120, 80, 40, 255 }, { 241, 196, 15, 255 },
        "Papan A-Frame di depan pintu toko menyambut pembeli"
    });

    // 5. Hanging Ceiling Lamp (Lampu Gantung Estetik)
    decorations.push_back({
        5, "Lampu Gantung Cafe (Pendant Lamp)", 5, 45000, false, false,
        { 0.0f, 4.2f, 3.0f }, 0.0f, { 0.6f, 0.9f, 0.6f },
        { 40, 40, 45, 255 }, { 255, 220, 100, 255 },
        "Lampu gantung temaram yang menambah suasana mewah"
    });

    // 6. Premium Welcome Floor Mat (Keset Pintu Mewah)
    decorations.push_back({
        6, "Karpet Pintu Emas (Luxury Mat)", 6, 25000, false, false,
        { 0.0f, 0.02f, 9.8f }, 0.0f, { 2.8f, 0.04f, 1.4f },
        { 30, 40, 60, 255 }, { 218, 165, 32, 255 },
        "Karpet selamat datang biru tua bermotif ornamen emas"
    });
}

void ShopCustomization::SetLightIntensity(float val) {
    lightIntensity = std::clamp(val, 0.6f, 1.4f);
}

Color ShopCustomization::GetFloorPrimaryColor() const {
    switch (currentFloor) {
        case FloorStyle::WOOD_PARQUET:    return Color{ 180, 130, 85, 255 };  // Warm parquet brown
        case FloorStyle::CHECKERED_RETRO: return Color{ 220, 220, 225, 255 };  // Base white retro tile
        case FloorStyle::MARBLE_LUXURY:   return Color{ 240, 238, 230, 255 };  // Polished ivory marble
        case FloorStyle::MODERN_CONCRETE: return Color{ 70, 75, 82, 255 };     // Dark sleek slate concrete
        case FloorStyle::BASIC_TILE:
        default:                          return Color{ 245, 245, 250, 255 };  // Clean white tile
    }
}

Color ShopCustomization::GetFloorGridColor() const {
    switch (currentFloor) {
        case FloorStyle::WOOD_PARQUET:    return Color{ 130, 85, 45, 255 };
        case FloorStyle::CHECKERED_RETRO: return Color{ 40, 45, 55, 255 };
        case FloorStyle::MARBLE_LUXURY:   return Color{ 218, 165, 32, 180 };
        case FloorStyle::MODERN_CONCRETE: return Color{ 45, 50, 58, 255 };
        case FloorStyle::BASIC_TILE:
        default:                          return Color{ 190, 200, 210, 255 };
    }
}

Color ShopCustomization::GetWallPrimaryColor() const {
    switch (currentWall) {
        case WallStyle::PAINTED_PASTEL:    return Color{ 245, 235, 220, 255 }; // Warm cream
        case WallStyle::BRICK_EXPOSED:     return Color{ 175, 85, 65, 255 };   // Brick red-brown
        case WallStyle::MODERN_PANEL:      return Color{ 55, 62, 72, 255 };    // Dark modern slate
        case WallStyle::PREMIUM_WALLPAPER: return Color{ 40, 48, 65, 255 };    // Deep royal navy
        case WallStyle::BASIC_WHITE:
        default:                           return Color{ 220, 225, 230, 255 }; // Light gray/white
    }
}

Color ShopCustomization::GetWallSecondaryColor() const {
    switch (currentWall) {
        case WallStyle::PAINTED_PASTEL:    return Color{ 180, 215, 230, 255 }; // Sky blue accent
        case WallStyle::BRICK_EXPOSED:     return Color{ 130, 60, 45, 255 };   // Dark mortar accent
        case WallStyle::MODERN_PANEL:      return Color{ 180, 130, 80, 255 };  // Wood slat accent
        case WallStyle::PREMIUM_WALLPAPER: return Color{ 218, 165, 32, 255 };  // Gold geometric trim
        case WallStyle::BASIC_WHITE:
        default:                           return Color{ 100, 110, 120, 255 };
    }
}

Color ShopCustomization::GetCeilingLightColor() const {
    Color baseCol = RAYWHITE;
    switch (currentLightTone) {
        case LightTone::NATURAL_DAYLIGHT:  baseCol = Color{ 240, 248, 255, 255 }; break;
        case LightTone::COOL_SUPERMARKET:  baseCol = Color{ 225, 240, 255, 255 }; break;
        case LightTone::CYBER_NEON:        baseCol = Color{ 100, 240, 255, 255 }; break;
        case LightTone::WARM_WHITE:
        default:                           baseCol = Color{ 255, 235, 180, 255 }; break;
    }
    return baseCol;
}

void ShopCustomization::StartMovingFurniture(int furnitureId, Vector3 curPos, float curRot, Vector3 size, const std::string& name) {
    isEditingPlacement = true;
    heldFurnitureId = furnitureId;
    heldDecorId = 0;
    heldObjectName = name;
    heldPosition = curPos;
    heldRotationY = curRot;
    heldSize = size;
    originalPosition = curPos;
    originalRotationY = curRot;
    isPlacementValid = true;
    validationMsg = "Posisi Siap";
}

void ShopCustomization::StartMovingDecoration(int decorId, Vector3 curPos, float curRot, Vector3 size, const std::string& name) {
    isEditingPlacement = true;
    heldFurnitureId = 0;
    heldDecorId = decorId;
    heldObjectName = name;
    heldPosition = curPos;
    heldRotationY = curRot;
    heldSize = size;
    originalPosition = curPos;
    originalRotationY = curRot;
    isPlacementValid = true;
    validationMsg = "Posisi Siap";
}

void ShopCustomization::CancelPlacement() {
    isEditingPlacement = false;
    heldFurnitureId = 0;
    heldDecorId = 0;
    heldObjectName = "";
}

bool ShopCustomization::ConfirmPlacement(Vector3& outPos, float& outRot, std::string& outFeedback) {
    if (!isEditingPlacement) {
        outFeedback = "Tidak ada objek yang sedang dipindahkan.";
        return false;
    }

    if (!isPlacementValid) {
        outFeedback = "Gagal: " + validationMsg + "!";
        return false;
    }

    outPos = heldPosition;
    outRot = heldRotationY;

    if (heldDecorId > 0) {
        for (auto& d : decorations) {
            if (d.id == heldDecorId) {
                d.position = heldPosition;
                d.rotationY = heldRotationY;
                d.isPlaced = true;
                break;
            }
        }
    }

    outFeedback = "Posisi " + heldObjectName + " berhasil diperbarui!";
    isEditingPlacement = false;
    heldFurnitureId = 0;
    heldDecorId = 0;
    heldObjectName = "";
    return true;
}

void ShopCustomization::RotateHeldObject(float deltaDegrees) {
    heldRotationY += deltaDegrees;
    while (heldRotationY >= 360.0f) heldRotationY -= 360.0f;
    while (heldRotationY < 0.0f) heldRotationY += 360.0f;
}

void ShopCustomization::CycleGridSize() {
    if (gridSize <= 0.25f) gridSize = 0.5f;
    else if (gridSize <= 0.5f) gridSize = 1.0f;
    else gridSize = 0.25f;
}

void ShopCustomization::UpdatePlacementMovement(Vector3 playerEyePos, Vector3 playerLookDir, float shopWidth, float shopLength) {
    if (!isEditingPlacement) return;

    // Raycast against floor plane (Y = 0)
    // Ray: P = Eye + t * Dir -> 0 = Eye.y + t * Dir.y -> t = -Eye.y / Dir.y
    if (fabsf(playerLookDir.y) > 0.001f) {
        float t = (heldPosition.y - playerEyePos.y) / playerLookDir.y;
        if (t > 0.5f && t < 15.0f) {
            Vector3 hitPos = Vector3Add(playerEyePos, Vector3Scale(playerLookDir, t));
            
            // Grid Snapping
            if (gridSnapping && gridSize > 0.01f) {
                hitPos.x = std::round(hitPos.x / gridSize) * gridSize;
                hitPos.z = std::round(hitPos.z / gridSize) * gridSize;
            }

            heldPosition.x = hitPos.x;
            heldPosition.z = hitPos.z;
        } else {
            // Keep at fixed distance in front of player
            Vector3 frontPos = Vector3Add(playerEyePos, Vector3Scale(playerLookDir, 3.2f));
            if (gridSnapping && gridSize > 0.01f) {
                frontPos.x = std::round(frontPos.x / gridSize) * gridSize;
                frontPos.z = std::round(frontPos.z / gridSize) * gridSize;
            }
            heldPosition.x = frontPos.x;
            heldPosition.z = frontPos.z;
        }
    }

    ValidateCurrentPlacement(shopWidth, shopLength);
}

void ShopCustomization::ValidateCurrentPlacement(float shopWidth, float shopLength) {
    isPlacementValid = true;
    validationMsg = "Posisi Siap Ditempatkan [ENTER / E]";

    float halfW = shopWidth / 2.0f;
    float halfL = shopLength / 2.0f;
    float margin = 0.6f;

    // 1. Boundary check inside shop floor
    if (heldPosition.x < (-halfW + margin) || heldPosition.x > (halfW - margin) ||
        heldPosition.z < (-halfL + margin) || heldPosition.z > (halfL - margin)) {
        isPlacementValid = false;
        validationMsg = "Invalid: Objek berada di luar batas ruangan toko!";
        return;
    }

    // 2. Doorway blockage check (Door is at South Wall: X from -2.0 to 2.0, Z > halfL - 3.0)
    if (fabsf(heldPosition.x) < 2.2f && heldPosition.z > (halfL - 3.2f)) {
        isPlacementValid = false;
        validationMsg = "Invalid: Menghalangi akses pintu masuk/keluar!";
        return;
    }

    // 3. Cashier Queue & Counter area check (X ~ 4.0 to 7.0, Z ~ 6.5 to 10.0)
    if (heldPosition.x > 3.5f && heldPosition.x < 7.5f && heldPosition.z > 6.0f && heldPosition.z < 10.5f) {
        if (heldFurnitureId != 4) { // Allow cabinet behind cashier
            isPlacementValid = false;
            validationMsg = "Invalid: Terlalu dekat dengan meja/antrean kasir!";
            return;
        }
    }

    // 4. Storage Pallet area check (X ~ 0.0 to 6.5, Z ~ -11.0 to -8.0)
    if (heldPosition.x > 0.0f && heldPosition.x < 7.0f && heldPosition.z > -11.5f && heldPosition.z < -7.5f) {
        isPlacementValid = false;
        validationMsg = "Invalid: Menghalangi area pallet penyimpanan gudang!";
        return;
    }
}

void ShopCustomization::RenderPlacementPreview(int shopWidth, int shopLength) {
    if (!isEditingPlacement) return;

    Color previewColor = isPlacementValid ? Color{ 46, 204, 113, 160 } : Color{ 231, 76, 60, 160 };
    Color wireColor = isPlacementValid ? Color{ 100, 255, 150, 255 } : Color{ 255, 100, 100, 255 };

    // Hologram Box Preview
    DrawCube(heldPosition, heldSize.x, heldSize.y, heldSize.z, previewColor);
    DrawCubeWires(heldPosition, heldSize.x, heldSize.y, heldSize.z, wireColor);

    // Ground Ring Target Marker
    DrawCircle3D(Vector3{ heldPosition.x, 0.02f, heldPosition.z }, heldSize.x * 0.75f, Vector3{ 1, 0, 0 }, 90.0f, wireColor);

    // Direction pointer
    float rad = heldRotationY * DEG2RAD;
    Vector3 forwardVector = { sinf(rad) * (heldSize.z * 0.7f), 0.05f, cosf(rad) * (heldSize.z * 0.7f) };
    DrawLine3D(Vector3{ heldPosition.x, 0.05f, heldPosition.z }, Vector3Add(Vector3{ heldPosition.x, 0.05f, heldPosition.z }, forwardVector), wireColor);
}

bool ShopCustomization::PurchaseDecoration(int decorIndex, Finance& finance, std::string& outFeedback) {
    if (decorIndex < 0 || decorIndex >= (int)decorations.size()) return false;
    auto& d = decorations[decorIndex];
    if (d.isOwned) {
        outFeedback = d.name + " sudah dimiliki!";
        return false;
    }

    if (finance.GetCurrentBalance() < d.price) {
        outFeedback = "Saldo tidak cukup! Butuh Rp" + std::to_string(d.price) + " (Saldo: Rp" + std::to_string(finance.GetCurrentBalance()) + ")";
        return false;
    }

    if (!finance.RecordExpense(d.price, "Beli Dekorasi: " + d.name)) {
        outFeedback = "Gagal memproses transaksi!";
        return false;
    }

    d.isOwned = true;
    d.isPlaced = true;
    outFeedback = "Dekorasi " + d.name + " berhasil dibeli & dipasang!";
    return true;
}

void ShopCustomization::RemoveDecoration(int decorId) {
    for (auto& d : decorations) {
        if (d.id == decorId) {
            d.isPlaced = false;
            return;
        }
    }
}

void ShopCustomization::DrawSingleDecoration(const DecorationItem& item) {
    if (!item.isOwned || !item.isPlaced) return;

    switch (item.typeId) {
        case 1: { // Monstera Plant
            DrawCube(Vector3{ item.position.x, item.position.y - 0.2f, item.position.z }, 0.5f, 0.5f, 0.5f, item.primaryColor);
            DrawSphere(Vector3{ item.position.x, item.position.y + 0.3f, item.position.z }, 0.40f, item.secondaryColor);
            DrawSphere(Vector3{ item.position.x + 0.1f, item.position.y + 0.5f, item.position.z - 0.1f }, 0.30f, Color{ 39, 174, 96, 255 });
            break;
        }
        case 2: { // Modern Wall Clock
            DrawCube(item.position, item.size.x, item.size.y, item.size.z, item.primaryColor);
            DrawCubeWires(item.position, item.size.x, item.size.y, item.size.z, item.secondaryColor);
            DrawCircle3D(Vector3{ item.position.x, item.position.y, item.position.z + 0.08f }, 0.32f, Vector3{ 0, 1, 0 }, 0.0f, RAYWHITE);
            break;
        }
        case 3: { // Poster
            DrawCube(item.position, item.size.x, item.size.y, item.size.z, item.primaryColor);
            DrawCubeWires(item.position, item.size.x, item.size.y, item.size.z, item.secondaryColor);
            break;
        }
        case 4: { // A-Frame Standing Sign
            DrawCube(item.position, item.size.x, item.size.y, item.size.z, item.primaryColor);
            DrawCubeWires(item.position, item.size.x, item.size.y, item.size.z, item.secondaryColor);
            break;
        }
        case 5: { // Hanging Pendant Lamp
            DrawCylinder(Vector3{ item.position.x, item.position.y + 0.5f, item.position.z }, 0.02f, 0.02f, 0.8f, 6, Color{ 30, 30, 30, 255 });
            DrawSphere(Vector3{ item.position.x, item.position.y, item.position.z }, 0.32f, item.primaryColor);
            DrawSphere(Vector3{ item.position.x, item.position.y - 0.15f, item.position.z }, 0.18f, item.secondaryColor);
            break;
        }
        case 6: { // Luxury Floor Mat
            DrawCube(item.position, item.size.x, item.size.y, item.size.z, item.primaryColor);
            DrawCubeWires(item.position, item.size.x, item.size.y, item.size.z, item.secondaryColor);
            break;
        }
        default:
            DrawCube(item.position, item.size.x, item.size.y, item.size.z, item.primaryColor);
            break;
    }
}

void ShopCustomization::RenderDecorations() {
    for (const auto& item : decorations) {
        DrawSingleDecoration(item);
    }
}

std::vector<AABB> ShopCustomization::GetDecorationColliders() const {
    std::vector<AABB> colliders;
    for (const auto& d : decorations) {
        if (d.isOwned && d.isPlaced && d.typeId != 2 && d.typeId != 3 && d.typeId != 5 && d.typeId != 6) { // Wall clock, poster, ceiling lamp, floor mat have no physical collision
            AABB aabb;
            aabb.min = { d.position.x - d.size.x / 2.0f, 0.0f, d.position.z - d.size.z / 2.0f };
            aabb.max = { d.position.x + d.size.x / 2.0f, d.position.y + d.size.y / 2.0f, d.position.z + d.size.z / 2.0f };
            colliders.push_back(aabb);
        }
    }
    return colliders;
}

void ShopCustomization::RenderSignboard(float shopWidth, float shopLength, float shopHeight) {
    float boardW = std::min(8.5f, shopWidth * 0.55f);
    float boardH = 1.4f;
    float boardThick = 0.4f;
    Vector3 boardPos = { 0.0f, shopHeight + 0.8f, (shopLength / 2.0f) + 0.25f };

    Color boardBg = Color{ 40, 45, 55, 255 };
    Color trimColor = Color{ 241, 196, 15, 255 };

    if (currentSignboard == SignboardStyle::MINIMALIST_WOOD) {
        boardBg = Color{ 139, 90, 43, 255 };
        trimColor = Color{ 255, 230, 150, 255 };
    } else if (currentSignboard == SignboardStyle::NEON_GLOW) {
        boardBg = Color{ 20, 25, 40, 255 };
        trimColor = Color{ 0, 230, 255, 255 };
    } else if (currentSignboard == SignboardStyle::MODERN_LED) {
        boardBg = Color{ 35, 42, 50, 255 };
        trimColor = Color{ 46, 204, 113, 255 };
    } else if (currentSignboard == SignboardStyle::VINTAGE_RETRO) {
        boardBg = Color{ 180, 50, 50, 255 };
        trimColor = Color{ 255, 215, 0, 255 };
    }

    // Main Signboard Structure
    DrawCube(boardPos, boardW, boardH, boardThick, boardBg);
    DrawCubeWires(boardPos, boardW, boardH, boardThick, trimColor);

    // Subtle 3D Letter Plates on Signboard Front
    int letterCount = std::min(10, (int)shopName.length());
    float letterSpacing = (boardW - 1.2f) / std::max(1, letterCount);
    for (int l = 0; l < letterCount; ++l) {
        float xOff = -boardW / 2.0f + 0.6f + l * letterSpacing;
        DrawCube(Vector3{ boardPos.x + xOff, boardPos.y, boardPos.z + boardThick / 2.0f + 0.03f }, letterSpacing * 0.7f, boardH * 0.55f, 0.06f, trimColor);
    }
}

void ShopCustomization::ToggleMenu() {
    menuOpen = !menuOpen;
}

void ShopCustomization::SetMenuOpen(bool open) {
    menuOpen = open;
}

void ShopCustomization::NextTab() {
    currentTab = (currentTab + 1) % 5;
    selectedIndex = 0;
}

void ShopCustomization::PreviousTab() {
    currentTab = (currentTab - 1 + 5) % 5;
    selectedIndex = 0;
}

void ShopCustomization::NextItem() {
    if (currentTab == 1) {
        if (!decorations.empty()) {
            selectedIndex = (selectedIndex + 1) % decorations.size();
        }
    }
}

void ShopCustomization::PreviousItem() {
    if (currentTab == 1) {
        if (!decorations.empty()) {
            selectedIndex = (selectedIndex - 1 + (int)decorations.size()) % decorations.size();
        }
    }
}

void ShopCustomization::RenderUI(int screenWidth, int screenHeight, int currentBalance) {
    if (!menuOpen) return;

    DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 0, 0, 0, 180 });

    int modalW = 900;
    int modalH = 560;
    int modalX = (screenWidth - modalW) / 2;
    int modalY = (screenHeight - modalH) / 2;

    // Main Modal Box
    DrawRectangle(modalX, modalY, modalW, modalH, Color{ 20, 26, 36, 252 });
    DrawRectangleLines(modalX, modalY, modalW, modalH, Color{ 0, 200, 255, 255 });

    // Header Title
    DrawText("KUSTOMISASI & TATA LETAK TOKO (STAGE 22)", modalX + 30, modalY + 16, 20, Color{ 0, 200, 255, 255 });
    std::string subHead = "Tata letak bebas furniture, dekorasi interior, gaya lantai/dinding, pencahayaan, & papan nama toko";
    DrawText(subHead.c_str(), modalX + 30, modalY + 40, 12, Color{ 170, 190, 210, 255 });

    // 5 Navigation Tabs Bar
    int tabY = modalY + 62;
    int tabW = 160;
    int tabH = 30;

    const char* tabLabels[5] = {
        "1. Tata Letak (Move)",
        "2. Dekorasi Toko",
        "3. Lantai & Dinding",
        "4. Pencahayaan",
        "5. Papan Nama"
    };

    for (int t = 0; t < 5; ++t) {
        int tX = modalX + 30 + t * (tabW + 10);
        bool isAct = (currentTab == t);
        DrawRectangle(tX, tabY, tabW, tabH, isAct ? Color{ 0, 180, 230, 220 } : Color{ 30, 40, 52, 200 });
        DrawRectangleLines(tX, tabY, tabW, tabH, isAct ? Color{ 255, 255, 255, 255 } : Color{ 60, 75, 90, 255 });
        DrawText(tabLabels[t], tX + 10, tabY + 8, 12, isAct ? Color{ 15, 25, 30, 255 } : RAYWHITE);
    }

    // ==========================================
    // TAB 0: TATA LETAK & RELOKASI FURNITURE (Move & Rotate)
    // ==========================================
    if (currentTab == 0) {
        int contentY = tabY + 45;
        
        // Guidance Banner
        DrawRectangle(modalX + 30, contentY, modalW - 60, 105, Color{ 25, 38, 55, 240 });
        DrawRectangleLines(modalX + 30, contentY, modalW - 60, 105, Color{ 52, 152, 219, 255 });

        DrawText("PANDUAN MODE RELOKASI FURNITURE & DEKORASI BEBAS:", modalX + 45, contentY + 12, 14, Color{ 255, 215, 0, 255 });
        DrawText("1. Tutup menu [C / ESC] dan arahkan kursor ke Furniture / Dekorasi yang ingin dipindahkan.", modalX + 45, contentY + 36, 12, RAYWHITE);
        DrawText("2. Tekan [R] saat membidik objek untuk memulai Mode Pemindahan (Free Placement).", modalX + 45, contentY + 54, 12, Color{ 100, 230, 255, 255 });
        DrawText("3. Gunakan [Mouse / WASD] untuk mengarahkan posisi, [Q / E] untuk Memutar Rotasi (15 deg / 45 deg).", modalX + 45, contentY + 72, 12, Color{ 241, 196, 15, 255 });
        DrawText("4. Tekan [G] untuk On/Off Grid Snapping (0.5m), [ENTER / E] untuk Konfirmasi, [ESC] Batal.", modalX + 45, contentY + 90, 12, Color{ 46, 204, 113, 255 });

        // Placement Settings Box
        int optY = contentY + 120;
        DrawRectangle(modalX + 30, optY, modalW - 60, 110, Color{ 22, 28, 38, 240 });
        DrawRectangleLines(modalX + 30, optY, modalW - 60, 110, Color{ 60, 75, 90, 255 });

        DrawText("PENGATURAN PENEMPATAN (PLACEMENT SETTINGS):", modalX + 45, optY + 12, 13, Color{ 200, 220, 240, 255 });

        std::string gridStr = "Grid Snapping: " + std::string(gridSnapping ? "AKTIF [0.5 Meter]" : "NON-AKTIF (Free Continuous)");
        DrawText(gridStr.c_str(), modalX + 45, optY + 38, 13, gridSnapping ? Color{ 46, 204, 113, 255 } : Color{ 230, 126, 34, 255 });

        DrawText("Status Validasi: Proteksi dinding, pintu masuk kasir & area lorong customer aktif.", modalX + 45, optY + 62, 12, Color{ 180, 195, 210, 255 });
        DrawText("Rotasi Halus: Mendukung rotasi 0 s.d. 360 derajat untuk fleksibilitas sudut estetik.", modalX + 45, optY + 82, 12, Color{ 180, 195, 210, 255 });

        // Hotkey Tips
        int tipY = optY + 125;
        DrawText("[TIPS] Furniture dan dekorasi yang dipindahkan akan tersimpan secara permanen di savegame!", modalX + 45, tipY, 12, Color{ 255, 215, 0, 255 });
    }
    // ==========================================
    // TAB 1: DEKORASI INTERIOR (Decorations)
    // ==========================================
    else if (currentTab == 1) {
        int listY = tabY + 40;
        int cardW = (modalW - 75) / 2;
        int cardH = 80;

        for (size_t i = 0; i < decorations.size() && i < 6; ++i) {
            const auto& d = decorations[i];
            bool isSel = ((int)i == selectedIndex);
            int col = (int)i % 2;
            int row = (int)i / 2;
            int cX = modalX + 30 + col * (cardW + 15);
            int cY = listY + row * (cardH + 10);

            Color bgCol = isSel ? Color{ 35, 65, 95, 240 } : Color{ 25, 32, 44, 200 };
            Color borderCol = isSel ? Color{ 0, 200, 255, 255 } : Color{ 55, 65, 75, 200 };

            DrawRectangle(cX, cY, cardW, cardH, bgCol);
            DrawRectangleLines(cX, cY, cardW, cardH, borderCol);

            // Icon
            DrawRectangle(cX + 12, cY + 15, 48, 48, d.primaryColor);
            DrawRectangleLines(cX + 12, cY + 15, 48, 48, d.secondaryColor);

            // Title & Description
            DrawText(d.name.c_str(), cX + 68, cY + 10, 13, isSel ? Color{ 255, 230, 100, 255 } : RAYWHITE);
            DrawText(d.description.c_str(), cX + 68, cY + 30, 10, Color{ 170, 185, 200, 255 });

            // Status / Price
            if (d.isOwned) {
                std::string st = d.isPlaced ? "TERPASANG (OWNED)" : "DISIMPAN (INVENTORY)";
                DrawText(st.c_str(), cX + 68, cY + 52, 11, d.isPlaced ? Color{ 46, 204, 113, 255 } : Color{ 241, 196, 15, 255 });
            } else {
                std::string pStr = "Beli: Rp" + std::to_string(d.price);
                DrawText(pStr.c_str(), cX + 68, cY + 52, 12, Color{ 255, 215, 0, 255 });
            }
        }

        // Action Hints
        int actY = modalY + 440;
        DrawRectangle(modalX + 30, actY, modalW - 60, 48, Color{ 18, 24, 32, 240 });
        DrawRectangleLines(modalX + 30, actY, modalW - 60, 48, Color{ 60, 75, 90, 255 });

        DrawText("[W / S / Panah] Pilih Dekorasi    [ENTER] Beli / Pasang Dekorasi    [R] Masuk Mode Pindah Objek",
                 modalX + 45, actY + 15, 12, Color{ 255, 220, 120, 255 });
    }
    // ==========================================
    // TAB 2: LANTAI & DINDING (Floor & Wall Styles)
    // ==========================================
    else if (currentTab == 2) {
        int contentY = tabY + 40;
        int halfW = (modalW - 75) / 2;

        // Left Column: Floor Styles
        DrawText("PILIHAN GAYA LANTAI (FLOOR STYLE):", modalX + 30, contentY, 13, Color{ 0, 200, 255, 255 });
        int fY = contentY + 22;

        FloorStyle fStyles[5] = {
            FloorStyle::BASIC_TILE, FloorStyle::WOOD_PARQUET, FloorStyle::CHECKERED_RETRO,
            FloorStyle::MARBLE_LUXURY, FloorStyle::MODERN_CONCRETE
        };

        for (int i = 0; i < 5; ++i) {
            FloorStyle st = fStyles[i];
            bool isCur = (currentFloor == st);
            int itemY = fY + i * 44;

            DrawRectangle(modalX + 30, itemY, halfW, 38, isCur ? Color{ 35, 65, 95, 240 } : Color{ 24, 30, 42, 200 });
            DrawRectangleLines(modalX + 30, itemY, halfW, 38, isCur ? Color{ 46, 204, 113, 255 } : Color{ 55, 65, 75, 200 });

            std::string label = "[" + std::to_string(i + 1) + "] " + GetFloorStyleName(st);
            DrawText(label.c_str(), modalX + 45, itemY + 11, 12, isCur ? Color{ 255, 230, 100, 255 } : RAYWHITE);

            if (isCur) {
                DrawText("AKTIF", modalX + halfW - 25, itemY + 11, 11, Color{ 46, 204, 113, 255 });
            }
        }

        // Right Column: Wall Styles
        int wX = modalX + 30 + halfW + 15;
        DrawText("PILIHAN GAYA DINDING (WALL STYLE):", wX, contentY, 13, Color{ 241, 196, 15, 255 });
        int wY = contentY + 22;

        WallStyle wStyles[5] = {
            WallStyle::BASIC_WHITE, WallStyle::PAINTED_PASTEL, WallStyle::BRICK_EXPOSED,
            WallStyle::MODERN_PANEL, WallStyle::PREMIUM_WALLPAPER
        };

        for (int i = 0; i < 5; ++i) {
            WallStyle wst = wStyles[i];
            bool isCur = (currentWall == wst);
            int itemY = wY + i * 44;

            DrawRectangle(wX, itemY, halfW, 38, isCur ? Color{ 45, 55, 75, 240 } : Color{ 24, 30, 42, 200 });
            DrawRectangleLines(wX, itemY, halfW, 38, isCur ? Color{ 241, 196, 15, 255 } : Color{ 55, 65, 75, 200 });

            std::string label = "[" + std::to_string(i + 6) + "] " + GetWallStyleName(wst);
            DrawText(label.c_str(), wX + 15, itemY + 11, 12, isCur ? Color{ 255, 230, 100, 255 } : RAYWHITE);

            if (isCur) {
                DrawText("AKTIF", wX + halfW - 40, itemY + 11, 11, Color{ 241, 196, 15, 255 });
            }
        }

        // Quick Controls
        int tipY = modalY + 440;
        DrawText("Tekan [1-5] untuk Ganti Lantai Langsung   |   Tekan [6-0] untuk Ganti Dinding Langsung",
                 modalX + 45, tipY, 12, Color{ 255, 220, 120, 255 });
    }
    // ==========================================
    // TAB 3: PENCAHAYAAN (Lighting Customization)
    // ==========================================
    else if (currentTab == 3) {
        int contentY = tabY + 40;

        DrawText("TEMPERATUR & WARNA CAHAYA LAMPU PLAFON (LIGHTING TONE):", modalX + 30, contentY, 13, Color{ 255, 220, 100, 255 });

        LightTone tones[4] = {
            LightTone::WARM_WHITE, LightTone::NATURAL_DAYLIGHT, LightTone::COOL_SUPERMARKET, LightTone::CYBER_NEON
        };

        int tCardW = (modalW - 90) / 4;
        int tCardH = 150;
        int tGridY = contentY + 24;

        for (int i = 0; i < 4; ++i) {
            LightTone lt = tones[i];
            bool isCur = (currentLightTone == lt);
            int cX = modalX + 30 + i * (tCardW + 10);

            DrawRectangle(cX, tGridY, tCardW, tCardH, isCur ? Color{ 35, 55, 80, 240 } : Color{ 24, 30, 42, 200 });
            DrawRectangleLines(cX, tGridY, tCardW, tCardH, isCur ? Color{ 255, 215, 0, 255 } : Color{ 55, 65, 75, 200 });

            // Bulb Icon Box
            Color bulbCol = (lt == LightTone::WARM_WHITE) ? Color{ 255, 235, 180, 255 } :
                            (lt == LightTone::NATURAL_DAYLIGHT) ? Color{ 240, 248, 255, 255 } :
                            (lt == LightTone::COOL_SUPERMARKET) ? Color{ 225, 240, 255, 255 } : Color{ 100, 240, 255, 255 };

            DrawRectangle(cX + (tCardW - 36) / 2, tGridY + 15, 36, 36, bulbCol);
            DrawRectangleLines(cX + (tCardW - 36) / 2, tGridY + 15, 36, 36, RAYWHITE);

            std::string numTag = "[" + std::to_string(i + 1) + "]";
            DrawText(numTag.c_str(), cX + 10, tGridY + 10, 11, Color{ 200, 220, 240, 255 });

            DrawText(GetLightToneName(lt).c_str(), cX + 8, tGridY + 62, 10, isCur ? Color{ 255, 230, 100, 255 } : RAYWHITE);

            if (isCur) {
                DrawRectangle(cX + 12, tGridY + 115, tCardW - 24, 24, Color{ 46, 204, 113, 255 });
                DrawText("TERPASANG", cX + 22, tGridY + 121, 10, Color{ 20, 30, 25, 255 });
            } else {
                DrawText("Klik / [1-4] Pilih", cX + 15, tGridY + 121, 10, Color{ 150, 170, 190, 255 });
            }
        }

        // Intensity Slider Box
        int sliderY = tGridY + tCardH + 30;
        DrawRectangle(modalX + 30, sliderY, modalW - 60, 80, Color{ 22, 28, 38, 240 });
        DrawRectangleLines(modalX + 30, sliderY, modalW - 60, 80, Color{ 60, 75, 90, 255 });

        DrawText("INTENSITAS KECERAHAN LAMPU:", modalX + 45, sliderY + 12, 13, RAYWHITE);

        int barW = modalW - 280;
        int barH = 14;
        int barX = modalX + 45;
        int barY = sliderY + 40;

        DrawRectangle(barX, barY, barW, barH, Color{ 40, 48, 60, 255 });
        float fillRatio = (lightIntensity - 0.6f) / 0.8f;
        DrawRectangle(barX, barY, (int)(barW * fillRatio), barH, Color{ 255, 215, 0, 255 });
        DrawRectangleLines(barX, barY, barW, barH, RAYWHITE);

        std::string intLabel = TextFormat("%.0f%%", lightIntensity * 100.0f);
        DrawText(intLabel.c_str(), barX + barW + 20, barY - 2, 15, Color{ 255, 215, 0, 255 });

        DrawText("Gunakan Tombol [ < A / D > ] untuk Mengatur Kecerahan (+-10%)", modalX + 45, sliderY + 60, 11, Color{ 180, 195, 210, 255 });
    }
    // ==========================================
    // TAB 4: PAPAN NAMA TOKO (Signboard Customization)
    // ==========================================
    else if (currentTab == 4) {
        int contentY = tabY + 40;

        DrawText("GAYA PAPAN NAMA FASAD DEPAN (STOREFRONT SIGNBOARD):", modalX + 30, contentY, 13, Color{ 0, 200, 255, 255 });

        SignboardStyle signs[4] = {
            SignboardStyle::MINIMALIST_WOOD, SignboardStyle::NEON_GLOW,
            SignboardStyle::MODERN_LED, SignboardStyle::VINTAGE_RETRO
        };

        int sCardW = (modalW - 90) / 4;
        int sCardH = 135;
        int sGridY = contentY + 24;

        for (int i = 0; i < 4; ++i) {
            SignboardStyle ss = signs[i];
            bool isCur = (currentSignboard == ss);
            int cX = modalX + 30 + i * (sCardW + 10);

            DrawRectangle(cX, sGridY, sCardW, sCardH, isCur ? Color{ 35, 55, 80, 240 } : Color{ 24, 30, 42, 200 });
            DrawRectangleLines(cX, sGridY, sCardW, sCardH, isCur ? Color{ 0, 200, 255, 255 } : Color{ 55, 65, 75, 200 });

            std::string numTag = "[" + std::to_string(i + 1) + "]";
            DrawText(numTag.c_str(), cX + 10, sGridY + 10, 11, Color{ 200, 220, 240, 255 });

            DrawText(GetSignboardStyleName(ss).c_str(), cX + 8, sGridY + 38, 10, isCur ? Color{ 255, 230, 100, 255 } : RAYWHITE);

            if (isCur) {
                DrawRectangle(cX + 10, sGridY + 95, sCardW - 20, 24, Color{ 46, 204, 113, 255 });
                DrawText("TERPASANG", cX + 18, sGridY + 101, 10, Color{ 20, 30, 25, 255 });
            } else {
                std::string btnStr = "Tekan [" + std::to_string(i + 1) + "] Pasang";
                DrawText(btnStr.c_str(), cX + 10, sGridY + 101, 10, Color{ 150, 170, 190, 255 });
            }
        }

        // Store Name Display Box
        int nameBoxY = sGridY + sCardH + 30;
        DrawRectangle(modalX + 30, nameBoxY, modalW - 60, 90, Color{ 22, 28, 38, 240 });
        DrawRectangleLines(modalX + 30, nameBoxY, modalW - 60, 90, Color{ 60, 75, 90, 255 });

        DrawText("NAMA TOKO ANDA DI PAPAN NAMA:", modalX + 45, nameBoxY + 12, 13, RAYWHITE);
        std::string nStr = "\"" + shopName + "\"";
        DrawText(nStr.c_str(), modalX + 45, nameBoxY + 38, 22, Color{ 255, 215, 0, 255 });
        DrawText("Papan nama otomatis dirender di fasad depan atas pintu masuk toko dalam tampilan 3D.", modalX + 45, nameBoxY + 68, 11, Color{ 170, 190, 210, 255 });
    }

    // Footer Navigation Controls
    int footY = modalY + modalH - 32;
    DrawText("[Q / E / TAB] Ganti Tab    [1 - 9] Pilih Cepat    [ESC / C] Tutup Menu Kustomisasi",
             modalX + 35, footY, 12, Color{ 255, 220, 120, 255 });
}

ShopCustomization::CustomizationSaveData ShopCustomization::ExportSaveData() const {
    CustomizationSaveData save;
    save.floorStyle = static_cast<int>(currentFloor);
    save.wallStyle = static_cast<int>(currentWall);
    save.lightTone = static_cast<int>(currentLightTone);
    save.lightIntensity = lightIntensity;
    save.signboardStyle = static_cast<int>(currentSignboard);
    save.shopName = shopName;

    for (const auto& d : decorations) {
        save.decorEntries.push_back({
            d.id, d.typeId, d.isOwned, d.isPlaced,
            d.position.x, d.position.y, d.position.z, d.rotationY
        });
    }

    return save;
}

void ShopCustomization::ImportSaveData(const CustomizationSaveData& save) {
    currentFloor = static_cast<FloorStyle>(save.floorStyle);
    currentWall = static_cast<WallStyle>(save.wallStyle);
    currentLightTone = static_cast<LightTone>(save.lightTone);
    lightIntensity = std::clamp(save.lightIntensity, 0.6f, 1.4f);
    currentSignboard = static_cast<SignboardStyle>(save.signboardStyle);
    if (!save.shopName.empty()) {
        shopName = save.shopName;
    }

    for (const auto& dEntry : save.decorEntries) {
        for (auto& d : decorations) {
            if (d.id == dEntry.id) {
                d.isOwned = dEntry.isOwned;
                d.isPlaced = dEntry.isPlaced;
                d.position = { dEntry.posX, dEntry.posY, dEntry.posZ };
                d.rotationY = dEntry.rotY;
                break;
            }
        }
    }
}
