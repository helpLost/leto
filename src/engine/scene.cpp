#include "scene.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

void compileShader(unsigned &var, const char* raw, int type), linkShader(unsigned &id, unsigned &vertex, unsigned &fragment);
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