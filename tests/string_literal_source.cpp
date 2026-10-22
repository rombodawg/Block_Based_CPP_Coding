#include "core.hpp"
#include <iostream>

int main() {
    auto message = cpplab::make_block(cpplab::BlockType::PrintText);
    message.value = "first line\nquotes: \"hello\" path: C:\\temp";
    auto generated = cpplab::generate_cpp({message});
    if (!generated.errors.empty()) return 1;
    std::cout << generated.code;
    return 0;
}
