#include <exception>
#include <iostream>
#include "mjoelnir.hpp"

int main(void) {
    Mjoelnir app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
