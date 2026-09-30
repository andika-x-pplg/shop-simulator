#pragma once
#include "Common.hpp"
#include "Product.hpp"
#include <vector>
#include <string>

class Player {
public:
    Player();
    ~Player() = default;

    void Init(Vector3 startPosition);
    void Update(float deltaTime, const std::vector<AABB>& colliders);
    void RenderHeldItem();
    
    Camera3D GetCamera() const { return camera; }
    Vector3 GetPosition() const { return position; }
    void SetPosition(Vector3 newPos) {
        position = newPos;
        camera.position = { position.x, position.y + eyeHeight, position.z };
        camera.target = { position.x + cosf(pitch) * cosf(yaw), position.y + eyeHeight + sinf(pitch), position.z + cosf(pitch) * sinf(yaw) };
    }
    Vector3 GetEyePosition() const;
    Vector3 GetLookDirection() const;

    // Carrying product slot
    ProductType GetHeldProduct() const { return heldProduct; }
    std::string GetHeldProductName() const;
    bool IsHoldingProduct() const { return heldProduct != ProductType::NONE; }
    void PickUpProduct(ProductType product);
    ProductType DropOrPlaceProduct();

    // Feedback message (e.g. "Tangan penuh!", "Stok rak kosong!")
    void SetFeedbackMessage(const std::string& msg, float duration = 2.0f);
    std::string GetFeedbackMessage() const { return feedbackMessage; }
    bool HasFeedbackMessage() const { return feedbackTimer > 0.0f; }

private:
    Camera3D camera;
    Vector3 position;
    Vector3 velocity;
    
    float moveSpeed;
    float mouseSensitivity;
    float playerRadius;
    float playerHeight;
    float eyeHeight;

    float pitch;
    float yaw;

    // Single item carrying system
    ProductType heldProduct;

    // Feedback message timer
    std::string feedbackMessage;
    float feedbackTimer;

    void HandleMouseLook();
    void HandleMovement(float deltaTime, const std::vector<AABB>& colliders);
    AABB GetBoundingBox(Vector3 pos) const;
};
