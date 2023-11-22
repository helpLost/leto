#include "window.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
namespace leto {
    monitor::monitor() {
        if(!glfwInit()) { throw std::runtime_error("GLFW failed to initialize. Something's seriously wrong with the program."); }
        primary = glfwGetPrimaryMonitor(); resolution = glfwGetVideoMode(primary); // get the actual monitor and its details
        if(!primary) { endprogram("Failed to grab the primary monitor. You're probably on an unsupported machine."); } std::cout << "Primary monitor successfully grabbed. Default dimensions are " << resolution->width << "x" << resolution->height << "." << std::endl;
    }

    window::window(std::string title, std::string icon, bool autostart) :TITLE(title)
    {
        // Params
        if(TITLE.empty()) { endprogram("A window's title is null. This is very likely a problem with the source code, please open a ticket on Github."); }
        WIDTH = int(PRIMARY.dimensions().x / 1.25); HEIGHT = int(PRIMARY.dimensions().y / 1.25); if(WIDTH <= 100 || HEIGHT <= 100) { endprogram("Window '" + TITLE + "'s width and height don't seem to be accurate. Try restarting the program."); }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Creating the window
        instance = glfwCreateWindow(WIDTH, HEIGHT, TITLE.c_str(), NULL, NULL); if(!instance) { endprogram("GLFW failed to create a window. Try restarting the program."); }
        glfwMakeContextCurrent(instance); glfwSetWindowPos(instance, 10, 50); if(ENVIRONMENT == PRERELEASE || ENVIRONMENT == RELEASE) { glfwSetInputMode(instance, GLFW_CURSOR, GLFW_CURSOR_DISABLED); } // setting this window to current
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { endprogram("GLAD failed to initialize. Something's gone very, very wrong."); }  
        glEnable(GL_DEPTH_TEST); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // remember blending will fuck with text
        glfwSetFramebufferSizeCallback(instance, framebuffer_size_callback);

        // Window icon loading
        GLFWimage images[1]; std::string path = "../src/data/interface/" + icon;
        images[0].pixels = stbi_load(path.c_str(), &images[0].width, &images[0].height, 0, 4); // last param is RGBA channels
        if(images[0].pixels) { glfwSetWindowIcon(instance, 1, images); } else { std::cout << "Failed to find window icon for window '" << TITLE << "' at 'src/data/interface/" << icon << "'. This is not a fatal error, but a file's probably missing." << std::endl; }
        stbi_image_free(images[0].pixels);

        // Wrapping up
        if(VYSNC) { glfwSwapInterval(1); }
        std::cout << "Created window '" << TITLE << "' successfully. Default dimensions are " << WIDTH << "x" << HEIGHT << "." << std::endl;
        if(autostart) { start(); } // if autostart is false you have to call "start()" somewhere
    }

    void window::start() { while(!glfwWindowShouldClose(instance)) { render(); update(); } endprogram(); } // run the window until it's closed
    void window::render() {
        // Color the backgroud and clear the buffer bits
        glClearColor(background[0], background[1], background[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Swap the buffers
        glfwSwapBuffers(instance);
    }
    void window::update() {
        // Poll for any window events (close, framebuffer, iconify, etc)
        glfwPollEvents();
    }
}