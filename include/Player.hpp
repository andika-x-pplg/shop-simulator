#pragma once
#include "Common.hpp"
#include <vector>

class Player {
public:
    Player();
    ~Player() = default;

    void Init(Vector3 startPosition);
    void Update(float deltaTime, const std::vector<AABB>& colliders);
    
    Camera3D GetCamera() const { return camera; }
    Vector3 GetPosition() const { return position; }

private:
    Camera3D camera;
    Vector3 position;
    Vector3 velocity;
    
    float moveSpeed;
    float mouseSensitivity;
    float playerRadius;
    float playerHeight;
    float eyeHeight;

    float pitch; // Camera vertical angle (radians or degrees)
    float yaw;   // Camera horizontal angle

    void HandleMouseLook();
    void HandleMovement(float deltaTime, const std::vector<AABB>& colliders);
    AABB GetBoundingBox(Vector3 pos) const;
};
