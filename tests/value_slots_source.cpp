#include "core.hpp"
#include <iostream>

int main() {
    using cpplab::Block;
    using cpplab::BlockType;
    std::vector<Block> blocks;
    auto put = [&](BlockType type, int x, int y, const std::string& value = {}) {
        Block b = cpplab::make_block(type);
        b.gridX = x; b.gridY = y;
        if (!value.empty()) b.value = value;
        blocks.push_back(b);
    };

    // #include <iostream>
    put(BlockType::TokHash, 0, 0); put(BlockType::TokInclude, 1, 0); put(BlockType::TokLess, 2, 0);
    put(BlockType::TokIdentifier, 3, 0, "iostream"); put(BlockType::TokGreater, 4, 0);
    // int main() {
    put(BlockType::TokInt, 0, 1); put(BlockType::TokMain, 1, 1); put(BlockType::TokLParen, 2, 1);
    put(BlockType::TokRParen, 3, 1); put(BlockType::TokLBrace, 4, 1);
    // Type name + Name + = + Number + ;
    put(BlockType::TokTypeName, 0, 2, "int"); put(BlockType::TokIdentifier, 1, 2, "score");
    put(BlockType::TokAssign, 2, 2); put(BlockType::TokNumber, 3, 2, "21"); put(BlockType::TokSemicolon, 4, 2);
    // std::cout << Expression << Character(newline) ;
    put(BlockType::TokStd, 0, 3); put(BlockType::TokScope, 1, 3); put(BlockType::TokCout, 2, 3);
    put(BlockType::TokShiftLeft, 3, 3); put(BlockType::TokExpression, 4, 3, "score * 2 + 1");
    put(BlockType::TokShiftLeft, 5, 3); put(BlockType::TokCharLiteral, 6, 3, "\\n"); put(BlockType::TokSemicolon, 7, 3);
    // std::cout << Text << Character(newline) ;
    put(BlockType::TokStd, 0, 4); put(BlockType::TokScope, 1, 4); put(BlockType::TokCout, 2, 4);
    put(BlockType::TokShiftLeft, 3, 4); put(BlockType::TokStringLiteral, 4, 4, "Text value is reusable!");
    put(BlockType::TokShiftLeft, 5, 4); put(BlockType::TokCharLiteral, 6, 4, "\\n"); put(BlockType::TokSemicolon, 7, 4);
    put(BlockType::TokReturn, 0, 5); put(BlockType::TokNumber, 1, 5, "0"); put(BlockType::TokSemicolon, 2, 5);
    put(BlockType::TokRBrace, 0, 6);

    const auto result = cpplab::generate_cpp(blocks);
    if (!result.errors.empty()) {
        for (const auto& error : result.errors) std::cerr << error << '\n';
        return 1;
    }
    std::cout << result.code;
    return 0;
}
