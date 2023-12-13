#ifndef SRC_ENGINE_WINDOW_HPP
#define SRC_ENGINE_WINDOW_HPP

    #include "scene.hpp"
    #include <GLFW/glfw3.h>
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
                int WIDTH, HEIGHT; float LASTFRAME, DELTATIME;
                bool VSYNC = true, ANTIALIASING = true;
                std::string TITLE; envtype ENVIRONMENT = DEBUG;
                monitor PRIMARY; camera CAMERA;

                static void mouse_callback(GLFWwindow* instance, double xpos, double ypos), scroll_callback(GLFWwindow* instance, double xoffset, double yoffset);
                virtual void mouse_callback(double xposIn, double yposIn), scroll_callback(double xoffset, double yoffset);
            public:
                GLFWwindow *instance; float background[3] = {1.0f, 1.0f, 1.0f}; // init to pure white
                std::vector<scene> scenes; std::vector<shader> shaders;
                window(std::string title, std::string icon, bool flycamera, bool autostart);
                void changeBG(glm::vec3 newbg); void addScene(scene &addition), addShader(shader &addition), addModel(int scene, model &addition), addDecal(int scene, decal &addition);
                void start(), render(), update();
        };
    }

#endif