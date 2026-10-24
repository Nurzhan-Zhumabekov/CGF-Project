#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

// Compiles a vertex + fragment shader pair from files (relative to the executable).
class Shader {
public:
    Shader(const std::string& vertPath, const std::string& fragPath);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const { glUseProgram(id_); }
    void set(const char* name, int v) const { glUniform1i(loc(name), v); }
    void set(const char* name, float v) const { glUniform1f(loc(name), v); }
    void set(const char* name, const glm::vec3& v) const { glUniform3fv(loc(name), 1, &v[0]); }
    void set(const char* name, const glm::mat3& m) const { glUniformMatrix3fv(loc(name), 1, GL_FALSE, &m[0][0]); }
    void set(const char* name, const glm::mat4& m) const { glUniformMatrix4fv(loc(name), 1, GL_FALSE, &m[0][0]); }

private:
    GLint loc(const char* name) const;
    GLuint id_ = 0;
    mutable std::unordered_map<std::string, GLint> cache_;
};
