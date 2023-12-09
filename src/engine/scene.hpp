#ifndef SRC_ENGINE_SCENE_HPP
#define SRC_ENGINE_SCENE_HPP

    #include <string>
    #include <vector>
    #include <GLAD/glad.h>
    #include <GLAD/glm.hpp>
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
                glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
                decal(std::string name, float height, float width, shader &shader); ~decal() { glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO); }
                void render(shader &shader);
        };
        class model {
            private:
            public:
        };
        class scene {
            private:
                std::vector<shader> SHADERS; std::vector<model> MODELS; std::vector<decal> DECALS;
            public:
                scene(std::vector<shader> shaders, std::vector<model> models, std::vector<decal> decals); scene();
                void addShader(shader &value), addModel(model &value), addDecal(decal &value);
                void render();
        };
    }

#endif