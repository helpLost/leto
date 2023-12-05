#ifndef SRC_ENGINE_SCENE_HPP
#define SRC_ENGINE_SCENE_HPP

    #include <GLAD/glad.h>
    namespace leto {
        class shader {
            private:
                unsigned PROGRAM;
            public:
                shader(std::string name); ~shader() { glDeleteProgram(PROGRAM); }
                unsigned ID() { return PROGRAM; } void use() { glUseProgram(PROGRAM); }

                void setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(PROGRAM, name.c_str()), (int)value); }
                void setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(PROGRAM, name.c_str()), value);  }
                void setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(PROGRAM, name.c_str()), value); }
        };
        class decal {
            private:
                unsigned INDICES[6] = {0, 1, 3, 1, 2, 3};
                unsigned VAO, VBO, EBO, TEXTURE;
            public:
                decal(std::string name, float height, float width); ~decal() { glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO); }
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