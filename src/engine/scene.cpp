#include "scene.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

void compileShader(unsigned &program, const char* vraw, const char* fraw);
void createVertexObject(unsigned &VAO, unsigned &VBO, unsigned &EBO, float vertices[], unsigned int indices[], float vertSize, float indSize, int drawType, GLint vertexAttribSize, bool texture, GLsizei stride, const void* pointer);
void createTexture2D(unsigned &var, int wrapping, int minfilter, int maxfilter, std::string path);
namespace leto {
    shader::shader(std::string name) {
        std::ifstream vfile("../src/data/shaders/" + name + "/vertex.vs"), ffile("../src/data/shaders/" + name + "/fragment.fs");
        if(vfile && ffile) {
            std::stringstream vstream, fstream; vstream << vfile.rdbuf(); fstream << ffile.rdbuf(); std::string vstr = vstream.str(), fstr = fstream.str(); 
            const char *vraw = vstr.c_str(), *fraw = fstr.c_str();
            compileShader(PROGRAM, vraw, fraw);
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
    void decal::render(shader &shader) {
        // should work? idk test this when home
        glBindTexture(GL_TEXTURE_2D, TEXTURE); shader.use();
        glBindVertexArray(VAO); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

void compileShader(unsigned &program, const char* vraw, const char* fraw) {
    auto checkErrors = [](unsigned &shader){ // fucky lambda magic
        int success; char infoLog[1024]; glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) { 
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "A shader had an error during compilation. Please check all file paths if this is happening during debug, if you're playing a release, please redownload the files, there is likely one missing. Error:\n" << infoLog << std::endl;
        }
    };
    unsigned vertex = glCreateShader(GL_VERTEX_SHADER), fragment = glCreateShader(GL_FRAGMENT_SHADER); 
    glShaderSource(vertex, 1, &vraw, NULL); glShaderSource(fragment, 1, &fraw, NULL); glCompileShader(vertex); checkErrors(vertex); glCompileShader(fragment); checkErrors(fragment);
    program = glCreateProgram(); glAttachShader(program, vertex); glAttachShader(program, fragment);
    glLinkProgram(program); glDetachShader(program, vertex); glDeleteShader(vertex); glDetachShader(program, vertex); glDeleteShader(fragment);
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