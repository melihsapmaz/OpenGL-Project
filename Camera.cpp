#include "Camera.h"
#include "ShaderClass.h"
#include "Model.h"

Camera::Camera(int width, int height, glm::vec3 position) {
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane) {
    // Initializes matrices
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // Makes camera look in the right direction from the right position
    view = glm::lookAt(Position, Position + Orientation, Up);

    // Use the camera's fov member variable instead of the parameter
    projection = glm::perspective(glm::radians(fov), (float) width / height, nearPlane, farPlane);

    // Sets new camera matrix
    cameraMatrix = projection * view;
}



void Camera::Matrix(Shader& shader, const char* uniform) {
	// Exports camera matrix
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow* window) {
    // Calculate delta time for frame-rate independent movement
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrameTime;
    lastFrameTime = currentFrame;

    // Limit delta time to prevent jumps after pauses
    if (deltaTime > 0.1f)
        deltaTime = 0.1f;

    // Store original height for maintaining fixed camera height
    float originalHeight = Position.y;

    // Calculate movement vectors in XZ plane only (for FPS-style movement)
    glm::vec3 forwardDir = glm::normalize(glm::vec3(Orientation.x, 0.0f, Orientation.z));
    glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, Up));

    // Base movement speed (units per second)
    float baseSpeed = 2.5f;
    float currentSpeed = baseSpeed * deltaTime;

    // Sprint option - hold shift for faster movement
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentSpeed *= 2.0f;

    // Track if any movement keys are pressed
    bool keyPressed = false;

    // Calculate desired movement
    glm::vec3 moveDir = glm::vec3(0.0f);

    // Check movement keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        moveDir += forwardDir;
        keyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        moveDir -= forwardDir;
        keyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        moveDir -= rightDir;
        keyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        moveDir += rightDir;
        keyPressed = true;
    }

    // Only move if a key was pressed
    if (keyPressed && glm::length(moveDir) > 0.0f) {
        if (glm::length(moveDir) > 1.0f)
            moveDir = glm::normalize(moveDir);
        glm::vec3 newPosition = Position + moveDir * currentSpeed;
        newPosition.y = originalHeight;

        // THIS IS THE IMPORTANT PART:
        if (!CheckCollisionRayCast(newPosition)) {
            Position = newPosition;
        } else {
            // Try sliding along walls
            glm::vec3 newPosX = Position + glm::vec3(moveDir.x * currentSpeed, 0.0f, 0.0f);
            newPosX.y = originalHeight;
            if (!CheckCollisionRayCast(newPosX)) {
                Position = newPosX;
            }
            glm::vec3 newPosZ = Position + glm::vec3(0.0f, 0.0f, moveDir.z * currentSpeed);
            newPosZ.y = originalHeight;
            if (!CheckCollisionRayCast(newPosZ)) {
                Position = newPosZ;
            }
        }
    }


    // ESC to quit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // MOUSE CONTROLS - Always active for looking around
    // Hide cursor permanently for FPS-style camera
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Handle first frame
    if (firstClick) {
        lastX = mouseX;
        lastY = mouseY;
        firstClick = false;
    }

    // Calculate mouse offset
    float xOffset = mouseX - lastX;
    float yOffset = lastY - mouseY; // Reversed for intuitive controls

    // Update last position
    lastX = mouseX;
    lastY = mouseY;

    // Apply sensitivity - lower value for slower mouse movement
    float mouseSensitivity = 0.05f;  // Reduced from 0.1f to make it slower
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    // Update yaw and pitch
    yaw += xOffset;
    pitch += yOffset;

    // Limit pitch to avoid flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Calculate new orientation
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    Orientation = glm::normalize(direction);

    // Handle zoom same as before
    static bool scrollCallbackRegistered = false;
    if (!scrollCallbackRegistered) {
        // Store camera pointer to window user pointer for callback access
        glfwSetWindowUserPointer(window, this);

        // Set global scroll callback
        glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
            Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(w));
            if (camera) {
                // Adjust FOV based on scroll
                camera->fov -= (float) yoffset * 2.0f;  // Reduced from 5.0f for smoother zoom
                camera->fov = glm::clamp(camera->fov, 10.0f, 90.0f); // Reasonable FOV range
            }
                              });

        scrollCallbackRegistered = true;
    }
}

// Update the CastRay method in Camera.cpp:
bool Camera::CastRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    // First check model instances (with their proper transforms)
    for (const auto& instance : collidableInstances) {
        if (instance.model->RayIntersectsModel(origin, direction, maxDistance, instance.transform)) {
            std::cout << "Collision detected with model instance" << std::endl;
            return true;
        }
    }

    // For debugging purposes (remove in final code)
    if (collidableInstances.empty()) {
        std::cout << "Warning: No model instances registered!" << std::endl;
    }

    return false;
}

bool Camera::CheckCollisionRayCast(const glm::vec3& newPosition) {
    const float playerRadius = 0.6f;
    const int numRays = 24;
    const float maxDistance = playerRadius + 0.1f;

    // Cast rays at three heights: feet, chest, head
    float heights[] = {0.2f, 1.0f, 1.7f}; // Adjust as needed for your model scale

    for (float h : heights) {
        glm::vec3 pos = newPosition;
        pos.y = h;
        for (int i = 0; i < numRays; i++) {
            float angle = (float) i * 2.0f * glm::pi<float>() / (float) numRays;
            glm::vec3 direction(cos(angle), 0.0f, sin(angle));
            if (CastRay(pos, direction, maxDistance)) {
                return true;
            }
        }
    }

    glm::vec3 moveDir = glm::normalize(newPosition - Position);
    if (glm::length(moveDir) > 0.01f) {
        for (float h : heights) {
            glm::vec3 pos = newPosition;
            pos.y = h;
            if (CastRay(pos, moveDir, playerRadius + 0.2f)) {
                return true;
            }
        }
    }
    return false;
}




// Implement in Camera.cpp
bool Camera::RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                               const glm::vec3& boxMin, const glm::vec3& boxMax, float& distance) {
    // Efficient AABB ray intersection algorithm
    glm::vec3 invDir = 1.0f / rayDirection;

    // Handle division by zero
    if (invDir.x == std::numeric_limits<float>::infinity() ||
        invDir.y == std::numeric_limits<float>::infinity() ||
        invDir.z == std::numeric_limits<float>::infinity()) {
        return false;
    }

    glm::vec3 tMin = (boxMin - rayOrigin) * invDir;
    glm::vec3 tMax = (boxMax - rayOrigin) * invDir;

    glm::vec3 t1 = glm::min(tMin, tMax);
    glm::vec3 t2 = glm::max(tMin, tMax);

    float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
    float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

    // If the ray origin is inside the box, treat as a hit
    if (rayOrigin.x >= boxMin.x && rayOrigin.x <= boxMax.x &&
        rayOrigin.y >= boxMin.y && rayOrigin.y <= boxMax.y &&
        rayOrigin.z >= boxMin.z && rayOrigin.z <= boxMax.z) {
        distance = 0.0f;
        std::cout << "Ray origin inside AABB!" << std::endl;
        return true;
    }

    // If tNear > tFar, the ray misses the box
    // If tFar < 0, the box is behind the ray
    if (tNear > tFar || tFar < 0) {
        return false; // No intersection
    }

    distance = tNear;
    return true; // Ray intersects AABB
}

glm::mat4 Camera::GetViewMatrix() {
	// Calculate and return the view matrix using glm::lookAt  
	return glm::lookAt(Position, Position + Orientation, Up);
}

glm::mat4 Camera::GetProjectionMatrix() {
    return glm::perspective(glm::radians(fov), (float) width / height, 0.1f, 100.0f);
}

void Camera::TestDirectCollision(const glm::vec3& position) {

    // Use a simple distance test to identify nearby models
    const float collisionThreshold = 1.0f;

    for (const auto& instance : collidableInstances) {
        glm::vec3 modelPos = glm::vec3(instance.transform[3]); // Extract position from transform
        float distance = glm::distance(
            glm::vec2(position.x, position.z), // XZ plane only
            glm::vec2(modelPos.x, modelPos.z)
        );

        if (distance < collisionThreshold) {
            std::cout << "DIRECT COLLISION DETECTED!" << std::endl;
        }
    }
}