#ifndef SRC_ENGINE_SCENE_HPP
#define SRC_ENGINE_SCENE_HPP

    #include <ASSIMP/Importer.hpp>
    #include <GLAD/glad.h>
    namespace leto {
        class shader {
            private:
                unsigned programID;
            public:
                shader(std::string name);
                unsigned ID() { return programID; } void use() { glUseProgram(programID); }

                void setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value); }
                void setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(programID, name.c_str()), value);  }
                void setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(programID, name.c_str()), value); }
        };
        class decal {
            private:
                unsigned INDICES[6] = {0, 1, 3, 1, 2, 3};
                unsigned VAO, VBO, EBO, TEXTURE;
            public:
                decal(std::string name, float height, float width);
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
                scene();
                void render();
        };
    }

#endif