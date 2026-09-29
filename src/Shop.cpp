#include "Shop.hpp"

Shop::Shop()
    : shopWidth(20.0f), shopLength(24.0f), shopHeight(5.0f)
{
}

void Shop::Init() {
    BuildStructure();
    BuildColliders();
}

void Shop::BuildStructure() {
    walls.clear();
    shelves.clear();

    float halfW = shopWidth / 2.0f;
    float halfL = shopLength / 2.0f;
    float wallThickness = 0.5f;

    Color wallColor = { 220, 225, 230, 255 };      // Soft light gray/white interior
    Color backWallColor = { 200, 210, 220, 255 };

    // North Wall (Back)
    walls.push_back({ { 0.0f, shopHeight / 2.0f, -halfL }, { shopWidth, shopHeight, wallThickness }, backWallColor });

    // West Wall (Left)
    walls.push_back({ { -halfW, shopHeight / 2.0f, 0.0f }, { wallThickness, shopHeight, shopLength }, wallColor });

    // East Wall (Right)
    walls.push_back({ { halfW, shopHeight / 2.0f, 0.0f }, { wallThickness, shopHeight, shopLength }, wallColor });

    // South Wall (Front) with Door opening in the center
    // Door opening is 4.0m wide (from -2.0 to 2.0)
    float sideWallWidth = (shopWidth - 4.0f) / 2.0f;
    float leftWallX = -halfW + sideWallWidth / 2.0f;
    float rightWallX = halfW - sideWallWidth / 2.0f;

    // South-Left Wall
    walls.push_back({ { leftWallX, shopHeight / 2.0f, halfL }, { sideWallWidth, shopHeight, wallThickness }, wallColor });
    // South-Right Wall
    walls.push_back({ { rightWallX, shopHeight / 2.0f, halfL }, { sideWallWidth, shopHeight, wallThickness }, wallColor });
    // Top Door Frame
    walls.push_back({ { 0.0f, shopHeight - 0.6f, halfL }, { 4.0f, 1.2f, wallThickness }, { 100, 110, 120, 255 } });

    // Store Shelves / Gondolas (Aisles)
    Color shelfWoodColor = { 130, 90, 60, 255 };
    Color shelfTopColor = { 160, 120, 80, 255 };

    // Aisle 1 (Left Aisle: 2 shelves)
    shelves.push_back({ { -5.5f, 1.1f, -4.0f }, { 2.0f, 2.2f, 6.0f }, shelfWoodColor, shelfTopColor });
    shelves.push_back({ { -5.5f, 1.1f, 4.0f }, { 2.0f, 2.2f, 6.0f }, shelfWoodColor, shelfTopColor });

    // Aisle 2 (Right Aisle: 2 shelves)
    shelves.push_back({ { 5.5f, 1.1f, -4.0f }, { 2.0f, 2.2f, 6.0f }, shelfWoodColor, shelfTopColor });
    shelves.push_back({ { 5.5f, 1.1f, 4.0f }, { 2.0f, 2.2f, 6.0f }, shelfWoodColor, shelfTopColor });

    // Center Island Display Table
    shelves.push_back({ { 0.0f, 0.6f, -3.0f }, { 3.0f, 1.2f, 4.0f }, { 80, 130, 180, 255 }, { 100, 160, 210, 255 } });

    // Counter table near entrance
    shelves.push_back({ { 5.0f, 0.6f, 9.0f }, { 4.0f, 1.2f, 1.5f }, { 70, 75, 80, 255 }, { 90, 95, 100, 255 } });
}

void Shop::BuildColliders() {
    colliders.clear();

    // Wall colliders
    for (const auto& w : walls) {
        AABB box;
        box.min = { w.position.x - w.size.x / 2.0f, 0.0f, w.position.z - w.size.z / 2.0f };
        box.max = { w.position.x + w.size.x / 2.0f, w.position.y + w.size.y / 2.0f, w.position.z + w.size.z / 2.0f };
        colliders.push_back(box);
    }

    // Shelf colliders
    for (const auto& s : shelves) {
        AABB box;
        box.min = { s.position.x - s.size.x / 2.0f, 0.0f, s.position.z - s.size.z / 2.0f };
        box.max = { s.position.x + s.size.x / 2.0f, s.position.y + s.size.y / 2.0f, s.position.z + s.size.z / 2.0f };
        colliders.push_back(box);
    }
}

void Shop::Render() {
    // Floor
    DrawPlane({ 0.0f, 0.0f, 0.0f }, { shopWidth + 8.0f, shopLength + 8.0f }, { 210, 215, 210, 255 }); // ground
    DrawCube({ 0.0f, -0.05f, 0.0f }, shopWidth, 0.1f, shopLength, { 240, 240, 245, 255 }); // shop tile floor
    DrawGrid((int)(shopLength / 2), 2.0f); // grid texture feel

    // Ceiling
    DrawCube({ 0.0f, shopHeight + 0.05f, 0.0f }, shopWidth, 0.1f, shopLength, { 180, 185, 190, 255 });

    // Render Walls
    for (const auto& w : walls) {
        DrawCube(w.position, w.size.x, w.size.y, w.size.z, w.color);
        DrawCubeWires(w.position, w.size.x, w.size.y, w.size.z, { 70, 75, 80, 255 });
    }

    // Door Frame Posts (accents)
    DrawCube({ -2.0f, 1.8f, shopLength / 2.0f }, 0.2f, 3.6f, 0.6f, { 80, 85, 90, 255 });
    DrawCube({ 2.0f, 1.8f, shopLength / 2.0f }, 0.2f, 3.6f, 0.6f, { 80, 85, 90, 255 });

    // Render Shelves
    for (const auto& s : shelves) {
        DrawCube(s.position, s.size.x, s.size.y, s.size.z, s.color);
        DrawCubeWires(s.position, s.size.x, s.size.y, s.size.z, { 40, 40, 45, 255 });
        
        // Shelf Top highlight
        Vector3 topPos = { s.position.x, s.position.y + s.size.y / 2.0f - 0.05f, s.position.z };
        DrawCube(topPos, s.size.x + 0.1f, 0.1f, s.size.z + 0.1f, s.topColor);

        // Sub-tier shelf lines to give realistic shop rack appearance
        if (s.size.y > 1.5f) {
            Vector3 midPos = { s.position.x, s.position.y, s.position.z };
            DrawCube(midPos, s.size.x + 0.05f, 0.08f, s.size.z + 0.05f, s.topColor);
        }
    }
}
