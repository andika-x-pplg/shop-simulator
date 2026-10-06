#pragma once
#include "Common.hpp"
#include <string>
#include <vector>
#include "raylib.h"

// Forward declaration
class Finance;
class Shop;
class DailyStats;

// Floor Style Types
enum class FloorStyle {
    BASIC_TILE = 0,    // Standard Clean White Ceramic Tile (Default)
    WOOD_PARQUET,      // Warm Wooden Parquet / Vinyl Planks
    CHECKERED_RETRO,   // Classic Black & White Checkered Retro Tiles
    MARBLE_LUXURY,     // Polished Luxury Golden Vein Marble
    MODERN_CONCRETE    // Sleek Dark Polished Industrial Concrete
};

inline std::string GetFloorStyleName(FloorStyle style) {
    switch (style) {
        case FloorStyle::WOOD_PARQUET:    return "Kayu Parket Hangat (Wood Parquet)";
        case FloorStyle::CHECKERED_RETRO: return "Keramik Retro Catur (Checkered Tiles)";
        case FloorStyle::MARBLE_LUXURY:   return "Marmer Emas Mewah (Golden Marble)";
        case FloorStyle::MODERN_CONCRETE: return "Semen Beton Modern (Polished Concrete)";
        case FloorStyle::BASIC_TILE:
        default:                          return "Keramik Putih Bersih (Basic Tile)";
    }
}

// Wall Style Types
enum class WallStyle {
    BASIC_WHITE = 0,   // Standard Minimalist Soft Gray/White Wall
    PAINTED_PASTEL,    // Warm Cozy Pastel Cream & Sky Blue Accent
    BRICK_EXPOSED,     // Rustic Exposed Red/Brown Brick
    MODERN_PANEL,      // Elegant Dark Slate & Wooden Slat Accent Panels
    PREMIUM_WALLPAPER  // Luxury Gold Geometric Motif Wallpaper
};

inline std::string GetWallStyleName(WallStyle style) {
    switch (style) {
        case WallStyle::PAINTED_PASTEL:    return "Cat Pastel Hangat (Pastel Cream)";
        case WallStyle::BRICK_EXPOSED:     return "Bata Ekspos Klasik (Exposed Brick)";
        case WallStyle::MODERN_PANEL:      return "Panel Kayu & Slate (Modern Panel)";
        case WallStyle::PREMIUM_WALLPAPER: return "Wallpaper Geometris Emas (Luxury Gold)";
        case WallStyle::BASIC_WHITE:
        default:                           return "Dinding Putih Minimalis (Basic White)";
    }
}

// Lighting Tone Types
enum class LightTone {
    WARM_WHITE = 0,    // Cozy 3000K Warm Yellow White (Atmospheric & Inviting)
    NATURAL_DAYLIGHT,  // Crisp 4500K Neutral Pure Daylight (Bright & Clean)
    COOL_SUPERMARKET,  // High-luminance 6500K Cool White (Supermarket Commercial)
    CYBER_NEON         // Vibrant Cyan / Magenta Ambient Hue (Modern Hipster)
};

inline std::string GetLightToneName(LightTone tone) {
    switch (tone) {
        case LightTone::NATURAL_DAYLIGHT: return "Daylight Alami 4500K (Natural Pure)";
        case LightTone::COOL_SUPERMARKET: return "Cool Supermarket 6500K (Bright White)";
        case LightTone::CYBER_NEON:       return "Neon Modern Cyber (Vibrant Ambient)";
        case LightTone::WARM_WHITE:
        default:                          return "Kuning Hangat 3000K (Warm Cozy)";
    }
}

// Signboard Style Types
enum class SignboardStyle {
    MINIMALIST_WOOD = 0, // Wooden Board with Clean Lettering
    NEON_GLOW,           // Illuminated Glowing Neon Box Sign
    MODERN_LED,          // Backlit Metal 3D Signboard
    VINTAGE_RETRO        // Classic Retro Bistro Storefront Sign
};

inline std::string GetSignboardStyleName(SignboardStyle style) {
    switch (style) {
        case SignboardStyle::NEON_GLOW:     return "Neon Box Menyala (Glowing Neon)";
        case SignboardStyle::MODERN_LED:    return "Metal Backlit LED (Modern 3D)";
        case SignboardStyle::VINTAGE_RETRO: return "Papan Vintage Retro (Classic Bistro)";
        case SignboardStyle::MINIMALIST_WOOD:
        default:                            return "Papan Kayu Minimalis (Wooden Board)";
    }
}

// Decoration Object Item (Placed inside the shop)
struct DecorationItem {
    int id;
    std::string name;
    int typeId;             // 1: Indoor Plant, 2: Wall Clock, 3: Wall Poster, 4: Advertising Banner, 5: Ceiling Hanging Lamp, 6: Floor Mat
    int price;
    bool isOwned;
    bool isPlaced;
    Vector3 position;
    float rotationY;        // In degrees (0, 45, 90, 180, 270, etc.)
    Vector3 size;
    Color primaryColor;
    Color secondaryColor;
    std::string description;
};

// ==========================================
// STAGE 22: SHOP CUSTOMIZATION MANAGER
// ==========================================
class ShopCustomization {
public:
    static ShopCustomization& Instance();

    void Init();

    // ==========================================
    // Visual Customization Attributes
    // ==========================================
    FloorStyle GetFloorStyle() const { return currentFloor; }
    void SetFloorStyle(FloorStyle style) { currentFloor = style; }

    WallStyle GetWallStyle() const { return currentWall; }
    void SetWallStyle(WallStyle style) { currentWall = style; }

    LightTone GetLightTone() const { return currentLightTone; }
    void SetLightTone(LightTone tone) { currentLightTone = tone; }
    float GetLightIntensity() const { return lightIntensity; } // 0.6f - 1.4f
    void SetLightIntensity(float val);

    SignboardStyle GetSignboardStyle() const { return currentSignboard; }
    void SetSignboardStyle(SignboardStyle style) { currentSignboard = style; }
    std::string GetShopName() const { return shopName; }
    void SetShopName(const std::string& name) { shopName = name; }

    // Visual Palette Queries for 3D Shop Rendering
    Color GetFloorPrimaryColor() const;
    Color GetFloorGridColor() const;
    Color GetWallPrimaryColor() const;
    Color GetWallSecondaryColor() const;
    Color GetCeilingLightColor() const;

    // ==========================================
    // Free Furniture & Decoration Placement / Edit Mode
    // ==========================================
    bool IsEditingPlacement() const { return isEditingPlacement; }
    void StartMovingFurniture(int furnitureId, Vector3 curPos, float curRot, Vector3 size, const std::string& name);
    void StartMovingDecoration(int decorId, Vector3 curPos, float curRot, Vector3 size, const std::string& name);
    void CancelPlacement();
    bool ConfirmPlacement(Vector3& outPos, float& outRot, std::string& outFeedback);

    // Update Placement Cursor Position & Rotation during movement
    void UpdatePlacementMovement(Vector3 playerEyePos, Vector3 playerLookDir, float shopWidth, float shopLength);
    void RotateHeldObject(float deltaDegrees);
    void ToggleGridSnap() { gridSnapping = !gridSnapping; }
    bool IsGridSnapEnabled() const { return gridSnapping; }
    float GetGridSize() const { return gridSize; }
    void CycleGridSize();

    // Placement Validity Check
    bool IsPlacementValid() const { return isPlacementValid; }
    std::string GetPlacementValidationMessage() const { return validationMsg; }
    Vector3 GetHeldObjectPosition() const { return heldPosition; }
    float GetHeldObjectRotation() const { return heldRotationY; }
    Vector3 GetHeldObjectSize() const { return heldSize; }
    std::string GetHeldObjectName() const { return heldObjectName; }
    int GetHeldFurnitureId() const { return heldFurnitureId; }
    int GetHeldDecorId() const { return heldDecorId; }

    // Render 3D Placement Hologram / Ghost Preview
    void RenderPlacementPreview(int shopWidth, int shopLength);

    // ==========================================
    // Decoration Management
    // ==========================================
    const std::vector<DecorationItem>& GetDecorations() const { return decorations; }
    std::vector<DecorationItem>& GetDecorationsRef() { return decorations; }
    bool PurchaseDecoration(int decorIndex, Finance& finance, std::string& outFeedback);
    void RemoveDecoration(int decorId);

    // Render 3D Decorations
    void RenderDecorations();
    std::vector<AABB> GetDecorationColliders() const;

    // Render 3D Signboard on Store Front Façade
    void RenderSignboard(float shopWidth, float shopLength, float shopHeight);

    // ==========================================
    // UI Modal Management (Tombol C / Customization Menu)
    // ==========================================
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu();
    void SetMenuOpen(bool open);

    int GetCurrentTab() const { return currentTab; } // 0: Furniture Layout, 1: Decoration, 2: Floor & Wall, 3: Lighting, 4: Signboard
    void NextTab();
    void PreviousTab();
    void SetTab(int tab) { currentTab = (tab >= 0 && tab < 5) ? tab : 0; }

    int GetSelectedIndex() const { return selectedIndex; }
    void NextItem();
    void PreviousItem();

    // Render 2D Modal UI
    void RenderUI(int screenWidth, int screenHeight, int currentBalance);

    // ==========================================
    // Save / Load Serialization Support
    // ==========================================
    struct CustomizationSaveData {
        int floorStyle;
        int wallStyle;
        int lightTone;
        float lightIntensity;
        int signboardStyle;
        std::string shopName;

        struct FurnitureTransformSave {
            int id;
            float posX;
            float posY;
            float posZ;
            float rotY;
        };
        std::vector<FurnitureTransformSave> furnitureTransforms;

        struct DecorSaveEntry {
            int id;
            int typeId;
            bool isOwned;
            bool isPlaced;
            float posX;
            float posY;
            float posZ;
            float rotY;
        };
        std::vector<DecorSaveEntry> decorEntries;
    };

    CustomizationSaveData ExportSaveData() const;
    void ImportSaveData(const CustomizationSaveData& save);

private:
    ShopCustomization();
    ~ShopCustomization() = default;

    ShopCustomization(const ShopCustomization&) = delete;
    ShopCustomization& operator=(const ShopCustomization&) = delete;

    void ValidateCurrentPlacement(float shopWidth, float shopLength);
    void DrawSingleDecoration(const DecorationItem& item);

    // Visual Style State
    FloorStyle currentFloor;
    WallStyle currentWall;
    LightTone currentLightTone;
    float lightIntensity;
    SignboardStyle currentSignboard;
    std::string shopName;

    // Decoration Objects Catalog & Owned Items
    std::vector<DecorationItem> decorations;

    // Free Placement / Moving Object Session State
    bool isEditingPlacement;
    int heldFurnitureId;     // >0 if moving furniture item
    int heldDecorId;         // >0 if moving decoration item
    std::string heldObjectName;
    Vector3 heldPosition;
    float heldRotationY;
    Vector3 heldSize;
    Vector3 originalPosition;
    float originalRotationY;

    bool gridSnapping;
    float gridSize;          // 0.5f or 1.0f
    bool isPlacementValid;
    std::string validationMsg;

    // UI Menu State
    bool menuOpen;
    int currentTab;          // 0: Layout, 1: Decoration, 2: Floor/Wall, 3: Lighting, 4: Signboard
    int selectedIndex;
};
