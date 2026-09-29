#include "Player.hpp"
#include <cmath>

Player::Player()
    : position{ 0.0f, 0.0f, 10.0f },
      velocity{ 0.0f, 0.0f, 0.0f },
      moveSpeed(6.0f),
      mouseSensitivity(0.003f),
      playerRadius(0.4f),
      playerHeight(1.8f),
      eyeHeight(1.65f),
      pitch(0.0f),
      yaw(-PI / 2.0f), // Face forward into shop (-Z direction)
      heldProduct(ProductType::NONE),
      feedbackMessage(""),
      feedbackTimer(0.0f)
{
}

void Player::Init(Vector3 startPosition) {
    position = startPosition;
    pitch = 0.0f;
    yaw = -PI / 2.0f;
    heldProduct = ProductType::NONE;
    feedbackMessage = "";
    feedbackTimer = 0.0f;

    camera.position = { position.x, position.y + eyeHeight, position.z };
    camera.target = { position.x, position.y + eyeHeight, position.z - 1.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

Vector3 Player::GetEyePosition() const {
    return { position.x, position.y + eyeHeight, position.z };
}

Vector3 Player::GetLookDirection() const {
    Vector3 lookDir;
    lookDir.x = cosf(pitch) * cosf(yaw);
    lookDir.y = sinf(pitch);
    lookDir.z = cosf(pitch) * sinf(yaw);
    return lookDir;
}

std::string Player::GetHeldProductName() const {
    return GetProductInfo(heldProduct).name;
}

void Player::PickUpProduct(ProductType product) {
    heldProduct = product;
}

ProductType Player::DropOrPlaceProduct() {
    ProductType previous = heldProduct;
    heldProduct = ProductType::NONE;
    return previous;
}

void Player::SetFeedbackMessage(const std::string& msg, float duration) {
    feedbackMessage = msg;
    feedbackTimer = duration;
}

AABB Player::GetBoundingBox(Vector3 pos) const {
    AABB box;
    box.min = { pos.x - playerRadius, pos.y, pos.z - playerRadius };
    box.max = { pos.x + playerRadius, pos.y + playerHeight, pos.z + playerRadius };
    return box;
}

void Player::HandleMouseLook() {
    Vector2 mouseDelta = GetMouseDelta();
    
    yaw += mouseDelta.x * mouseSensitivity;
    pitch -= mouseDelta.y * mouseSensitivity;

    // Clamp pitch to avoid flipping upside down (-89 deg to 89 deg)
    const float maxPitch = 89.0f * DEG2RAD;
    if (pitch > maxPitch) pitch = maxPitch;
    if (pitch < -maxPitch) pitch = -maxPitch;
}

void Player::HandleMovement(float deltaTime, const std::vector<AABB>& colliders) {
    Vector3 forward = { cosf(yaw), 0.0f, sinf(yaw) };
    Vector3 right = { -sinf(yaw), 0.0f, cosf(yaw) };

    Vector3 moveDir = { 0.0f, 0.0f, 0.0f };

    if (IsKeyDown(KEY_W)) moveDir = Vector3Add(moveDir, forward);
    if (IsKeyDown(KEY_S)) moveDir = Vector3Subtract(moveDir, forward);
    if (IsKeyDown(KEY_D)) moveDir = Vector3Add(moveDir, right);
    if (IsKeyDown(KEY_A)) moveDir = Vector3Subtract(moveDir, right);

    if (Vector3Length(moveDir) > 0.001f) {
        moveDir = Vector3Normalize(moveDir);
    }

    Vector3 delta = Vector3Scale(moveDir, moveSpeed * deltaTime);

    // X-Axis collision check
    Vector3 testPosX = position;
    testPosX.x += delta.x;
    AABB playerBoxX = GetBoundingBox(testPosX);
    bool collideX = false;
    for (const auto& box : colliders) {
        if (playerBoxX.CheckCollision(box)) {
            collideX = true;
            break;
        }
    }
    if (!collideX) {
        position.x = testPosX.x;
    }

    // Z-Axis collision check
    Vector3 testPosZ = position;
    testPosZ.z += delta.z;
    AABB playerBoxZ = GetBoundingBox(testPosZ);
    bool collideZ = false;
    for (const auto& box : colliders) {
        if (playerBoxZ.CheckCollision(box)) {
            collideZ = true;
            break;
        }
    }
    if (!collideZ) {
        position.z = testPosZ.z;
    }

    // Update Camera position and target direction
    camera.position = GetEyePosition();
    Vector3 lookDir = GetLookDirection();
    camera.target = Vector3Add(camera.position, lookDir);
}

void Player::Update(float deltaTime, const std::vector<AABB>& colliders) {
    if (feedbackTimer > 0.0f) {
        feedbackTimer -= deltaTime;
        if (feedbackTimer <= 0.0f) {
            feedbackMessage = "";
        }
    }

    HandleMouseLook();
    HandleMovement(deltaTime, colliders);
}

void Player::RenderHeldItem() {
    if (heldProduct == ProductType::NONE) return;

    ProductInfo info = GetProductInfo(heldProduct);

    // Calculate position in front of the camera (slightly bottom-right like a first-person item hold)
    Vector3 lookDir = Vector3Normalize(GetLookDirection());
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(lookDir, camera.up));
    Vector3 camUp = camera.up;

    // Offset: 0.55m forward, 0.22m right, -0.25m down
    Vector3 holdOffset = Vector3Add(Vector3Scale(lookDir, 0.55f),
                         Vector3Add(Vector3Scale(camRight, 0.22f),
                                    Vector3Scale(camUp, -0.22f)));

    Vector3 itemPos = Vector3Add(camera.position, holdOffset);

    // Draw the held item with subtle depth
    if (heldProduct == ProductType::BEVERAGE) {
        DrawCube(itemPos, info.modelDimensions.x * 0.9f, info.modelDimensions.y * 0.9f, info.modelDimensions.z * 0.9f, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x * 0.9f, info.modelDimensions.y * 0.9f, info.modelDimensions.z * 0.9f, { 20, 80, 160, 255 });

        Vector3 capPos = { itemPos.x, itemPos.y + (info.modelDimensions.y * 0.9f) / 2.0f + 0.03f, itemPos.z };
        DrawCube(capPos, info.modelDimensions.x * 0.6f, 0.06f, info.modelDimensions.z * 0.6f, info.secondaryColor);
    }
    else if (heldProduct == ProductType::BREAD) {
        DrawCube(itemPos, info.modelDimensions.x * 0.9f, info.modelDimensions.y * 0.9f, info.modelDimensions.z * 0.9f, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x * 0.9f, info.modelDimensions.y * 0.9f, info.modelDimensions.z * 0.9f, { 140, 90, 40, 255 });

        Vector3 topSlit = { itemPos.x, itemPos.y + (info.modelDimensions.y * 0.9f) / 2.0f + 0.02f, itemPos.z };
        DrawCube(topSlit, info.modelDimensions.x * 0.7f, 0.03f, info.modelDimensions.z * 0.35f, info.secondaryColor);
    }
    else if (heldProduct == ProductType::CANNED_FOOD) {
        DrawCube(itemPos, info.modelDimensions.x * 0.9f, info.modelDimensions.y * 0.9f, info.modelDimensions.z * 0.9f, info.primaryColor);
        DrawCubeWires(itemPos, info.modelDimensions.x * 0.9f, info.modelDimensions.y * 0.9f, info.modelDimensions.z * 0.9f, { 150, 30, 30, 255 });

        Vector3 rimTop = { itemPos.x, itemPos.y + (info.modelDimensions.y * 0.9f) / 2.0f + 0.02f, itemPos.z };
        DrawCube(rimTop, info.modelDimensions.x * 0.8f, 0.03f, info.modelDimensions.z * 0.8f, info.secondaryColor);
    }
}
