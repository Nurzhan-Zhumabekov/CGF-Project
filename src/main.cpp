#include "App.h"
#include "Utils.h"
#include <cstdio>
#include <exception>

int main(int, char** argv) {
    try {
        res::init(argv[0]);
        App app;
        app.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal: %s\n", e.what());
        return 1;
    }
    return 0;
}
