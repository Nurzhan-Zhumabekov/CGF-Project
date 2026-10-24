#include "Shader.h"
#include "Utils.h"
#include <stdexcept>
#include <vector>

static GLuint compile(GLenum type, const std::string& src, const std::string& name) {
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len + 1);
        glGetShaderInfoLog(s, len, nullptr, log.data());
        throw std::runtime_error("shader " + name + ": " + log.data());
    }
    return s;
}

Shader::Shader(const std::string& vertPath, const std::string& fragPath) {
    GLuint vs = compile(GL_VERTEX_SHADER, res::readFile(vertPath), vertPath);
    GLuint fs = compile(GL_FRAGMENT_SHADER, res::readFile(fragPath), fragPath);
    id_ = glCreateProgram();
    glAttachShader(id_, vs);
    glAttachShader(id_, fs);
    glLinkProgram(id_);
    GLint ok = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(id_, sizeof log, nullptr, log);
        throw std::runtime_error(std::string("link ") + vertPath + ": " + log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() { glDeleteProgram(id_); }

GLint Shader::loc(const char* name) const {
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;
    return cache_[name] = glGetUniformLocation(id_, name);
}
