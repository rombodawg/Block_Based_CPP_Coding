#include "core.hpp"
#include <iostream>

int main() {
    std::string report;
    const bool passed = cpplab::self_test(report);
    std::cout << report;
    return passed ? 0 : 1;
}
