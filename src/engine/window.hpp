#ifndef SRC_ENGINE_WINDOW_HPP
#define SRC_ENGINE_WINDOW_HPP

    #include <string>
    #include <vector>
    #include <iostream>
    #include "scene.hpp"
    #include "camera.hpp"
    #include <GLFW/glfw3.h>
    #include <GLAD/glm.hpp>
    namespace leto {
        inline void endprogram(std::string error) { glfwTerminate(); throw std::runtime_error(error.c_str()); }; inline void endprogram() { glfwTerminate(); }
        enum envtype{DEBUG, PRERELEASE, RELEASE};
        class monitor {
            private: const GLFWvidmode *resolution; GLFWmonitor *primary;
            public:
                glm::vec2 dimensions() { return glm::vec2(resolution->width, resolution->height); }
                monitor();
        };
        class window {
            private:
                int WIDTH, HEIGHT;
                float LASTFRAME, DELTATIME;
                bool VSYNC = true, ANTIALIASING = true;
                std::string TITLE; monitor PRIMARY; envtype ENVIRONMENT = DEBUG;
                camera CAMERA = camera(glm::vec3(0.0f, 0.0f, 3.0f));

                static void mouse_callback(GLFWwindow* instance, double xpos, double ypos)
                {
                    window* obj = reinterpret_cast<window *>(glfwGetWindowUserPointer(instance));
                    obj->mouse_callback(xpos, ypos);
                }
                static void scroll_callback(GLFWwindow* instance, double xoffset, double yoffset)
                {
                    window* obj = reinterpret_cast<window *>(glfwGetWindowUserPointer(instance));
                    obj->scroll_callback(xoffset, yoffset);
                } 
            public:
                GLFWwindow *instance; float background[3] = {1.0f, 1.0f, 1.0f}; // init to pure white
                std::vector<scene> scenes; std::vector<shader> shaders;
                window(std::string title, std::string icon, bool flycamera, bool autostart);
                void changebg(glm::vec3 newbg) { background[0] = newbg.x; background[1] = newbg.y; background[2] = newbg.z; }
                void addscene(scene &addition) { scenes.push_back(addition); } void addshader(shader &addition) { shaders.push_back(addition); }
                void start(), render(), update();

                virtual void mouse_callback(double xposIn, double yposIn)
                {
                    float xpos = static_cast<float>(xposIn);
                    float ypos = static_cast<float>(yposIn);

                    if (CAMERA.firstMouse)
                    {
                        CAMERA.lastX = xpos;
                        CAMERA.lastY = ypos;
                        CAMERA.firstMouse = false;
                    }

                    float xoffset = xpos - CAMERA.lastX;
                    float yoffset = CAMERA.lastY - ypos; // reversed since y-coordinates go from bottom to top

                    CAMERA.lastX = xpos;
                    CAMERA.lastY = ypos;

                    CAMERA.ProcessMouseMovement(xoffset, yoffset);
                }
                virtual void scroll_callback(double xoffset, double yoffset)
                {
                    CAMERA.ProcessMouseScroll(static_cast<float>(yoffset));
                }
        };
    }

#endif