#include "Customer.hpp"
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
      targetRackId(0),
      waitTimer(0.0f),
      maxWaitDuration(4.0f),
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
      targetRackId(0),
      waitTimer(0.0f),
      maxWaitDuration(3.5f + (id % 3) * 1.0f), // 3.5s to 5.5s browsing duration
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
        case CustomerState::WALKING_TO_SHELF: return "Menuju Rak";
        case CustomerState::AT_SHELF: return "Melihat Produk";
        case CustomerState::LEAVING: return "Menuju Pintu Keluar";
        case CustomerState::EXITING: return "Keluar dari Toko";
        case CustomerState::DESPAWNED: return "Selesai (Despawn)";
        default: return "Idle";
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

void Customer::SetTargetRack(Vector3 rackInteractionPos, int rackId) {
    targetRackPos = rackInteractionPos;
    targetRackId = rackId;
}

void Customer::BuildExitWaypoints() {
    waypoints.clear();
    currentWaypointIndex = 0;

    // Waypoint 1: Return to central main aisle walkway
    waypoints.push_back({ 0.0f, 0.0f, targetRackPos.z });
    // Waypoint 2: Inside shop lobby near door
    waypoints.push_back({ 0.0f, 0.0f, 8.5f });
    // Waypoint 3: Step out through the door
    waypoints.push_back({ 0.0f, 0.0f, 11.5f });
    // Waypoint 4: Outside door
    waypoints.push_back({ 0.0f, 0.0f, 13.0f });
    // Waypoint 5: Outside despawn area
    waypoints.push_back({ position.x > 0 ? 8.0f : -8.0f, 0.0f, 18.0f });
}

void Customer::MoveTowards(Vector3 target, float deltaTime) {
    Vector3 toTarget = Vector3Subtract(target, position);
    toTarget.y = 0.0f; // Keep on flat ground

    float dist = Vector3Length(toTarget);
    if (dist > 0.05f) {
        Vector3 dir = Vector3Normalize(toTarget);
        position = Vector3Add(position, Vector3Scale(dir, moveSpeed * deltaTime));
        rotationY = atan2f(-dir.x, -dir.z); // Face movement direction
        bobbingTimer += deltaTime * 8.0f;
    }
}

void Customer::Update(float deltaTime) {
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
            // Reached inside lobby -> Transition to WALKING_TO_SHELF
            state = CustomerState::WALKING_TO_SHELF;
            
            // Build path to targeted rack
            waypoints.clear();
            currentWaypointIndex = 0;
            // First move along Z-axis of aisle to align with rack
            waypoints.push_back({ 0.0f, 0.0f, targetRackPos.z });
            // Then move to the rack interaction front spot
            waypoints.push_back(targetRackPos);
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
            // Reached the shelf!
            state = CustomerState::AT_SHELF;
            waitTimer = 0.0f;
        }
    }
    else if (state == CustomerState::AT_SHELF) {
        waitTimer += deltaTime;
        // Face slightly towards rack center
        // When wait duration is exceeded, begin LEAVING
        if (waitTimer >= maxWaitDuration) {
            state = CustomerState::LEAVING;
            BuildExitWaypoints();
        }
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

    // 5. Overhead Name & Status Tag
    Vector3 tagPos = { basePos.x, headPos.y + 0.55f, basePos.z };
    // Small colored indicator cube above head
    Color statusMarkerColor = (state == CustomerState::AT_SHELF) ? Color{ 255, 215, 0, 255 } : Color{ 50, 205, 50, 255 };
    DrawCube(tagPos, 0.12f, 0.12f, 0.12f, statusMarkerColor);
}
