#ifndef SRC_ENGINE_WINDOW_HPP
#define SRC_ENGINE_WINDOW_HPP

    #include <string>
    #include <vector>
    #include <iostream>
    #include "scene.hpp"
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
                float FPS, LASTFRAME, DELTATIME;
                bool VYSNC = true, ANTIALIASING = true;
                std::string TITLE; monitor PRIMARY; envtype ENVIRONMENT = DEBUG;
            public:
                GLFWwindow *instance; float background[3] = {1.0f, 1.0f, 1.0f}; // init to pure white
                std::vector<scene> scenes;
                window(std::string title, std::string icon, bool autostart); void changebg(glm::vec3 newbg) { background[0] = newbg.x; background[1] = newbg.y; background[2] = newbg.z; }
                void addscene(scene &addition) { scenes.push_back(addition); }
                void start(), render(), update();
        };
    }

#endif