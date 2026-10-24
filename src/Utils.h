#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

// Resolves shaders/ and assets/ relative to the executable, falling back to the working directory.
namespace res {
inline std::filesystem::path& base() {
    static std::filesystem::path p;
    return p;
}
inline void init(const char* argv0) { base() = std::filesystem::absolute(argv0).parent_path(); }
inline std::string path(const std::string& rel) {
    std::filesystem::path p = base() / rel;
    return std::filesystem::exists(p) ? p.string() : rel;
}
inline std::string readFile(const std::string& rel) {
    std::ifstream f(path(rel));
    if (!f) throw std::runtime_error("cannot open " + rel);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
}  // namespace res
