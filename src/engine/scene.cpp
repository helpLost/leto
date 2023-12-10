#include "scene.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>
#include <GLAD/gtc/matrix_transform.hpp>
#include <GLAD/gtc/type_ptr.hpp>

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

    decal::decal(std::string name, float height, float width, shader &shader) {
        float vertices[] = {
            // positions          // texture coords
            width, height, 0.0f,  1.0f, 1.0f,    width,-height, 0.0f,  1.0f, 0.0f, // top right, bottom right
           -width,-height, 0.0f,  0.0f, 0.0f,   -width, height, 0.0f,  0.0f, 1.0f  // bottom left, top left 
        };
        createVertexObject(VAO, VBO, EBO, vertices, INDICES, sizeof(vertices), sizeof(INDICES), GL_STATIC_DRAW, 3, true, 5 * sizeof(float), (void*)0);
            float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
            glGenTextures(1, &TEXTURE);
            glBindTexture(GL_TEXTURE_2D, TEXTURE); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_set_flip_vertically_on_load(true);  
            // load image, create texture and generate mipmaps
            int w, h, nrChannels;
            unsigned char *data = stbi_load("E:/Projects/leto/src/data/images/rose.png", &w, &h, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                std::cout << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data);
        // shader.use(); shader.setInt("texture1", 0);
        // createTexture2D(TEXTURE, GL_REPEAT, GL_LINEAR, GL_LINEAR, name);
        // shader.use(); shader.setInt("TEXTURE", 0);
    }
    void decal::render(shader &shader) {
        glBindTexture(GL_TEXTURE_2D, TEXTURE); glBindVertexArray(VAO); 
        glm::mat4 model = glm::mat4(1.0f);  model = glm::translate(model, position); shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
    if (texture) { glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); } // texture
}
void createTexture2D(unsigned &var, int wrapping, int minfilter, int maxfilter, std::string path) {
    glActiveTexture(GL_TEXTURE0); glGenTextures(1, &var); glBindTexture(GL_TEXTURE_2D, var);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, maxfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);

    stbi_set_flip_vertically_on_load(true);  
    int width, height, channels; std::string fullpath = "../src/data/images/" + path; unsigned char *data = stbi_load(fullpath.c_str(), &width, &height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else { std::cout << "Failed to load texture with path '" << path << "'. Please check all filepaths." << std::endl; }
    stbi_image_free(data);
}