#include "camera.hpp"
namespace leto {
    void camera::processKeyboard(movement direction, float deltaTime) {
        float speed = movementSpeed * deltaTime;
        if (direction == FORWARD) { position += front * speed; } if (direction == BACKWARD) { position -= front * speed; }
        if (direction == LEFT) { position -= right * speed; } if (direction == RIGHT) { position += right * speed; }
        if (direction == UP) { position += up * speed; } if (direction == DOWN) { position -= up * speed; }
    }
    void camera::processMouse(float xoffset, float yoffset) {
        xoffset *= sensitivity; yoffset *= sensitivity;
        yaw += xoffset; pitch += yoffset;

        // make sure that when pitch is out of bounds screen doesn't get flipped
        if (pitch > 89.0f) { pitch = 89.0f; } if (pitch < -89.0f) { pitch = -89.0f; }
        updateFront();
    }
    void camera::processScroll(float yoffset) { zoom -= (float)yoffset; if (zoom < 1.0f) { zoom = 1.0f; } if (zoom > 45.0f) { zoom = 45.0f; } }
}