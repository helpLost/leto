#include "scene.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

void compileShader(unsigned &var, const char* raw, int type), linkShader(unsigned &id, unsigned &vertex, unsigned &fragment);
void createVertexObject(unsigned &VAO, unsigned &VBO, unsigned &EBO, float vertices[], unsigned int indices[], float vertSize, float indSize, int drawType, GLint vertexAttribSize, bool texture, GLsizei stride, const void* pointer);
void createTexture2D(unsigned &var, int wrapping, int minfilter, int maxfilter, std::string path);
namespace leto {
    shader::shader(std::string name) {
        std::string vraw, fraw; std::ifstream vfile("../src/data/shaders/" + name + "/vertex.vs"), ffile("../src/data/shaders/" + name + "/fragment.fs");
        const char* vbetter, *fbetter;
        if(vfile && ffile) {
            std::stringstream vstream, fstream; vstream << vfile.rdbuf(); fstream << ffile.rdbuf(); 
            vraw = vstream.str(); fraw = fstream.str(); vbetter = vraw.c_str(); fbetter = fraw.c_str();

            unsigned vertex, fragment;
            compileShader(vertex, vbetter, GL_VERTEX_SHADER); compileShader(fragment, fbetter, GL_FRAGMENT_SHADER);
            linkShader(programID, vertex, fragment);
        } else { std::cout << "The streams for shader '" << name << "' failed to initialize correctly. This error, while not fatal, may cause unforseen gameplay issues." << std::endl; }
    }
    decal::decal(std::string name, float height, float width) {
        float vertices[20] = {
            // positions          // texture coords
            width, height, 0.0f,  1.0f, 1.0f,    width,-height, 0.0f,  1.0f, 0.0f, // top right, bottom right
           -width,-height, 0.0f,  0.0f, 0.0f,   -width, height, 0.0f,  0.0f, 1.0f  // bottom left, top left 
        };
        createVertexObject(VAO, VBO, EBO, vertices, INDICES, sizeof(vertices), sizeof(INDICES), GL_STATIC_DRAW, 3, true, 5 * sizeof(float), (void*)0);
        createTexture2D(TEXTURE, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, "leto.png");
    }
}

void checkErrors(unsigned &shader) {
    int success; char infoLog[1024]; glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) { 
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "A shader had an error during compilation. Please check all file paths if this is happening during debug, if you're playing a release, please redownload the files, there is likely one missing. Error:\n" << infoLog << std::endl;
    }
}
void compileShader(unsigned &var, const char* raw, int type) {
    var = glCreateShader(type); glShaderSource(var, 1, &raw, NULL);
    glCompileShader(var); checkErrors(var);
}
void linkShader(unsigned &id, unsigned &vertex, unsigned &fragment) {
    id = glCreateProgram(); glAttachShader(id, vertex); glAttachShader(id, fragment);
    glLinkProgram(id); glDeleteShader(vertex); glDeleteShader(fragment);
}

void createVertexObject(unsigned &VAO, unsigned &VBO, unsigned &EBO, float vertices[], unsigned int indices[], float vertSize, float indSize, int drawType, GLint vertexAttribSize, bool texture, GLsizei stride, const void* pointer) {
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, drawType); glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, indices, drawType);

    glEnableVertexAttribArray(0); glVertexAttribPointer(0, vertexAttribSize, GL_FLOAT, GL_FALSE, stride, pointer); // position
    if (texture) { glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); } // texture
}
void createTexture2D(unsigned &var, int wrapping, int minfilter, int maxfilter, std::string path) {
    glGenTextures(1, &var); glBindTexture(GL_TEXTURE_2D, var);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);

    int width, height, channels; std::string fullpath = "../src/data/images/" + path; unsigned char *data = stbi_load(fullpath.c_str(), &width, &height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else { std::cout << "Failed to load texture with path '" << path << "'. Please check all filepaths." << std::endl; }
    stbi_image_free(data);
}