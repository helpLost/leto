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
            int w, h, nrChannels; std::string path = "../src/data/images/sprites" + name;
            unsigned char *data = stbi_load(path.c_str(), &w, &h, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                std::cout << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data); stbi_set_flip_vertically_on_load(false);
    }
    void decal::render(shader &shader) {
        glBindTexture(GL_TEXTURE_2D, TEXTURE); glBindVertexArray(VAO); 
        glm::mat4 model = glm::mat4(1.0f);  model = glm::translate(model, position); shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    model::model(std::string const &path, bool gamma) : gammaCorrection(gamma) { loadModel(path); }
    void model::loadModel(std::string const &path)
    {
        stbi_set_flip_vertically_on_load(true);
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
        stbi_set_flip_vertically_on_load(false);
    }
    unsigned int model::TextureFromFile(const char *path, const std::string &directory, bool gamma)
    {
        std::string filename = std::string(path);
        filename = directory + '/' + filename;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    scene::scene(std::vector<shader> shaders, std::vector<model> models, std::vector<decal> decals) {}
    scene::scene() {}
    void scene::render(std::vector<shader> shaders) {
        // ofc there's a better way of doing this but I have other things to work on
        for(int i = 0; i < models.size(); i++) { models[i].render(shaders[models[i].shaderIndex]); }
        for(int i = 0; i < decals.size(); i++) { decals[i].render(shaders[decals[i].shaderIndex]); }
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