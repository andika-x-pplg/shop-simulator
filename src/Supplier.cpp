#include "Supplier.hpp"

Supplier::Supplier()
    : menuOpen(false),
      selectedProductIndex(0),
      orderQuantity(10),
      orderIdCounter(1)
{
    availableProducts = {
        ProductType::BEVERAGE,
        ProductType::BREAD,
        ProductType::CANNED_FOOD
    };
}

void Supplier::Init() {
    menuOpen = false;
    selectedProductIndex = 0;
    orderQuantity = 10;
    orderIdCounter = 1;
    activeOrders.clear();
}

ProductType Supplier::GetSelectedProductType() const {
    if (selectedProductIndex >= 0 && selectedProductIndex < (int)availableProducts.size()) {
        return availableProducts[selectedProductIndex];
    }
    return ProductType::NONE;
}

void Supplier::NextProduct() {
    selectedProductIndex = (selectedProductIndex + 1) % availableProducts.size();
}

void Supplier::PreviousProduct() {
    selectedProductIndex = (selectedProductIndex - 1 + availableProducts.size()) % availableProducts.size();
}

void Supplier::IncreaseQuantity(int step) {
    orderQuantity += step;
    if (orderQuantity > 50) orderQuantity = 50;
}

void Supplier::DecreaseQuantity(int step) {
    orderQuantity -= step;
    if (orderQuantity < 1) orderQuantity = 1;
}

bool Supplier::PlaceOrder(ProductType type, int quantity, int& currentShopMoney, std::string& outErrorMessage) {
    if (quantity <= 0) {
        outErrorMessage = "Jumlah pesanan harus lebih dari 0!";
        return false;
    }

    ProductInfo info = GetProductInfo(type);
    int totalCost = info.buyPrice * quantity;

    if (currentShopMoney < totalCost) {
        outErrorMessage = "Uang tidak cukup! Butuh Rp" + std::to_string(totalCost) + " (Saldo: Rp" + std::to_string(currentShopMoney) + ")";
        return false;
    }

    // Deduct money from shop treasury
    currentShopMoney -= totalCost;

    // Create delivery order (5 seconds delivery time)
    SupplierOrder order;
    order.orderId = orderIdCounter++;
    order.productType = type;
    order.quantity = quantity;
    order.totalCost = totalCost;
    order.deliveryTimer = 5.0f;
    order.status = OrderStatus::ORDERED;

    activeOrders.push_back(order);
    return true;
}

void Supplier::Update(float deltaTime, Storage& storage, std::vector<std::string>& outDeliveredNotices) {
    for (auto it = activeOrders.begin(); it != activeOrders.end();) {
        it->deliveryTimer -= deltaTime;

        if (it->deliveryTimer > 2.5f) {
            it->status = OrderStatus::ORDERED;
        } else if (it->deliveryTimer > 0.0f) {
            it->status = OrderStatus::DELIVERING;
        } else {
            // Delivery Arrived! Add stock to Storage
            it->status = OrderStatus::ARRIVED;
            storage.AddStock(it->productType, it->quantity);

            ProductInfo info = GetProductInfo(it->productType);
            std::string notice = "Pesanan " + info.name + " (" + std::to_string(it->quantity) + "x) telah tiba di Storage!";
            outDeliveredNotices.push_back(notice);

            it = activeOrders.erase(it);
            continue;
        }

        ++it;
    }
}
