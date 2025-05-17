#include "OpticalApplication.hpp"
#include <iostream>

int main() {
    try {
        OpticalApplication app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "An unhandled C++ standard exception reached main: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unknown unhandled exception reached main." << std::endl;
        return 1;
    }
    return 0;
}