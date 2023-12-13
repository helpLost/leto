#ifndef SRC_ENGINE_CAMERA_HPP
#define SRC_ENGINE_CAMERA_HPP

    #include <iostream>
    #include <GLAD/glm.hpp>
    #include <GLAD/gtc/matrix_transform.hpp>
    namespace leto {
        enum movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN }; enum type { FLY, FPS, CINEMATIC };
        class camera
        {
            public:
                float lastX, lastY, yaw = -90.0f, pitch = 0.0f, zoom = 45.0f;
                bool firstMouse = true; type cameratype;

                camera(glm::vec3 position, int windowWidth, int windowHeight) :position(position), worldUp(up), lastX(windowWidth / 2.0f), lastY(windowHeight / 2.0f) { updateFront(); }
                void processKeyboard(movement direction, float deltaTime), processMouse(float xoffset, float yoffset), processScroll(float yoffset);
                glm::mat4 GetViewMatrix() { return glm::lookAt(position, position + front, up); }
            private:
                float movementSpeed = 2.5f, sensitivity = 0.1f;
                glm::vec3 position, front = glm::vec3(0.0f, 0.0f, -1.0f), up = glm::vec3(0.0f, 1.0f, 0.0f), right, worldUp;
                // calculates the front vector from the Camera's (updated) Euler Angles
                void updateFront()
                {
                    // calculate the new Front vector
                    glm::vec3 fr;
                    fr.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
                    fr.y = sin(glm::radians(pitch));
                    fr.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
                    front = glm::normalize(fr);
                    // also re-calculate the Right and Up vector
                    right = glm::normalize(glm::cross(front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
                    up    = glm::normalize(glm::cross(right, front));
                }
        };
    }

#endif