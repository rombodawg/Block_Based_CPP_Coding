#include "core.hpp"
#include <iostream>

int main() {
    using namespace cpplab;
    std::vector<Block> blocks;
    auto add = [&](BlockType type, int depth, const std::string& value = {}, const std::string& extra = {}) {
        Block b = make_block(type);
        b.depth = depth;
        if (!value.empty()) b.value = value;
        if (!extra.empty()) b.extra = extra;
        blocks.push_back(std::move(b));
    };
    add(BlockType::IncludeHeader, 0, "vector");
    add(BlockType::DeclareInt, 0, "increment", "0");
    add(BlockType::ReadInput, 0, "increment");
    add(BlockType::DeclareInt, 0, "total", "0");
    add(BlockType::DeclareIntList, 0, "numbers", "1, 2, 3");
    add(BlockType::ForLoop, 0, "int i = 0; i < static_cast<int>(numbers.size()); ++i");
    add(BlockType::If, 1, "i == 1");
    add(BlockType::Continue, 2);
    add(BlockType::SetVariable, 1, "total", "total + numbers[i]");
    add(BlockType::ShowVariable, 0, "total");
    add(BlockType::ShowVariable, 0, "increment");
    add(BlockType::DeclareInt, 0, "count", "0");
    add(BlockType::While, 0, "count < 5");
    add(BlockType::SetVariable, 1, "count", "count + 1");
    add(BlockType::If, 1, "count == 2");
    add(BlockType::Break, 2);
    add(BlockType::ShowVariable, 0, "count");

    auto generated = generate_cpp(blocks);
    if (!generated.errors.empty()) {
        for (const auto& error : generated.errors) std::cerr << error << '\n';
        return 1;
    }
    std::cout << generated.code;
    return 0;
}
