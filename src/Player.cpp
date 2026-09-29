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
      yaw(-PI / 2.0f) // Face forward into shop (-Z direction)
{
}

void Player::Init(Vector3 startPosition) {
    position = startPosition;
    pitch = 0.0f;
    yaw = -PI / 2.0f;

    camera.position = { position.x, position.y + eyeHeight, position.z };
    camera.target = { position.x, position.y + eyeHeight, position.z - 1.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;
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

    // Update Camera position and direction
    camera.position = { position.x, position.y + eyeHeight, position.z };

    Vector3 lookDir;
    lookDir.x = cosf(pitch) * cosf(yaw);
    lookDir.y = sinf(pitch);
    lookDir.z = cosf(pitch) * sinf(yaw);

    camera.target = Vector3Add(camera.position, lookDir);
}

void Player::Update(float deltaTime, const std::vector<AABB>& colliders) {
    HandleMouseLook();
    HandleMovement(deltaTime, colliders);
}
