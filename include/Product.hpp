#pragma once
#include <string>
#include "raylib.h"

enum class ProductType {
    NONE = 0,
    BEVERAGE,   // Minuman (Jual: Rp5.000, Supplier: Rp3.000)
    BREAD,      // Roti (Jual: Rp8.000, Supplier: Rp5.000)
    CANNED_FOOD // Makanan Kaleng (Jual: Rp12.000, Supplier: Rp8.000)
};

struct ProductInfo {
    ProductType type;
    std::string name;
    int sellPrice;           // Harga jual ke customer
    int buyPrice;            // Harga beli dari supplier
    Color primaryColor;
    Color secondaryColor;
    Vector3 modelDimensions; // Ukuran visual item
};

inline ProductInfo GetProductInfo(ProductType type) {
    switch (type) {
        case ProductType::BEVERAGE:
            return { ProductType::BEVERAGE, "Minuman", 5000, 3000, { 30, 144, 255, 255 }, { 240, 248, 255, 255 }, { 0.22f, 0.45f, 0.22f } };
        case ProductType::BREAD:
            return { ProductType::BREAD, "Roti", 8000, 5000, { 210, 150, 75, 255 }, { 245, 222, 179, 255 }, { 0.40f, 0.25f, 0.25f } };
        case ProductType::CANNED_FOOD:
            return { ProductType::CANNED_FOOD, "Makanan Kaleng", 12000, 8000, { 220, 50, 50, 255 }, { 200, 200, 210, 255 }, { 0.28f, 0.35f, 0.28f } };
        default:
            return { ProductType::NONE, "Tidak ada", 0, 0, BLANK, BLANK, { 0.0f, 0.0f, 0.0f } };
    }
}
