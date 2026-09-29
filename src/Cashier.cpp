#include "Cashier.hpp"
#include <cmath>

Cashier::Cashier()
    : position{ 5.5f, 0.6f, 8.5f },
      size{ 3.5f, 1.2f, 1.6f },
      counterColor{ 70, 75, 80, 255 },
      counterTopColor{ 90, 95, 100, 255 },
      registerColor{ 35, 40, 45, 255 },
      registerScreenColor{ 50, 205, 50, 255 },
      npcSkinColor{ 255, 220, 185, 255 },
      npcUniformColor{ 24, 44, 97, 255 },  // Official Navy Blue Store Uniform
      npcCapColor{ 20, 30, 60, 255 },      // Navy Store Visor/Cap
      npcApronColor{ 210, 45, 45, 255 },   // Red store apron
      npcIdleTimer(0.0f),
      currentQueueCount(0)
{
}

Cashier::Cashier(Vector3 position, Vector3 size)
    : position(position),
      size(size),
      counterColor{ 70, 75, 80, 255 },
      counterTopColor{ 90, 95, 100, 255 },
      registerColor{ 35, 40, 45, 255 },
      registerScreenColor{ 50, 205, 50, 255 },
      npcSkinColor{ 255, 220, 185, 255 },
      npcUniformColor{ 24, 44, 97, 255 },
      npcCapColor{ 20, 30, 60, 255 },
      npcApronColor{ 210, 45, 45, 255 },
      npcIdleTimer(0.0f),
      currentQueueCount(0)
{
}

void Cashier::Init() {
    currentQueueCount = 0;
    npcIdleTimer = 0.0f;
}

void Cashier::Update(float deltaTime) {
    npcIdleTimer += deltaTime * 2.5f;
}

Vector3 Cashier::GetNpcPosition() const {
    // Counter center: X = 5.5, Z = 8.5. Counter bounds in Z: 7.7 to 9.3
    // Cash register is on the counter at X = 4.9, Z = 8.5
    // NPC stands behind the counter at X = 4.9m, Z = 9.95m (clearly outside counter, facing North towards customer)
    return { position.x - 0.6f, 0.0f, position.z + size.z / 2.0f + 0.65f };
}

AABB Cashier::GetCollider() const {
    // Combined collider for counter table and cashier booth area
    AABB box;
    box.min = { position.x - size.x / 2.0f, 0.0f, position.z - size.z / 2.0f };
    box.max = { position.x + size.x / 2.0f, position.y + size.y / 2.0f, position.z + size.z / 2.0f + 1.2f };
    return box;
}

Vector3 Cashier::GetQueuePosition(int queueIndex) const {
    // Customers stand in front of the counter facing the cash register (X = 4.9, Z = 8.5)
    // Slot 0 (paying): in front of counter at X = 4.9m, Z = 6.85m facing +Z
    // Slot 1, 2: queue back towards Z = 5.25m, 3.65m (open right aisle)
    float registerX = position.x - 0.6f;
    float frontZ = position.z - size.z / 2.0f - 0.85f;
    float queueSpacing = 1.6f;

    return { registerX, 0.0f, frontZ - queueIndex * queueSpacing };
}

bool Cashier::ProcessPayment(int customerId, const std::string& customerName, ProductType product, int& outAmount) {
    if (product == ProductType::NONE) {
        outAmount = 0;
        return false;
    }

    ProductInfo info = GetProductInfo(product);
    outAmount = info.sellPrice;
    return true;
}

void Cashier::RenderCashierNpc() {
    Vector3 npcPos = GetNpcPosition();
    
    // Subtle breathing/idle animation
    float breathOffset = sinf(npcIdleTimer) * 0.015f;

    // 1. Legs (Pants) - Facing -Z (towards front of store / customer)
    float legHeight = 0.65f;
    Vector3 leftLegPos = { npcPos.x - 0.14f, legHeight / 2.0f, npcPos.z };
    Vector3 rightLegPos = { npcPos.x + 0.14f, legHeight / 2.0f, npcPos.z };

    DrawCube(leftLegPos, 0.18f, legHeight, 0.22f, { 30, 35, 45, 255 });
    DrawCubeWires(leftLegPos, 0.18f, legHeight, 0.22f, { 15, 20, 25, 255 });

    DrawCube(rightLegPos, 0.18f, legHeight, 0.22f, { 30, 35, 45, 255 });
    DrawCubeWires(rightLegPos, 0.18f, legHeight, 0.22f, { 15, 20, 25, 255 });

    // 2. Torso (Store Uniform Shirt + Red Apron)
    float bodyHeight = 0.75f;
    Vector3 torsoPos = { npcPos.x, legHeight + bodyHeight / 2.0f + breathOffset, npcPos.z };
    DrawCube(torsoPos, 0.55f, bodyHeight, 0.35f, npcUniformColor);
    DrawCubeWires(torsoPos, 0.55f, bodyHeight, 0.35f, { 15, 25, 45, 255 });

    // Front Apron overlay (facing -Z towards counter)
    Vector3 apronPos = { npcPos.x, torsoPos.y - 0.05f, npcPos.z - 0.10f };
    DrawCube(apronPos, 0.46f, bodyHeight * 0.85f, 0.18f, npcApronColor);

    // Arms resting forward near counter
    Vector3 leftArmPos = { npcPos.x - 0.22f, torsoPos.y - 0.08f, npcPos.z - 0.20f };
    Vector3 rightArmPos = { npcPos.x + 0.22f, torsoPos.y - 0.08f, npcPos.z - 0.20f };
    DrawCube(leftArmPos, 0.14f, 0.14f, 0.26f, npcUniformColor);
    DrawCube(rightArmPos, 0.14f, 0.14f, 0.26f, npcUniformColor);

    // 3. Head
    Vector3 headPos = { npcPos.x, legHeight + bodyHeight + 0.22f + breathOffset, npcPos.z };
    DrawSphere(headPos, 0.22f, npcSkinColor);
    DrawSphereWires(headPos, 0.22f, 8, 8, { 180, 140, 120, 255 });

    // 4. Store Uniform Cap / Visor (Visor brim extends towards -Z)
    Vector3 capPos = { npcPos.x, headPos.y + 0.14f, npcPos.z };
    DrawCube(capPos, 0.40f, 0.12f, 0.40f, npcCapColor);
    Vector3 visorBrimPos = { npcPos.x, headPos.y + 0.10f, npcPos.z - 0.22f };
    DrawCube(visorBrimPos, 0.34f, 0.03f, 0.16f, npcCapColor);

    // 5. Overhead Role Indicator Tag ("KASIR" badge marker)
    Vector3 roleBadgePos = { npcPos.x, headPos.y + 0.55f, npcPos.z };
    DrawCube(roleBadgePos, 0.15f, 0.15f, 0.15f, { 0, 191, 255, 255 });
    DrawCubeWires(roleBadgePos, 0.15f, 0.15f, 0.15f, RAYWHITE);
}

void Cashier::Render() {
    // 1. Counter Table Body
    DrawCube(position, size.x, size.y, size.z, counterColor);
    DrawCubeWires(position, size.x, size.y, size.z, { 30, 35, 40, 255 });

    // 2. Counter Countertop
    Vector3 topPos = { position.x, position.y + size.y / 2.0f - 0.04f, position.z };
    DrawCube(topPos, size.x + 0.1f, 0.08f, size.z + 0.1f, counterTopColor);

    // 3. Cash Register / POS Machine (on top of counter)
    Vector3 registerPos = { position.x - 0.6f, position.y + size.y / 2.0f + 0.15f, position.z };
    DrawCube(registerPos, 0.65f, 0.25f, 0.65f, registerColor);
    DrawCubeWires(registerPos, 0.65f, 0.25f, 0.65f, { 20, 20, 25, 255 });

    // 4. POS Display Screen (Screen faces -Z towards customer)
    Vector3 screenPos = { registerPos.x, registerPos.y + 0.22f, registerPos.z - 0.12f };
    DrawCube(screenPos, 0.35f, 0.22f, 0.15f, { 20, 20, 20, 255 });
    
    Vector3 screenGlassPos = { screenPos.x, screenPos.y, screenPos.z - 0.08f };
    DrawCube(screenGlassPos, 0.30f, 0.18f, 0.02f, registerScreenColor);

    // 5. Overhead "KASIR / CASHIER" Sign
    Vector3 signPos = { position.x, position.y + size.y + 1.2f, position.z };
    DrawCube(signPos, 2.2f, 0.45f, 0.12f, { 30, 144, 255, 255 });
    DrawCubeWires(signPos, 2.2f, 0.45f, 0.12f, RAYWHITE);

    // 6. Dedicated Cashier NPC (Standing cleanly behind counter)
    RenderCashierNpc();
}
