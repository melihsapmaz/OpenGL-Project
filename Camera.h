#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <vector>
#include <unordered_map>

// Forward declarations
class Shader;
class Model;

class Camera {
public:
    // Stores the main vectors of the camera
    glm::vec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 cameraMatrix = glm::mat4(1.0f);

    // Camera orientation parameters
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = 0.0f;
    float lastY = 0.0f;
    float fov = 45.0f;

    // Time tracking
    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;

    // List of models to check for collisions
    std::vector<Model*> collidableModels;

    // Camera control variables
    bool firstClick = true;
    int width;
    int height;
    float speed = 0.1f;
    float sensitivity = 100.0f;

    // Camera constructor
    Camera(int width, int height, glm::vec3 position);

    // Camera matrix functions
    void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
    void Matrix(Shader& shader, const char* uniform);
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

    // Input handling
    void Inputs(GLFWwindow* window);

    // Collision functions
    void AddCollidableModel(Model* model) {
        collidableModels.push_back(model);
    }
    void ClearCollidableModels() {
        collidableModels.clear();
    }

    // Ray casting collision detection
    bool CheckCollisionRayCast(const glm::vec3& newPosition);

    // Cast rays in multiple directions to check for collisions
    bool CastRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);

    // Helper method to check ray intersection with an AABB (Axis-Aligned Bounding Box)
    bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                           const glm::vec3& boxMin, const glm::vec3& boxMax, float& distance);

    // Store model transforms for collision detection
    std::unordered_map<const Model*, glm::mat4> modelMatrices;

    // Method to register model with its transform
    void RegisterModelTransform(const Model* model, const glm::mat4& transform) {
        modelMatrices[model] = transform;
    }

    struct ModelInstance {
        const Model* model;
        glm::mat4 transform;
    };

    std::vector<ModelInstance> collidableInstances;

    // Change the RegisterModelTransform method:
    void AddModelInstance(const Model* model, const glm::mat4& transform) {
        collidableInstances.push_back({model, transform});
    }
    void TestDirectCollision(const glm::vec3& position);
};

#endif
