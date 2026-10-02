#pragma once
#include "Product.hpp"
#include "Storage.hpp"
#include <vector>
#include <string>

enum class OrderStatus {
    ORDERED,    // Order placed, waiting for delivery dispatch
    DELIVERING, // In transit
    ARRIVED     // Delivered to storage
};

struct SupplierOrder {
    int orderId;
    ProductType productType;
    int quantity;
    int totalCost;
    float deliveryTimer; // Seconds remaining until arrival
    OrderStatus status;
};

class Finance; // Forward declaration

class Supplier {
public:
    Supplier();
    ~Supplier() = default;

    void Init();
    void Update(float deltaTime, Storage& storage, std::vector<std::string>& outDeliveredNotices);

    // Order operations with single-source Finance system & dynamic market supplier price
    bool PlaceOrder(ProductType type, int quantity, Finance& finance, std::string& outErrorMessage, int customUnitCost = -1, float deliveryTimeMult = 1.0f);

    // Active Orders & Delivery Queries
    const std::vector<SupplierOrder>& GetActiveOrders() const { return activeOrders; }
    bool HasActiveOrders() const { return !activeOrders.empty(); }
    void LoadActiveOrders(const std::vector<SupplierOrder>& orders) {
        activeOrders = orders;
    }

    // UI Menu State
    bool IsMenuOpen() const { return menuOpen; }
    void ToggleMenu() { menuOpen = !menuOpen; }
    void SetMenuOpen(bool open) { menuOpen = open; }

    // Selected item in menu
    int GetSelectedProductIndex() const { return selectedProductIndex; }
    void NextProduct();
    void PreviousProduct();

    int GetOrderQuantity() const { return orderQuantity; }
    void IncreaseQuantity(int step = 5);
    void DecreaseQuantity(int step = 5);

    ProductType GetSelectedProductType() const;

private:
    bool menuOpen;
    int selectedProductIndex;
    int orderQuantity;
    int orderIdCounter;

    std::vector<SupplierOrder> activeOrders;
    std::vector<ProductType> availableProducts;
};
