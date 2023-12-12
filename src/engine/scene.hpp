#ifndef SRC_ENGINE_SCENE_HPP
#define SRC_ENGINE_SCENE_HPP

    #include <string>
    #include <iostream>
    #include <vector>
    #include <GLAD/glad.h>
    #include <GLAD/glm.hpp>
    #include <assimp/Importer.hpp>
    #include <assimp/scene.h>
    #include <assimp/postprocess.h>
    namespace leto {
        class shader {
            private:
                unsigned PROGRAM;
            public:
                shader(std::string name); ~shader() { glDeleteProgram(PROGRAM); }
                unsigned ID() { return PROGRAM; } void use() { glUseProgram(PROGRAM); }

                void setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(PROGRAM, name.c_str()), (int)value); } void setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(PROGRAM, name.c_str()), value); } void setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(PROGRAM, name.c_str()), value); }
                void setVec2(const std::string &name, const glm::vec2 &value) const { glUniform2fv(glGetUniformLocation(PROGRAM, name.c_str()), 1, &value[0]); } void setVec2(const std::string &name, float x, float y) const { glUniform2f(glGetUniformLocation(PROGRAM, name.c_str()), x, y); } void setVec3(const std::string &name, const glm::vec3 &value) const { glUniform3fv(glGetUniformLocation(PROGRAM, name.c_str()), 1, &value[0]); } void setVec3(const std::string &name, float x, float y, float z) const { glUniform3f(glGetUniformLocation(PROGRAM, name.c_str()), x, y, z); } void setVec4(const std::string &name, const glm::vec4 &value) const { glUniform4fv(glGetUniformLocation(PROGRAM, name.c_str()), 1, &value[0]); } void setVec4(const std::string &name, float x, float y, float z, float w) const { glUniform4f(glGetUniformLocation(PROGRAM, name.c_str()), x, y, z, w); }
                void setMat2(const std::string &name, const glm::mat2 &mat) const { glUniformMatrix2fv(glGetUniformLocation(PROGRAM, name.c_str()), 1, GL_FALSE, &mat[0][0]); } void setMat3(const std::string &name, const glm::mat3 &mat) const { glUniformMatrix3fv(glGetUniformLocation(PROGRAM, name.c_str()), 1, GL_FALSE, &mat[0][0]); } void setMat4(const std::string &name, const glm::mat4 &mat) const { glUniformMatrix4fv(glGetUniformLocation(PROGRAM, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
        };
        class decal {
            private:
                unsigned INDICES[6] = {0, 1, 3, 1, 2, 3};
                unsigned VAO, VBO, EBO, TEXTURE;
            public:
                int shaderIndex = 0; // assume the "decal" shader is at index 0
                glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
                decal(std::string name, float height, float width, shader &shader); ~decal() { glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO); }
                void render(shader &shader);
        };
        struct Vertex {
                glm::vec3 Position, Normal, Tangent, Bitangent; glm::vec2 TexCoords;
                int BoneIDs[4]; float BoneWeights[4];
        };
        struct Texture {
            unsigned int id;
            std::string type, path;
        };
        class Mesh {
            public:
                // mesh Data
                std::vector<Vertex>       vertices;
                std::vector<unsigned int> indices;
                std::vector<Texture>      textures;
                unsigned int VAO;

                // constructor
                Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
                {
                    this->vertices = vertices;
                    this->indices = indices;
                    this->textures = textures;

                    // now that we have all the required data, set the vertex buffers and its attribute pointers.
                    setupMesh();
                }

                // render the mesh
                void Draw(leto::shader &shader) 
                {
                    // bind appropriate textures
                    unsigned int diffuseNr  = 1;
                    unsigned int specularNr = 1;
                    unsigned int normalNr   = 1;
                    unsigned int heightNr   = 1;
                    for(unsigned int i = 0; i < textures.size(); i++)
                    {
                        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                        // retrieve texture number (the N in diffuse_textureN)
                        std::string number;
                        std::string name = textures[i].type;
                        if(name == "texture_diffuse")
                            number = std::to_string(diffuseNr++);
                        else if(name == "texture_specular")
                            number = std::to_string(specularNr++); // transfer unsigned int to string
                        else if(name == "texture_normal")
                            number = std::to_string(normalNr++); // transfer unsigned int to string
                        else if(name == "texture_height")
                            number = std::to_string(heightNr++); // transfer unsigned int to string

                        // now set the sampler to the correct texture unit
                        glUniform1i(glGetUniformLocation(shader.ID(), (name + number).c_str()), i);
                        // and finally bind the texture
                        glBindTexture(GL_TEXTURE_2D, textures[i].id);
                    }
                    
                    // draw mesh
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);

                    // always good practice to set everything back to defaults once configured.
                    glActiveTexture(GL_TEXTURE0);
                }

            private:
                // render data 
                unsigned int VBO, EBO;

                // initializes all the buffer objects/arrays
                void setupMesh()
                {
                    // create buffers/arrays
                    glGenVertexArrays(1, &VAO);
                    glGenBuffers(1, &VBO);
                    glGenBuffers(1, &EBO);

                    glBindVertexArray(VAO);
                    // load data into vertex buffers
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    // A great thing about structs is that their memory layout is sequential for all its items.
                    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
                    // again translates to 3/2 floats which translates to a byte array.
                    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

                    // set the vertex attribute pointers
                    // vertex Positions
                    glEnableVertexAttribArray(0);	
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
                    // vertex normals
                    glEnableVertexAttribArray(1);	
                    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
                    // vertex texture coords
                    glEnableVertexAttribArray(2);	
                    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
                    // vertex tangent
                    glEnableVertexAttribArray(3);
                    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
                    // vertex bitangent
                    glEnableVertexAttribArray(4);
                    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
                    // ids
                    glEnableVertexAttribArray(5);
                    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::BoneIDs));

                    // weights
                    glEnableVertexAttribArray(6);
                    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::BoneWeights));
                    glBindVertexArray(0);
                }
            };
        class model 
        {
        public:
            // model data 
            std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
            std::vector<Mesh>    meshes;
            std::string directory;
            int shaderIndex = 1; // assume the "model" shader is at index 1
            bool gammaCorrection;

            unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
            model(std::string const &path, bool gamma = false);
            void render(shader &shader)
            {
                glEnable(GL_DEPTH_TEST);
                for(unsigned int i = 0; i < meshes.size(); i++)
                    meshes[i].Draw(shader);
                glDisable(GL_DEPTH_TEST);
            }
            
        private:
            // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
            void loadModel(std::string const &path);

            // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
            void processNode(aiNode *node, const aiScene *scene)
            {
                // process each mesh located at the current node
                for(unsigned int i = 0; i < node->mNumMeshes; i++)
                {
                    // the node object only contains indices to index the actual objects in the scene. 
                    // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
                    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                    meshes.push_back(processMesh(mesh, scene));
                }
                // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
                for(unsigned int i = 0; i < node->mNumChildren; i++)
                {
                    processNode(node->mChildren[i], scene);
                }
            }

            Mesh processMesh(aiMesh *mesh, const aiScene *scene)
            {
                // data to fill
                std::vector<Vertex> vertices;
                std::vector<unsigned int> indices;
                std::vector<Texture> textures;

                // walk through each of the mesh's vertices
                for(unsigned int i = 0; i < mesh->mNumVertices; i++)
                {
                    Vertex vertex;
                    glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
                    // positions
                    vector.x = mesh->mVertices[i].x;
                    vector.y = mesh->mVertices[i].y;
                    vector.z = mesh->mVertices[i].z;
                    vertex.Position = vector;
                    // normals
                    if (mesh->HasNormals())
                    {
                        vector.x = mesh->mNormals[i].x;
                        vector.y = mesh->mNormals[i].y;
                        vector.z = mesh->mNormals[i].z;
                        vertex.Normal = vector;
                    }
                    // texture coordinates
                    if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
                    {
                        glm::vec2 vec;
                        // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                        vec.x = mesh->mTextureCoords[0][i].x; 
                        vec.y = mesh->mTextureCoords[0][i].y;
                        vertex.TexCoords = vec;
                        // tangent
                        vector.x = mesh->mTangents[i].x;
                        vector.y = mesh->mTangents[i].y;
                        vector.z = mesh->mTangents[i].z;
                        vertex.Tangent = vector;
                        // bitangent
                        vector.x = mesh->mBitangents[i].x;
                        vector.y = mesh->mBitangents[i].y;
                        vector.z = mesh->mBitangents[i].z;
                        vertex.Bitangent = vector;
                    }
                    else
                        vertex.TexCoords = glm::vec2(0.0f, 0.0f);

                    vertices.push_back(vertex);
                }
                // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
                for(unsigned int i = 0; i < mesh->mNumFaces; i++)
                {
                    aiFace face = mesh->mFaces[i];
                    // retrieve all indices of the face and store them in the indices vector
                    for(unsigned int j = 0; j < face.mNumIndices; j++)
                        indices.push_back(face.mIndices[j]);        
                }
                // process materials
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
                // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
                // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
                // Same applies to other texture as the following list summarizes:
                // diffuse: texture_diffuseN
                // specular: texture_specularN
                // normal: texture_normalN

                // 1. diffuse maps
                std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
                // 2. specular maps
                std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
                // 3. normal maps
                std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
                textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
                // 4. height maps
                std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
                textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
                
                // return a mesh object created from the extracted mesh data
                return Mesh(vertices, indices, textures);
            }

            // checks all material textures of a given type and loads the textures if they're not loaded yet.
            // the required info is returned as a Texture struct.
            std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
            {
                std::vector<Texture> textures;
                for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
                {
                    aiString str;
                    mat->GetTexture(type, i, &str);
                    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
                    bool skip = false;
                    for(unsigned int j = 0; j < textures_loaded.size(); j++)
                    {
                        if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                        {
                            textures.push_back(textures_loaded[j]);
                            skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                            break;
                        }
                    }
                    if(!skip)
                    {   // if texture hasn't been loaded already, load it
                        Texture texture;
                        texture.id = TextureFromFile(str.C_Str(), this->directory);
                        texture.type = typeName;
                        texture.path = str.C_Str();
                        textures.push_back(texture);
                        textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
                    }
                }
                return textures;
            }
        };

        class scene {
            public:
                std::vector<model> models; std::vector<decal> decals;
                scene(std::vector<shader> shaders, std::vector<model> models, std::vector<decal> decals); scene();
                void render(std::vector<shader> shaders);
        };
    }

#endif