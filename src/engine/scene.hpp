#ifndef SRC_ENGINE_SCENE_HPP
#define SRC_ENGINE_SCENE_HPP

    #include "camera.hpp"
    #include <string>
    #include <vector>
    #include <GLAD/glad.h>
    #include <assimp/scene.h>
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

        struct vertex { glm::vec3 position, normal, tangent, bitangent; glm::vec2 texture; int boneids[4]; float boneweights[4]; }; struct texture { unsigned id; std::string type, path; };
        class mesh {
            private:
               unsigned VAO, VBO, EBO;
            public:
                std::vector<vertex> vertices; std::vector<unsigned> indices; std::vector<texture> textures;
                mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures);
                void render(leto::shader &shader);
        };
        class model {
            private:
                void processNode(aiNode *node, const aiScene *scene, std::string directory); unsigned loadTexture(const char *path, const std::string &directory);
                std::vector<texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, std::string directory);
            public:
                std::vector<mesh> meshes; std::vector<texture> loadedtex; // stores all the textures loaded so far as to make sure nothing gets loaded more than once
                int shaderIndex = 1; // assume the "model" shader is at index 1

                model(std::string const &path); void render(shader &shader);
        };
        class scene {
            public:
                std::vector<model> models; std::vector<decal> decals;
                scene(std::vector<shader> shaders, std::vector<model> models, std::vector<decal> decals); scene();
                void render(std::vector<shader> shaders);
        };
    }

#endif