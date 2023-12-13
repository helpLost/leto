#include "window.hpp"
#include <GLAD/gtc/matrix_transform.hpp>
#include <GLAD/gtc/type_ptr.hpp>
#include <STB/stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
namespace leto {
    #pragma region INITIALIZATION
        monitor::monitor() {
            if(!glfwInit()) { throw std::runtime_error("GLFW failed to initialize. Something's seriously wrong with the program."); }
            primary = glfwGetPrimaryMonitor(); resolution = glfwGetVideoMode(primary); // get the actual monitor and its details
            if(!primary) { endprogram("Failed to grab the primary monitor. You're probably on an unsupported machine."); } std::cout << "Primary monitor successfully grabbed. Default dimensions are " << resolution->width << "x" << resolution->height << "." << std::endl;
        }
        window::window(std::string title, std::string icon, bool flycamera, bool autostart) :TITLE(title), CAMERA(glm::vec3(0.0f, 0.0f, 3.0f), PRIMARY.dimensions().x, PRIMARY.dimensions().y) {
            // Params
            if(TITLE.empty()) { endprogram("A window's title is null. This is very likely a problem with the source code, please open a ticket on Github."); }
            WIDTH = int(PRIMARY.dimensions().x / 1.25); HEIGHT = int(PRIMARY.dimensions().y / 1.25); if(WIDTH <= 100 || HEIGHT <= 100) { endprogram("Window '" + TITLE + "'s width and height don't seem to be accurate. Try restarting the program."); }
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            // Creating the window
            instance = glfwCreateWindow(WIDTH, HEIGHT, TITLE.c_str(), NULL, NULL); if(!instance) { endprogram("GLFW failed to create a window. Try restarting the program."); }
            glfwMakeContextCurrent(instance); glfwSetWindowPos(instance, 10, 50); // setting this window to current
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { endprogram("GLAD failed to initialize. Something's gone very, very wrong."); }  
            glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // remember blending will fuck with text
            glfwSetFramebufferSizeCallback(instance, framebuffer_size_callback); glfwSetWindowUserPointer(instance, this); glfwSetCursorPosCallback(instance, mouse_callback); glfwSetScrollCallback(instance, scroll_callback);
            glfwSetInputMode(instance, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            // Window icon loading
            GLFWimage images[1]; std::string path = "../src/data/assets/images/interface/" + icon;
            images[0].pixels = stbi_load(path.c_str(), &images[0].width, &images[0].height, 0, 4); // last param is RGBA channels, with four it means the image must have an alpha
            if(images[0].pixels) { glfwSetWindowIcon(instance, 1, images); } else { std::cout << "Failed to find window icon for window '" << TITLE << "' at " << path << "'. This is not a fatal error, but a file's probably missing." << std::endl; }
            stbi_image_free(images[0].pixels);

            // Wrapping up
            shader dcl("dcl"), mdl("mdl"); scene sce;
            addShader(dcl); addShader(mdl); addScene(sce);

            if(VSYNC) { glfwSwapInterval(1); }
            std::cout << "Created window '" << TITLE << "' successfully. Default dimensions are " << WIDTH << "x" << HEIGHT << "." << std::endl;
            if(autostart) { start(); } // if autostart is false you have to call "start()" somewhere
        }
    #pragma endregion
    #pragma region RUNTIME
        void window::start() { model mdl("../src/data/assets/models/backpack/backpack.obj"); addModel(0, mdl); while(!glfwWindowShouldClose(instance)) { render(); update(); } endprogram(); } // run the window until it's closed
        void window::render() {
            // Color the backgroud and clear the buffer bits
            glClearColor(background[0], background[1], background[2], 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shaders[1].use();

            // view/projection transformations
            glm::mat4 projection = glm::perspective(glm::radians(CAMERA.zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = CAMERA.GetViewMatrix();
            shaders[1].setMat4("projection", projection);
            shaders[1].setMat4("view", view);

            // render the loaded model
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
            shaders[1].setMat4("model", model);

            scenes[0].render(shaders);

            // Swap the buffers
            glfwSwapBuffers(instance);
        }
    void window::update() {
        // Get Deltatime
        float currentFrame = static_cast<float>(glfwGetTime());
        DELTATIME = currentFrame - LASTFRAME; LASTFRAME = currentFrame;

        if (glfwGetKey(instance, GLFW_KEY_W) == GLFW_PRESS)
            CAMERA.processKeyboard(FORWARD, DELTATIME);
        if (glfwGetKey(instance, GLFW_KEY_S) == GLFW_PRESS)
            CAMERA.processKeyboard(BACKWARD, DELTATIME);
        if (glfwGetKey(instance, GLFW_KEY_A) == GLFW_PRESS)
            CAMERA.processKeyboard(LEFT, DELTATIME);
        if (glfwGetKey(instance, GLFW_KEY_D) == GLFW_PRESS)
            CAMERA.processKeyboard(RIGHT, DELTATIME);
        if (glfwGetKey(instance, GLFW_KEY_SPACE) == GLFW_PRESS)
            CAMERA.processKeyboard(UP, DELTATIME);
        if (glfwGetKey(instance, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            CAMERA.processKeyboard(DOWN, DELTATIME);

        // Poll for any window events (close, framebuffer, iconify, etc)
        glfwPollEvents();
    }
    #pragma endregion
    #pragma region CHANGES AND CALLBACKS
        void window::changeBG(glm::vec3 newbg) { background[0] = newbg.x; background[1] = newbg.y; background[2] = newbg.z; }
        void window::addScene(scene &addition) { scenes.push_back(addition); } void window::addShader(shader &addition) { shaders.push_back(addition); } void window::addModel(int scene, model &addition) { scenes[scene].models.push_back(addition); } void window::addDecal(int scene, decal &addition) { scenes[scene].decals.push_back(addition); }

        void window::mouse_callback(GLFWwindow* instance, double xpos, double ypos) { window* obj = reinterpret_cast<window *>(glfwGetWindowUserPointer(instance)); obj->mouse_callback(xpos, ypos); }
        void window::scroll_callback(GLFWwindow* instance, double xoffset, double yoffset) { window* obj = reinterpret_cast<window *>(glfwGetWindowUserPointer(instance)); obj->scroll_callback(xoffset, yoffset); } 

        void window::mouse_callback(double xposIn, double yposIn) {
            if (CAMERA.firstMouse) { CAMERA.lastX = xposIn; CAMERA.lastY = yposIn; CAMERA.firstMouse = false; }
            float xoffset = xposIn - CAMERA.lastX, yoffset = CAMERA.lastY - yposIn; // y reversed since y-coordinates go from bottom to top
            CAMERA.lastX = xposIn; CAMERA.lastY = yposIn; CAMERA.processMouse(xoffset, yoffset);
        }
        void window::scroll_callback(double xoffset, double yoffset) { CAMERA.processScroll(static_cast<float>(yoffset)); }
    #pragma endregion
}