#pragma once
#include <string>
#include <vector>
#include "raylib.h"

// Tahap 15: Product Categories
enum class ProductCategory {
    DRINK = 0,    // Minuman
    FOOD,         // Makanan pokok / instan
    SNACK,        // Camilan / biskuit
    HOUSEHOLD     // Kebutuhan rumah tangga
};

// Tahap 15: Scalable Product Types
enum class ProductType {
    NONE = 0,
    BEVERAGE,       // DRK-001: Air Mineral / Soda
    TEA_BOTTLE,     // DRK-002: Teh Botol
    BREAD,          // FOD-001: Roti Tawar
    INSTANT_NOODLE, // FOD-002: Mie Instan
    CANNED_FOOD,    // FOD-003: Makanan Kaleng / Kornet
    SNACK_BISCUIT,  // SNK-001: Biskuit Cokelat
    SOAP_BAR,       // HOU-001: Sabun Mandi Batang
    TISSUE_PACK     // HOU-002: Tisu Wajah
};

struct ProductInfo {
    int id;                  // Unique numeric Product ID
    ProductType type;        // Product enum
    std::string sku;         // Unique SKU string (e.g. "DRK-001")
    std::string name;        // Display name
    ProductCategory category;// Product Category
    int sellPrice;           // Standard baseline selling price
    int buyPrice;            // Supplier procurement cost
    Color primaryColor;      // Main packaging / model color
    Color secondaryColor;    // Accent / lid / label color
    Vector3 modelDimensions; // 3D bounding dimensions for rendering
};

inline std::string GetCategoryName(ProductCategory cat) {
    switch (cat) {
        case ProductCategory::DRINK: return "Minuman (Drink)";
        case ProductCategory::FOOD: return "Makanan (Food)";
        case ProductCategory::SNACK: return "Camilan (Snack)";
        case ProductCategory::HOUSEHOLD: return "Kebutuhan Rumah (Household)";
        default: return "Lainnya";
    }
}

inline ProductInfo GetProductInfo(ProductType type) {
    switch (type) {
        case ProductType::BEVERAGE:
            return { 1, ProductType::BEVERAGE, "DRK-001", "Air Mineral", ProductCategory::DRINK, 5000, 3000,
                     Color{ 30, 144, 255, 255 }, Color{ 240, 248, 255, 255 }, Vector3{ 0.22f, 0.45f, 0.22f } };
        case ProductType::TEA_BOTTLE:
            return { 2, ProductType::TEA_BOTTLE, "DRK-002", "Teh Botol", ProductCategory::DRINK, 6000, 3500,
                     Color{ 180, 80, 20, 255 }, Color{ 240, 190, 80, 255 }, Vector3{ 0.22f, 0.45f, 0.22f } };
        case ProductType::BREAD:
            return { 3, ProductType::BREAD, "FOD-001", "Roti Tawar", ProductCategory::FOOD, 8000, 5000,
                     Color{ 210, 150, 75, 255 }, Color{ 245, 222, 179, 255 }, Vector3{ 0.40f, 0.25f, 0.25f } };
        case ProductType::INSTANT_NOODLE:
            return { 4, ProductType::INSTANT_NOODLE, "FOD-002", "Mie Instan", ProductCategory::FOOD, 4000, 2500,
                     Color{ 240, 180, 0, 255 }, Color{ 200, 30, 30, 255 }, Vector3{ 0.32f, 0.18f, 0.28f } };
        case ProductType::CANNED_FOOD:
            return { 5, ProductType::CANNED_FOOD, "FOD-003", "Makanan Kaleng", ProductCategory::FOOD, 12000, 8000,
                     Color{ 220, 50, 50, 255 }, Color{ 200, 200, 210, 255 }, Vector3{ 0.28f, 0.35f, 0.28f } };
        case ProductType::SNACK_BISCUIT:
            return { 6, ProductType::SNACK_BISCUIT, "SNK-001", "Biskuit Cokelat", ProductCategory::SNACK, 7000, 4500,
                     Color{ 110, 60, 30, 255 }, Color{ 255, 215, 0, 255 }, Vector3{ 0.35f, 0.22f, 0.22f } };
        case ProductType::SOAP_BAR:
            return { 7, ProductType::SOAP_BAR, "HOU-001", "Sabun Mandi", ProductCategory::HOUSEHOLD, 5000, 3000,
                     Color{ 50, 200, 150, 255 }, Color{ 240, 255, 250, 255 }, Vector3{ 0.26f, 0.15f, 0.32f } };
        case ProductType::TISSUE_PACK:
            return { 8, ProductType::TISSUE_PACK, "HOU-002", "Tisu Wajah", ProductCategory::HOUSEHOLD, 10000, 6500,
                     Color{ 230, 230, 240, 255 }, Color{ 100, 180, 240, 255 }, Vector3{ 0.38f, 0.20f, 0.30f } };
        default:
            return { 0, ProductType::NONE, "NONE-000", "Tidak ada", ProductCategory::FOOD, 0, 0,
                     BLANK, BLANK, Vector3{ 0.0f, 0.0f, 0.0f } };
    }
}

inline const std::vector<ProductType>& GetAllProductTypes() {
    static const std::vector<ProductType> allTypes = {
        ProductType::BEVERAGE,
        ProductType::TEA_BOTTLE,
        ProductType::BREAD,
        ProductType::INSTANT_NOODLE,
        ProductType::CANNED_FOOD,
        ProductType::SNACK_BISCUIT,
        ProductType::SOAP_BAR,
        ProductType::TISSUE_PACK
    };
    return allTypes;
}

inline ProductType GetProductTypeFromSKU(const std::string& sku) {
    for (auto type : GetAllProductTypes()) {
        if (GetProductInfo(type).sku == sku) {
            return type;
        }
    }
    return ProductType::NONE;
}
