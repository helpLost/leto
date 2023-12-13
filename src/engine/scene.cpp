#include "scene.hpp"
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

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

    mesh::mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures) 
        :vertices(vertices), indices(indices), textures(textures) 
    {
        glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
        glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO); glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // positions, normals | texture coords | tangent, bitangent | bone ids, and bone weights
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0); glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texture));
        glEnableVertexAttribArray(3); glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, tangent)); glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, bitangent));
        glEnableVertexAttribArray(5); glVertexAttribIPointer(5, 4, GL_INT, sizeof(vertex), (void*)offsetof(vertex, boneids)); glEnableVertexAttribArray(6); glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, boneweights));

        glBindVertexArray(0);
    }
    void mesh::render(leto::shader &shader) 
    {
        unsigned diffuse = 1, specular = 1, normal = 1, height = 1;
        for(unsigned i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number; std::string name = textures[i].type;
            if(name == "texture_diffuse") { number = std::to_string(diffuse++); } else if(name == "texture_specular") { number = std::to_string(specular++); } else if(name == "texture_normal") { number = std::to_string(normal++); } else if(name == "texture_height") { number = std::to_string(height++); }
            shader.setInt((name + number).c_str(), i); glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        
        glBindVertexArray(VAO); glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); glActiveTexture(GL_TEXTURE0);
    }

    model::model(std::string const &path) {
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
        std::string directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene, directory);
        stbi_set_flip_vertically_on_load(false);
    }
    void model::render(shader &shader) {
        glEnable(GL_DEPTH_TEST);
        for(unsigned i = 0; i < meshes.size(); i++)
            meshes[i].render(shader);
        glDisable(GL_DEPTH_TEST);
    }
    void model::processNode(aiNode *node, const aiScene *scene, std::string directory) {
        for(unsigned i = 0; i < node->mNumMeshes; i++) { 
            std::vector<vertex> vert; std::vector<unsigned> ind; std::vector<texture> tex;
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 

            // loop through each of the mesh's vertices
            for(unsigned i = 0; i < mesh->mNumVertices; i++)
            {
                vertex vertex; glm::vec3 vector;
                vector.x = mesh->mVertices[i].x; vector.y = mesh->mVertices[i].y; vector.z = mesh->mVertices[i].z; vertex.position = vector;

                if(mesh->HasNormals()) { vector.x = mesh->mNormals[i].x; vector.y = mesh->mNormals[i].y; vector.z = mesh->mNormals[i].z; vertex.normal = vector; }
                if(mesh->mTextureCoords[0]) {
                    glm::vec2 vec; vec.x = mesh->mTextureCoords[0][i].x; vec.y = mesh->mTextureCoords[0][i].y; vertex.texture = vec;
                    vector.x = mesh->mTangents[i].x; vector.y = mesh->mTangents[i].y; vector.z = mesh->mTangents[i].z; vertex.tangent = vector;
                    vector.x = mesh->mBitangents[i].x; vector.y = mesh->mBitangents[i].y; vector.z = mesh->mBitangents[i].z; vertex.bitangent = vector;
                } else { vertex.texture = glm::vec2(0.0f, 0.0f); }
                vert.push_back(vertex);
            }
            // loop through the mesh's faces
            for(unsigned i = 0; i < mesh->mNumFaces; i++) { aiFace face = mesh->mFaces[i]; for(unsigned j = 0; j < face.mNumIndices; j++) { ind.push_back(face.mIndices[j]); } }

            // process materials, we assume a convention for sampler names in the shaders. Each diffuse texture should be named as 'texture_[TYPE][NUMBER]' where [type] is whatever kind of texture it is and [number] is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", directory); tex.insert(tex.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", directory); tex.insert(tex.end(), specularMaps.begin(), specularMaps.end());
            std::vector<texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", directory); tex.insert(tex.end(), normalMaps.begin(), normalMaps.end());
            std::vector<texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", directory); tex.insert(tex.end(), heightMaps.begin(), heightMaps.end());

            meshes.push_back(leto::mesh(vert, ind, tex)); 
        }
        for(unsigned i = 0; i < node->mNumChildren; i++) { processNode(node->mChildren[i], scene, directory); }
    }
    std::vector<texture> model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, std::string directory) {
        std::vector<texture> textures;
        for(unsigned i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str; mat->GetTexture(type, i, &str);
            // check if texture was loaded before and skip it if it was
            bool skip = false; for(unsigned j = 0; j < loadedtex.size(); j++) { if(std::strcmp(loadedtex[j].path.data(), str.C_Str()) == 0) { textures.push_back(loadedtex[j]); skip = true; break; } }
            if(!skip) {
                texture tex; tex.id = loadTexture(str.C_Str(), directory); tex.type = typeName; tex.path = str.C_Str();
                textures.push_back(tex); loadedtex.push_back(tex);
            }
        }
        return textures;
    }
    unsigned model::loadTexture(const char *path, const std::string &directory) {
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