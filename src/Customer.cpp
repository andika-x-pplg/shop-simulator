#include "Customer.hpp"
#include "Shop.hpp"
#include <cmath>

Customer::Customer()
    : id(0),
      name("Customer"),
      position{ 0.0f, 0.0f, 16.0f },
      velocity{ 0.0f, 0.0f, 0.0f },
      rotationY(0.0f),
      moveSpeed(2.8f),
      state(CustomerState::ENTERING),
      currentWaypointIndex(0),
      targetRackPos{ 0.0f, 0.0f, 0.0f },
      targetRackId(-1),
      targetProductType(ProductType::NONE),
      waitTimer(0.0f),
      maxWaitDuration(3.0f),
      heldProduct(ProductType::NONE),
      bodyColor{ 255, 218, 185, 255 },
      shirtColor{ 70, 130, 180, 255 },
      pantsColor{ 45, 52, 54, 255 },
      bobbingTimer(0.0f)
{
}

Customer::Customer(int id, const std::string& name, Vector3 spawnPos, Color bodyColor, Color shirtColor)
    : id(id),
      name(name),
      position(spawnPos),
      velocity{ 0.0f, 0.0f, 0.0f },
      rotationY(PI), // Face north towards shop door
      moveSpeed(2.8f),
      state(CustomerState::ENTERING),
      currentWaypointIndex(0),
      targetRackPos{ 0.0f, 0.0f, 0.0f },
      targetRackId(-1),
      targetProductType(ProductType::NONE),
      waitTimer(0.0f),
      maxWaitDuration(2.5f + (id % 3) * 0.8f), // 2.5s to 4.1s browsing duration
      heldProduct(ProductType::NONE),
      bodyColor(bodyColor),
      shirtColor(shirtColor),
      pantsColor{ 50, 55, 60, 255 },
      bobbingTimer(0.0f)
{
    BuildEntryWaypoints();
}

std::string Customer::GetStateString() const {
    switch (state) {
        case CustomerState::ENTERING: return "Masuk ke Toko";
        case CustomerState::SELECTING_PRODUCT: return "Mencari Produk";
        case CustomerState::WALKING_TO_SHELF: return "Menuju Rak " + GetProductInfo(targetProductType).name;
        case CustomerState::AT_SHELF: return "Melihat " + GetProductInfo(targetProductType).name;
        case CustomerState::TAKING_PRODUCT: return "Mengambil " + GetProductInfo(targetProductType).name;
        case CustomerState::LEAVING: return "Membawa " + GetHeldProductName() + " ke Pintu";
        case CustomerState::EXITING: return "Keluar dari Toko";
        case CustomerState::DESPAWNED: return "Selesai (Despawn)";
        default: return "Idle";
    }
}

std::string Customer::GetHeldProductName() const {
    return GetProductInfo(heldProduct).name;
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

void Customer::SetTargetRack(Vector3 rackInteractionPos, int rackId, ProductType type) {
    targetRackPos = rackInteractionPos;
    targetRackId = rackId;
    targetProductType = type;
}

bool Customer::SelectAvailableRack(Shop& shop) {
    int rackIdx = shop.FindAvailableRackIndex(id);
    if (rackIdx >= 0) {
        Vector3 rackFront = shop.GetRackFrontPosition((size_t)rackIdx);
        ProductType pType = shop.GetRacks()[(size_t)rackIdx].GetProductType();
        SetTargetRack(rackFront, rackIdx, pType);
        return true;
    }
    return false;
}

void Customer::BuildExitWaypoints() {
    waypoints.clear();
    currentWaypointIndex = 0;

    // Waypoint 1: Return to central main aisle walkway from current X position
    waypoints.push_back({ 0.0f, 0.0f, position.z });
    // Waypoint 2: Inside shop lobby near door
    waypoints.push_back({ 0.0f, 0.0f, 8.5f });
    // Waypoint 3: Step out through the door
    waypoints.push_back({ 0.0f, 0.0f, 11.5f });
    // Waypoint 4: Outside door
    waypoints.push_back({ 0.0f, 0.0f, 13.0f });
    // Waypoint 5: Outside despawn area (alternate left or right side)
    float exitSide = (id % 2 == 0) ? 8.0f : -8.0f;
    waypoints.push_back({ exitSide, 0.0f, 18.0f });
}

void Customer::MoveTowards(Vector3 target, float deltaTime) {
    Vector3 toTarget = Vector3Subtract(target, position);
    toTarget.y = 0.0f; // Keep on flat floor

    float dist = Vector3Length(toTarget);
    if (dist > 0.05f) {
        Vector3 dir = Vector3Normalize(toTarget);
        position = Vector3Add(position, Vector3Scale(dir, moveSpeed * deltaTime));
        rotationY = atan2f(-dir.x, -dir.z); // Face movement direction
        bobbingTimer += deltaTime * 8.0f;
    }
}

void Customer::Update(float deltaTime, Shop& shop) {
    if (state == CustomerState::DESPAWNED) return;

    if (state == CustomerState::ENTERING) {
        if (currentWaypointIndex < waypoints.size()) {
            Vector3 targetWp = waypoints[currentWaypointIndex];
            MoveTowards(targetWp, deltaTime);

            float dist = Vector3Distance(position, targetWp);
            if (dist < 0.35f) {
                currentWaypointIndex++;
            }
        } else {
            // Reached inside lobby -> Select a rack with available stock
            state = CustomerState::SELECTING_PRODUCT;
        }
    }
    else if (state == CustomerState::SELECTING_PRODUCT) {
        bool found = SelectAvailableRack(shop);
        if (found) {
            state = CustomerState::WALKING_TO_SHELF;
            // Build path to targeted rack through main walkway
            waypoints.clear();
            currentWaypointIndex = 0;
            // 1. Move along Z-axis of aisle to align with rack
            waypoints.push_back({ 0.0f, 0.0f, targetRackPos.z });
            // 2. Move directly to the rack front spot
            waypoints.push_back(targetRackPos);
        } else {
            // All racks empty! Exit shop without buying
            state = CustomerState::LEAVING;
            BuildExitWaypoints();
        }
    }
    else if (state == CustomerState::WALKING_TO_SHELF) {
        if (currentWaypointIndex < waypoints.size()) {
            Vector3 targetWp = waypoints[currentWaypointIndex];
            MoveTowards(targetWp, deltaTime);

            float dist = Vector3Distance(position, targetWp);
            if (dist < 0.35f) {
                currentWaypointIndex++;
            }
        } else {
            // Reached front of shelf!
            state = CustomerState::AT_SHELF;
            waitTimer = 0.0f;
        }
    }
    else if (state == CustomerState::AT_SHELF) {
        waitTimer += deltaTime;
        if (waitTimer >= maxWaitDuration) {
            state = CustomerState::TAKING_PRODUCT;
        }
    }
    else if (state == CustomerState::TAKING_PRODUCT) {
        // Double-check rack stock before taking
        auto& racks = shop.GetRacks();
        if (targetRackId >= 0 && (size_t)targetRackId < racks.size() && racks[(size_t)targetRackId].HasStock()) {
            if (racks[(size_t)targetRackId].TakeProduct()) {
                heldProduct = targetProductType;
            }
        } else {
            // If someone took the last stock while customer was walking, try selecting another rack
            bool foundAnother = SelectAvailableRack(shop);
            if (foundAnother) {
                state = CustomerState::WALKING_TO_SHELF;
                waypoints.clear();
                currentWaypointIndex = 0;
                waypoints.push_back({ 0.0f, 0.0f, targetRackPos.z });
                waypoints.push_back(targetRackPos);
                return;
            }
        }

        // Proceed to leave shop with or without item
        state = CustomerState::LEAVING;
        BuildExitWaypoints();
    }
    else if (state == CustomerState::LEAVING || state == CustomerState::EXITING) {
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
            // Finished exiting outside -> Despawn
            state = CustomerState::DESPAWNED;
        }
    }
}

void Customer::RenderHeldProduct() {
    if (heldProduct == ProductType::NONE) return;

    ProductInfo info = GetProductInfo(heldProduct);

    // Calculate customer forward and right vectors from rotationY
    Vector3 forward = { -sinf(rotationY), 0.0f, -cosf(rotationY) };
    Vector3 right = { cosf(rotationY), 0.0f, -sinf(rotationY) };

    // Item carried in right arm/front: 0.28m forward, 0.26m right, 0.85m height
    Vector3 itemPos = {
        position.x + forward.x * 0.28f + right.x * 0.26f,
        position.y + 0.85f,
        position.z + forward.z * 0.28f + right.z * 0.26f
    };

    if (heldProduct == ProductType::BEVERAGE) {
        DrawCube(itemPos, info.modelDimensions.x * 0.85f, info.modelDimensions.y * 0.85f, info.modelDimensions.z * 0.85f, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x * 0.85f, info.modelDimensions.y * 0.85f, info.modelDimensions.z * 0.85f, { 20, 80, 160, 255 });
        Vector3 capPos = { itemPos.x, itemPos.y + (info.modelDimensions.y * 0.85f) / 2.0f + 0.02f, itemPos.z };
        DrawCube(capPos, info.modelDimensions.x * 0.55f, 0.04f, info.modelDimensions.z * 0.55f, info.secondaryColor);
    }
    else if (heldProduct == ProductType::BREAD) {
        DrawCube(itemPos, info.modelDimensions.x * 0.85f, info.modelDimensions.y * 0.85f, info.modelDimensions.z * 0.85f, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x * 0.85f, info.modelDimensions.y * 0.85f, info.modelDimensions.z * 0.85f, { 140, 90, 40, 255 });
        Vector3 topSlit = { itemPos.x, itemPos.y + (info.modelDimensions.y * 0.85f) / 2.0f + 0.02f, itemPos.z };
        DrawCube(topSlit, info.modelDimensions.x * 0.65f, 0.03f, info.modelDimensions.z * 0.3f, info.secondaryColor);
    }
    else if (heldProduct == ProductType::CANNED_FOOD) {
        DrawCube(itemPos, info.modelDimensions.x * 0.85f, info.modelDimensions.y * 0.85f, info.modelDimensions.z * 0.85f, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x * 0.85f, info.modelDimensions.y * 0.85f, info.modelDimensions.z * 0.85f, { 150, 30, 30, 255 });
        Vector3 rimTop = { itemPos.x, itemPos.y + (info.modelDimensions.y * 0.85f) / 2.0f + 0.02f, itemPos.z };
        DrawCube(rimTop, info.modelDimensions.x * 0.75f, 0.03f, info.modelDimensions.z * 0.75f, info.secondaryColor);
    }
}

void Customer::Render() {
    if (state == CustomerState::DESPAWNED) return;

    // Walking animation bobbing
    float bobOffset = (state == CustomerState::AT_SHELF) ? 0.0f : sinf(bobbingTimer) * 0.04f;
    float legAngle = (state == CustomerState::AT_SHELF) ? 0.0f : sinf(bobbingTimer) * 0.15f;

    Vector3 basePos = { position.x, position.y + bobOffset, position.z };

    // 1. Legs (2 small cubes)
    float legHeight = 0.65f;
    Vector3 leftLegPos = { basePos.x - 0.14f, basePos.y + legHeight / 2.0f, basePos.z - legAngle };
    Vector3 rightLegPos = { basePos.x + 0.14f, basePos.y + legHeight / 2.0f, basePos.z + legAngle };

    DrawCube(leftLegPos, 0.18f, legHeight, 0.22f, pantsColor);
    DrawCubeWires(leftLegPos, 0.18f, legHeight, 0.22f, { 25, 25, 30, 255 });

    DrawCube(rightLegPos, 0.18f, legHeight, 0.22f, pantsColor);
    DrawCubeWires(rightLegPos, 0.18f, legHeight, 0.22f, { 25, 25, 30, 255 });

    // 2. Torso / Body (Cube with shirt color)
    float bodyHeight = 0.75f;
    Vector3 torsoPos = { basePos.x, basePos.y + legHeight + bodyHeight / 2.0f, basePos.z };
    DrawCube(torsoPos, 0.55f, bodyHeight, 0.35f, shirtColor);
    DrawCubeWires(torsoPos, 0.55f, bodyHeight, 0.35f, { 30, 30, 35, 255 });

    // 3. Head (Sphere with skin/body color)
    Vector3 headPos = { basePos.x, basePos.y + legHeight + bodyHeight + 0.22f, basePos.z };
    DrawSphere(headPos, 0.22f, bodyColor);
    DrawSphereWires(headPos, 0.22f, 8, 8, { 180, 140, 120, 255 });

    // 4. Hair / Hat accent on top
    Vector3 hairPos = { basePos.x, headPos.y + 0.14f, basePos.z };
    DrawCube(hairPos, 0.38f, 0.12f, 0.38f, { 60, 45, 35, 255 });

    // 5. Render Held Product (Visual item in customer's hand)
    RenderHeldProduct();

    // 6. Overhead Name & Status Tag
    Vector3 tagPos = { basePos.x, headPos.y + 0.55f, basePos.z };
    Color statusMarkerColor = (state == CustomerState::AT_SHELF) ? Color{ 255, 215, 0, 255 } :
                              (heldProduct != ProductType::NONE) ? Color{ 50, 205, 50, 255 } : Color{ 100, 180, 255, 255 };
    DrawCube(tagPos, 0.12f, 0.12f, 0.12f, statusMarkerColor);
}
