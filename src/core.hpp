#pragma once

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace cpplab {

enum class BlockType {
    IncludeHeader,
    PrintText,
    PrintExpression,
    DeclareInt,
    DeclareDouble,
    DeclareText,
    DeclareBool,
    DeclareChar,
    DeclareIntList,
    ShowVariable,
    SetVariable,
    ReadInput,
    If,
    ElseIf,
    Else,
    Repeat,
    ForLoop,
    While,
    Break,
    Continue,
    Return,
    RawStatement,
    RawProgram,
    Comment,

    // Atomic C++ pieces: every Tok* block contributes exactly one token.
    TokInclude, TokInt, TokDouble, TokFloat, TokString, TokBool, TokChar, TokVoid, TokAuto, TokConst,
    TokIf, TokElse, TokFor, TokWhile, TokDo, TokSwitch, TokCase, TokDefault, TokBreak, TokContinue, TokReturn,
    TokClass, TokStruct, TokPublic, TokPrivate, TokProtected, TokTemplate, TokTypename, TokNamespace, TokUsing,
    TokTry, TokCatch, TokThrow, TokNew, TokDelete, TokStatic, TokVirtual, TokOverride, TokSizeof, TokMain,
    TokStd, TokCout, TokCin, TokEndl, TokVector, TokTrue, TokFalse, TokNullptr,
    TokIdentifier, TokTypeName, TokExpression, TokKeyword, TokNumber, TokStringLiteral, TokCharLiteral,
    TokLParen, TokRParen, TokLBrace, TokRBrace, TokLBracket, TokRBracket, TokSemicolon, TokComma, TokColon,
    TokQuestion, TokDot, TokScope, TokArrow, TokAssign, TokPlus, TokMinus, TokStar, TokSlash, TokPercent,
    TokHash, TokSymbol,
    TokEqual, TokNotEqual, TokLess, TokGreater, TokLessEqual, TokGreaterEqual, TokAnd, TokOr, TokNot,
    TokAmp, TokPipe, TokCaret, TokTilde, TokShiftLeft, TokShiftRight, TokIncrement, TokDecrement,
    TokPlusAssign, TokMinusAssign, TokMultiplyAssign, TokDivideAssign, TokModAssign,

    // One-purpose learning actions. These are deliberately separate from syntax tokens.
    ActionSetNumber, ActionSetText, ActionPrintText, ActionShowValue, ActionReadNumber, ActionChangeNumber, ActionComment
};

struct Block {
    BlockType type = BlockType::PrintText;
    std::string value;
    std::string extra;
    int depth = 0;
    int gridX = 0;
    int gridY = 0;
};

struct BlockInfo {
    BlockType type;
    const char* name;
    const char* shortDescription;
    const char* longDescription;
    const char* example;
    const char* warning;
};

inline const std::vector<BlockInfo>& block_catalog() {
    static const std::vector<BlockInfo> entries = [] {
        std::vector<BlockInfo> v;
        auto add = [&](BlockType t, const char* name, const char* label, const char* shortTip,
                       const char* longTip, const char* example, const char* warning) {
            (void)label; // The palette label is returned by tile_label(type); kept beside metadata for auditability.
            v.push_back({t, name, shortTip, longTip, example, warning});
        };
        // Each Tok* entry is one piece of C++ syntax, not a whole statement.
        add(BlockType::TokInclude, "include", "incl", "Names a header to include.",
            "This tile contributes only the word include. Place a separate # tile immediately before it, then add <, the header name, and > as individual tiles.",
            "# include < iostream >", "The # and include tiles must be next to each other at the start of a source line.");
        add(BlockType::TokInt, "int", "int", "Whole-number type.", "The keyword int means a variable stores a whole number. It is only the type word; add a name, =, a starting number, and ; as separate tiles.", "int score = 0;", "A declaration needs a variable name and usually ends with a semicolon.");
        add(BlockType::TokDouble, "double", "dbl", "Decimal-number type.", "The keyword double stores numbers that can have a fractional part. This block contributes only the word double.", "double height = 1.75;", "Decimal values can have tiny rounding differences.");
        add(BlockType::TokFloat, "float", "flt", "Another decimal type.", "The keyword float is a decimal-number type that usually uses less precision than double.", "float speed = 2.5f;", "A float literal often uses an f suffix, such as 2.5f.");
        add(BlockType::TokString, "string", "str", "Text type name.", "Use this token in std::string. It is just one type name; std and :: are separate pieces too.", "std::string name = \"Ada\";", "std::string requires the string header.");
        add(BlockType::TokBool, "bool", "bool", "True-or-false type.", "A bool variable can contain true or false. This tile contributes only the type keyword.", "bool ready = true;", "true and false are not surrounded by quotes.");
        add(BlockType::TokChar, "char", "char", "Single-character type.", "The char type holds one character, usually written with single quotes.", "char grade = 'A';", "Use std::string for words or sentences.");
        add(BlockType::TokVoid, "void", "void", "Means no return value.", "Use void for a function that does not send a value back to its caller.", "void sayHi() { ... }", "A void function should not return a value.");
        add(BlockType::TokAuto, "auto", "auto", "Let C++ infer a type.", "auto asks C++ to work out a variable's type from the value assigned to it.", "auto answer = 42;", "C++ still needs enough information to determine the type.");
        add(BlockType::TokConst, "const", "const", "Marks a value as unchangeable.", "const says a variable cannot be assigned a new value after initialization.", "const int limit = 10;", "A const variable needs an initial value.");
        add(BlockType::TokIf, "if", "if", "Starts a condition.", "This tile is only the word if. Add (, a test such as score < 5, ), {, the body, and } as separate tiles.", "if (score < 5) { ... }", "if needs a condition in parentheses.");
        add(BlockType::TokElse, "else", "else", "Runs when the test is false.", "else begins an alternative branch. It is a separate word; use } before it and { after it as separate tiles.", "} else {", "else must match a previous if and cannot have its own condition.");
        add(BlockType::TokFor, "for", "for", "Starts a counted loop.", "for repeats code using a start action, a test, and an update. Each punctuation mark is a separate tile.", "for (int i = 0; i < 5; ++i) { ... }", "The loop header has two semicolons inside the parentheses.");
        add(BlockType::TokWhile, "while", "while", "Repeats while a test is true.", "while checks its condition before each repetition. You build the parentheses and body with separate blocks.", "while (score < 5) { ... }", "Make sure the condition can eventually become false.");
        add(BlockType::TokDo, "do", "do", "Starts a do-while loop.", "A do-while loop runs its body at least once, then checks its condition at the end.", "do { ... } while (ready);", "A do-while loop ends with a semicolon after the condition.");
        add(BlockType::TokSwitch, "switch", "sw", "Chooses among matching cases.", "switch compares one value with case labels. Each keyword and punctuation mark is a separate tile.", "switch (choice) { case 1: break; }", "Remember break when you do not want a case to continue into the next case.");
        add(BlockType::TokCase, "case", "case", "Starts one switch option.", "case marks a value to match inside a switch block.", "case 1:", "A case normally ends with a colon.");
        add(BlockType::TokDefault, "default", "def", "Fallback switch option.", "default runs when no case value matches, if execution reaches it.", "default:", "A switch should not contain multiple default labels.");
        add(BlockType::TokBreak, "break", "brk", "Exits a loop or switch.", "break immediately leaves the nearest loop or switch statement.", "break;", "break must be inside a loop or switch.");
        add(BlockType::TokContinue, "continue", "next", "Skips to the next loop pass.", "continue stops the current pass of the nearest loop and starts its next check/update.", "continue;", "continue must be inside a loop.");
        add(BlockType::TokReturn, "return", "ret", "Leaves a function.", "return ends the current function, optionally providing a value to the caller.", "return 0;", "The returned value must match the function's return type.");
        add(BlockType::TokClass, "class", "class", "Begins a class definition.", "A class groups data and functions into a new type. The class body is built using braces and semicolons.", "class Player { ... };", "A class definition usually ends with }; including the semicolon.");
        add(BlockType::TokStruct, "struct", "struct", "Begins a struct definition.", "A struct defines a type whose members are public by default.", "struct Point { int x; };", "Remember the semicolon after the closing brace.");
        add(BlockType::TokPublic, "public", "pub", "Opens public access.", "In a class or struct, public makes the following members accessible from outside, until another access label.", "public:", "Use a colon after this access keyword.");
        add(BlockType::TokPrivate, "private", "priv", "Opens private access.", "In a class, private hides following members from ordinary outside access.", "private:", "Class members are private by default.");
        add(BlockType::TokProtected, "protected", "prot", "Opens protected access.", "protected members can be used inside the class and its derived classes.", "protected:", "Use this inside a class or struct body.");
        add(BlockType::TokTemplate, "template", "tmpl", "Begins a template declaration.", "Templates let functions and classes work with types or values supplied later.", "template <typename T>", "A template declaration must be followed by a matching function or type definition.");
        add(BlockType::TokTypename, "typename", "type", "Names a template type parameter.", "typename is commonly used when declaring a type parameter in a template.", "template <typename T>", "The template parameter must be given a name.");
        add(BlockType::TokNamespace, "namespace", "ns", "Groups names together.", "A namespace groups related names and helps prevent name conflicts.", "namespace tools { ... }", "A namespace definition uses braces.");
        add(BlockType::TokUsing, "using", "using", "Introduces a name or alias.", "using can create a type alias or bring a name into a scope.", "using Count = int;", "Avoid using namespace std in large projects; explicit std:: names are clearer for beginners.");
        add(BlockType::TokTry, "try", "try", "Starts error-handling code.", "try marks code that may throw an exception and can be followed by one or more catch blocks.", "try { ... } catch (...) { ... }", "A try block needs at least one matching catch block.");
        add(BlockType::TokCatch, "catch", "catch", "Handles a thrown exception.", "catch introduces code that handles an exception from a matching try block.", "catch (const std::exception& e) { ... }", "catch must follow a try or another catch handler.");
        add(BlockType::TokThrow, "throw", "throw", "Signals an exception.", "throw reports an error by sending an exception to a matching catch block, or out of the current function.", "throw std::runtime_error(\"Oops\");", "The exception type and any required header must be available.");
        add(BlockType::TokNew, "new", "new", "Creates a dynamic object.", "new requests storage for an object and returns a pointer to it. Modern C++ often prefers smart pointers or containers.", "auto p = new int(3);", "Raw new requires careful ownership; do not forget the matching delete when appropriate.");
        add(BlockType::TokDelete, "delete", "del", "Releases a raw new object.", "delete releases one object created using raw new.", "delete p;", "Only delete a pointer from a matching new; never delete the same object twice.");
        add(BlockType::TokStatic, "static", "stat", "Gives a declaration static storage or meaning.", "static has different uses in C++; one common use is a local variable that keeps its value between calls.", "static int count = 0;", "Its meaning depends on where it appears.");
        add(BlockType::TokVirtual, "virtual", "virt", "Enables polymorphic methods.", "virtual lets a derived class replace a base-class method when called through a base pointer or reference.", "virtual void draw();", "Virtual functions usually appear in a class declaration.");
        add(BlockType::TokOverride, "override", "over", "Marks an overridden method.", "override asks the compiler to verify that this method actually replaces a virtual base-class method.", "void draw() override;", "A misspelled or mismatched method will cause a compiler error, which is useful feedback.");
        add(BlockType::TokSizeof, "sizeof", "size", "Asks for a type or object's size.", "sizeof produces the size in bytes of a type or object.", "sizeof(int)", "The result is a size, not the number of items in a container.");
        add(BlockType::TokMain, "main", "main", "Program entry function name.", "The operating system starts a normal C++ console program by calling main. Build int main ( ) { ... } from separate tiles.", "int main() { return 0; }", "A program should have exactly one main function.");
        add(BlockType::TokStd, "std", "std", "Standard-library namespace.", "std contains much of the C++ standard library. The double-colon tile is separate.", "std::cout", "Spelling and capitalization matter.");
        add(BlockType::TokCout, "cout", "cout", "Writes to program output.", "std::cout is the standard output stream. Combine it with << and a value or text.", "std::cout << score;", "Use std::cout, or provide an appropriate using declaration.");
        add(BlockType::TokCin, "cin", "cin", "Reads program input.", "std::cin reads values from the program's input stream using >>.", "std::cin >> age;", "The destination variable must already exist and have a compatible type.");
        add(BlockType::TokEndl, "endl", "endl", "Ends a line and flushes output.", "std::endl writes a newline and flushes the output stream. A simple '\\n' is often enough.", "std::cout << score << std::endl;", "It belongs in std::endl, not by itself.");
        add(BlockType::TokVector, "vector", "vec", "A growable list type.", "std::vector stores a sequence of items and can grow as needed. Add #include <vector> above main.", "std::vector<int> scores;", "List positions start at zero. Include the vector header.");
        add(BlockType::TokIdentifier, "Name", "name", "A name you choose.", "Use this editable value tile for a variable, function, class, or your own identifier. Every copy can hold a different name; double-click the placed tile to edit it.", "score", "Names cannot contain spaces and cannot be C++ keywords.");
        add(BlockType::TokTypeName, "Type name", "tname", "A type such as int or MyType.", "Use this editable slot where C++ expects a type. Examples include int, double, std::string, std::vector<int>, or a class you created. For complex types, you can also assemble the type from smaller tiles.", "std::vector<int> scores;", "The type must exist and any needed library header must be included.");
        add(BlockType::TokExpression, "Expression", "expr", "A value or calculation you enter.", "This is a reusable expression slot, not a whole statement. Type a value, variable, function call, or calculation such as score + 1. Use smaller operator/name/number tiles when you want to build the expression piece by piece.", "score + 1", "Do not put a whole statement or trailing semicolon here.");
        add(BlockType::TokKeyword, "C++ keyword", "kw", "A reusable language-word slot.", "Use this editable tile for C++ keywords that do not have their own tile, including constexpr, extern, friend, unsigned, long, operator, requires, concept, co_await, and newer language words. Each copy can contain a different keyword.", "constexpr", "Enter one keyword only; this is not for a variable name.");
        add(BlockType::TokNumber, "Number", "num", "A number you can edit.", "This is one reusable number tile. Double-click it and type any numeric literal you need, such as 0, 42, -3, 3.14, or 0xFF; you do not need a separate block for every possible number.", "42", "Do not add quotation marks unless you want text. A minus sign can also be its own tile.");
        add(BlockType::TokStringLiteral, "Text", "txt", "Text that becomes a quoted value.", "Type any words in this reusable Text tile. The editor adds safe double quotes for C++, so you do not need separate tiles for every phrase.", "\"Hello!\"", "Text values use double quotes; the editor adds them for you.");
        add(BlockType::TokCharLiteral, "Character", "charv", "One character as a value.", "Enter one character, such as A or ?, in this reusable value tile. The editor surrounds it with C++ single quotes.", "'A'", "Use Text for a word or sentence; Character is for one character.");
        add(BlockType::TokTrue, "true", "true", "The true boolean value.", "true is the boolean value meaning yes or on.", "bool ready = true;", "Do not put quotes around true.");
        add(BlockType::TokFalse, "false", "false", "The false boolean value.", "false is the boolean value meaning no or off.", "bool ready = false;", "Do not put quotes around false.");
        add(BlockType::TokNullptr, "nullptr", "null", "A pointer to nothing.", "nullptr represents a null pointer: it does not point to an object.", "int* p = nullptr;", "It is not the same as the number zero in every context.");
        add(BlockType::TokHash, "#", "#", "Starts a preprocessor line.", "The # symbol starts directives such as #define, #ifdef, and #pragma. Place it first on the grid row, then add the directive word as another tile.", "#define LIMIT 10", "Preprocessor directives must begin a source line.");
        add(BlockType::TokSymbol, "Symbol", "sym", "An extra one-piece symbol.", "Use this editable tile for a C++ operator or punctuator without a dedicated tile, such as &=, <<=, ..., .* or ->*. Enter exactly one C++ symbol token.", "score <<= 1;", "Enter one symbol token only. Use the dedicated tiles for common operators to get their usual spacing automatically.");
        // Punctuation and operators each have their own tile.
        add(BlockType::TokLParen, "(", "(", "Opens parentheses.", "This tile starts a parenthesized group. It is commonly used after if, while, for, or a function name.", "if (ready)", "Every opening parenthesis needs a matching closing parenthesis.");
        add(BlockType::TokRParen, ")", ")", "Closes parentheses.", "This tile ends the parenthesized group started by (.", "if (ready)", "Every closing parenthesis should match an opening one.");
        add(BlockType::TokLBrace, "{", "{", "Opens a code group.", "An opening brace begins a block of code, such as a function, class, loop, or if body. The editor indents later lines inside it.", "int main() {", "Every opening brace needs a matching closing brace.");
        add(BlockType::TokRBrace, "}", "}", "Closes a code group.", "A closing brace ends the current function, class, loop, or conditional body.", "}", "Match it to an earlier opening brace.");
        add(BlockType::TokLBracket, "[", "[", "Opens an index or capture.", "Square brackets access an array/list item or introduce a lambda capture list.", "scores[0]", "Match it with a closing bracket.");
        add(BlockType::TokRBracket, "]", "]", "Closes a bracket group.", "Square brackets close an index or capture list.", "scores[0]", "Match it with an opening bracket.");
        add(BlockType::TokSemicolon, ";", ";", "Ends a statement.", "Many C++ statements and declarations end with a semicolon. This punctuation is its own tile.", "int score = 0;", "A missing semicolon is a common beginner compiler error.");
        add(BlockType::TokComma, ",", ",", "Separates items.", "A comma separates function arguments, list items, and some declarations.", "max(a, b)", "Do not replace commas with semicolons inside an argument list.");
        add(BlockType::TokColon, ":", ":", "Punctuation for labels and cases.", "A colon follows case, default, public, private, and other labels; it also appears in conditional expressions and range-based loops.", "case 1:", "Its exact meaning depends on the surrounding syntax.");
        add(BlockType::TokQuestion, "?", "qst", "Starts a short if/else expression.", "The conditional operator chooses one of two values based on a condition. It is often called the ternary operator.", "ready ? 1 : 0", "A ? expression also needs a matching colon.");
        add(BlockType::TokDot, ".", ".", "Accesses a member.", "The dot accesses a member of an object, such as a field or method.", "player.score", "Use -> instead when accessing through a pointer.");
        add(BlockType::TokScope, "::", "::", "Accesses a named scope.", "The scope operator selects a name inside a namespace or class.", "std::cout", "Type two colons with no space between them.");
        add(BlockType::TokArrow, "->", "->", "Accesses through a pointer.", "The arrow operator accesses a member through a pointer.", "pointer->value", "The expression on the left must support pointer member access.");
        add(BlockType::TokAssign, "=", "=", "Stores a value.", "A single equals sign assigns the right-hand value to the left-hand variable.", "score = 10;", "Use == to compare for equality.");
        add(BlockType::TokPlus, "+", "+", "Adds or indicates a positive value.", "The plus operator adds values. It can also mean unary positive in some expressions.", "score + 1", "The values must support addition.");
        add(BlockType::TokMinus, "-", "-", "Subtracts or negates.", "The minus operator subtracts one value from another or makes a value negative.", "score - 1", "The values must support subtraction.");
        add(BlockType::TokStar, "*", "*", "Multiplies or works with pointers.", "The star multiplies numbers in an expression; in declarations it can also indicate a pointer type.", "2 * 3", "The meaning depends on where it appears.");
        add(BlockType::TokSlash, "/", "/", "Divides one value by another.", "The slash divides the left value by the right value.", "10 / 2", "Integer divided by integer discards the fractional part.");
        add(BlockType::TokPercent, "%", "%", "Gets a remainder.", "The remainder operator gives what is left after integer division.", "10 % 3", "Both operands should be integer types.");
        add(BlockType::TokEqual, "==", "==", "Tests whether values match.", "The equality operator is true when its left and right values are equal.", "score == 5", "Do not confuse == with =, which stores a value.");
        add(BlockType::TokNotEqual, "!=", "!=", "Tests whether values differ.", "The not-equal operator is true when the two values are different.", "score != 0", "Both sides must be comparable.");
        add(BlockType::TokLess, "<", "<", "Tests whether left is smaller.", "The less-than operator compares values, except immediately after #include where it opens a header name.", "score < 5", "The same symbol is also used in templates and include headers.");
        add(BlockType::TokGreater, ">", ">", "Tests whether left is larger.", "The greater-than operator compares values, except when closing an angle-bracket header or template.", "score > 5", "Two > symbols together form the right-shift operator.");
        add(BlockType::TokLessEqual, "<=", "<=", "Tests smaller than or equal.", "True when the left value is less than or equal to the right value.", "score <= 5", "Both sides must be comparable.");
        add(BlockType::TokGreaterEqual, ">=", ">=", "Tests larger than or equal.", "True when the left value is greater than or equal to the right value.", "score >= 5", "Both sides must be comparable.");
        add(BlockType::TokAnd, "&&", "&&", "Requires both tests true.", "Logical AND is true only when both conditions are true.", "ready && score > 0", "Both sides should be conditions or values convertible to true/false.");
        add(BlockType::TokOr, "||", "||", "Needs either test true.", "Logical OR is true when at least one condition is true.", "ready || overrideEnabled", "Use && when both conditions must be true.");
        add(BlockType::TokNot, "!", "!", "Reverses true and false.", "Logical NOT changes true to false and false to true.", "!ready", "A single ! is different from !=.");
        add(BlockType::TokAmp, "&", "&", "Reference or bitwise AND.", "The ampersand can create a reference in a declaration or perform bitwise AND between integers.", "int& alias = score;", "Its meaning depends on its position.");
        add(BlockType::TokPipe, "|", "|", "Bitwise OR.", "Bitwise OR combines bits in integer values.", "flags | mask", "For true/false logic, use || instead.");
        add(BlockType::TokCaret, "^", "^", "Bitwise exclusive OR.", "Bitwise XOR sets a bit when the matching input bits differ.", "flags ^ mask", "This is not the exponent operator in C++.");
        add(BlockType::TokTilde, "~", "~", "Bitwise NOT.", "Bitwise NOT flips each bit in an integer value.", "~flags", "It acts on bits, not on true/false logic.");
        add(BlockType::TokShiftLeft, "<<", "<<", "Prints or shifts bits left.", "With std::cout, << sends a value to the output stream. For integers, it can also shift bits left.", "std::cout << score;", "Meaning depends on the types on each side.");
        add(BlockType::TokShiftRight, ">>", ">>", "Reads input or shifts bits right.", "With std::cin, >> reads a value. For integers, it can also shift bits right.", "std::cin >> score;", "Meaning depends on the types on each side.");
        add(BlockType::TokIncrement, "++", "++", "Adds one to a value.", "The increment operator adds one to a changeable variable.", "++score", "The variable must be writable.");
        add(BlockType::TokDecrement, "--", "--", "Subtracts one from a value.", "The decrement operator subtracts one from a changeable variable.", "--score", "The variable must be writable.");
        add(BlockType::TokPlusAssign, "+=", "+=", "Adds and stores.", "Adds the right-hand value to the left-hand variable and stores the result.", "score += 2;", "The left side must be writable.");
        add(BlockType::TokMinusAssign, "-=", "-=", "Subtracts and stores.", "Subtracts the right-hand value from the left-hand variable and stores the result.", "score -= 2;", "The left side must be writable.");
        add(BlockType::TokMultiplyAssign, "*=", "*=", "Multiplies and stores.", "Multiplies the left-hand variable by the right-hand value and stores the result.", "score *= 2;", "The left side must be writable.");
        add(BlockType::TokDivideAssign, "/=", "/=", "Divides and stores.", "Divides the left-hand variable by the right-hand value and stores the result.", "score /= 2;", "Do not divide by zero.");
        add(BlockType::TokModAssign, "%=", "%=", "Stores the remainder.", "Calculates the remainder and stores it in the left-hand variable.", "score %= 3;", "Both values should be integers and the right side cannot be zero.");
        // Friendly one-purpose beginner actions. These are named semantic actions (not syntax tokens);
        // the separate Name/Type/Number/Text/Expression and punctuation tiles expose the syntax underneath.
        add(BlockType::ActionSetNumber, "Set number", "set #", "Create a named number.", "This action creates one integer variable. It is a single beginner action, unlike the tiny syntax tiles above.", "int score = 0;", "Choose a name without spaces. Create it before you use it.");
        add(BlockType::ActionSetText, "Set text", "set txt", "Create a text value.", "Creates one std::string variable and safely puts quotation marks around the text you enter.", "std::string message = \"Hello\";", "A text value and a number are different types.");
        add(BlockType::ActionPrintText, "Print text", "print", "Show words on screen.", "Prints the message you enter. This is one clear action; it does not hide other control-flow syntax.", "std::cout << \"Hello\" << '\\n';", "Use Show value to print a variable or calculation instead.");
        add(BlockType::ActionShowValue, "Show value", "show", "Display a variable's value.", "Prints the current value of the named variable or expression.", "std::cout << score << '\\n';", "A variable name has no quotation marks around it.");
        add(BlockType::ActionReadNumber, "Read number", "read", "Read a number from input.", "Reads one numeric value into an existing integer variable from standard input.", "std::cin >> score;", "Create the destination variable first.");
        add(BlockType::ActionChangeNumber, "Change number", "change", "Update a number.", "Adds the amount you choose to an existing integer variable. This is useful for scores and loop counters.", "score += 1;", "The variable must already have been created.");
        add(BlockType::ActionComment, "Comment", "note", "Leave a note in the code.", "Adds a // comment line for people reading the program. It does not change how the program runs.", "// Try changing score", "A comment is not displayed to the person running the program.");

        // Put reusable fields first so new learners can find them without scrolling through advanced C++.
        // The next group is beginner actions; all remaining individual keywords/operators/punctuation follow.
        auto palette_rank = [](BlockType t) {
            switch (t) {
                case BlockType::TokIdentifier: case BlockType::TokTypeName: case BlockType::TokExpression:
                case BlockType::TokKeyword: case BlockType::TokNumber: case BlockType::TokStringLiteral:
                case BlockType::TokCharLiteral: case BlockType::TokSymbol: return 0;
                case BlockType::ActionSetNumber: case BlockType::ActionSetText: case BlockType::ActionPrintText:
                case BlockType::ActionShowValue: case BlockType::ActionReadNumber: case BlockType::ActionChangeNumber:
                case BlockType::ActionComment: return 1;
                case BlockType::TokHash: case BlockType::TokInclude: case BlockType::TokInt: case BlockType::TokDouble:
                case BlockType::TokString: case BlockType::TokBool: case BlockType::TokChar: case BlockType::TokAuto:
                case BlockType::TokIf: case BlockType::TokElse: case BlockType::TokFor: case BlockType::TokWhile:
                case BlockType::TokReturn: case BlockType::TokMain: case BlockType::TokStd: case BlockType::TokCout:
                case BlockType::TokCin: case BlockType::TokTrue: case BlockType::TokFalse: case BlockType::TokLParen:
                case BlockType::TokRParen: case BlockType::TokLBrace: case BlockType::TokRBrace: case BlockType::TokSemicolon:
                case BlockType::TokAssign: case BlockType::TokPlus: case BlockType::TokEqual: case BlockType::TokLess:
                case BlockType::TokGreater: case BlockType::TokShiftLeft: return 2;
                default: return 3;
            }
        };
        std::stable_sort(v.begin(), v.end(), [&](const BlockInfo& a, const BlockInfo& b) {
            return palette_rank(a.type) < palette_rank(b.type);
        });
        return v;
    }();
    return entries;
}

inline const BlockInfo* block_info(BlockType type) {
    for (const auto& info : block_catalog()) if (info.type == type) return &info;
    return nullptr;
}

inline const char* block_name(BlockType type) {
    const auto* info = block_info(type);
    return info ? info->name : "Block";
}

inline const char* tile_label(BlockType type) {
    // Compact labels are intentionally short enough to fit on square, one-cell tiles.
    switch (type) {
        case BlockType::IncludeHeader: return "#inc";
        case BlockType::PrintText: return "print";
        case BlockType::PrintExpression: return "calc";
        case BlockType::DeclareInt: return "int";
        case BlockType::DeclareDouble: return "real";
        case BlockType::DeclareText: return "text";
        case BlockType::DeclareBool: return "bool";
        case BlockType::DeclareChar: return "char";
        case BlockType::DeclareIntList: return "list";
        case BlockType::ShowVariable: return "show";
        case BlockType::SetVariable: return "set";
        case BlockType::ReadInput: return "input";
        case BlockType::If: return "if";
        case BlockType::ElseIf: return "elif";
        case BlockType::Else: return "else";
        case BlockType::Repeat: return "rep";
        case BlockType::ForLoop: return "for";
        case BlockType::While: return "while";
        case BlockType::Break: return "break";
        case BlockType::Continue: return "next";
        case BlockType::Return: return "ret";
        case BlockType::RawStatement: return "line";
        case BlockType::RawProgram: return "file";
        case BlockType::Comment: return "note";
        case BlockType::TokInclude: return "inc"; case BlockType::TokInt: return "int"; case BlockType::TokDouble: return "dbl";
        case BlockType::TokFloat: return "flt"; case BlockType::TokString: return "str"; case BlockType::TokBool: return "bool";
        case BlockType::TokChar: return "char"; case BlockType::TokVoid: return "void"; case BlockType::TokAuto: return "auto";
        case BlockType::TokConst: return "const"; case BlockType::TokIf: return "if"; case BlockType::TokElse: return "else";
        case BlockType::TokFor: return "for"; case BlockType::TokWhile: return "while"; case BlockType::TokDo: return "do";
        case BlockType::TokSwitch: return "sw"; case BlockType::TokCase: return "case"; case BlockType::TokDefault: return "def";
        case BlockType::TokBreak: return "brk"; case BlockType::TokContinue: return "next"; case BlockType::TokReturn: return "ret";
        case BlockType::TokClass: return "class"; case BlockType::TokStruct: return "struct"; case BlockType::TokPublic: return "pub";
        case BlockType::TokPrivate: return "priv"; case BlockType::TokProtected: return "prot"; case BlockType::TokTemplate: return "tmpl";
        case BlockType::TokTypename: return "type"; case BlockType::TokNamespace: return "ns"; case BlockType::TokUsing: return "using";
        case BlockType::TokTry: return "try"; case BlockType::TokCatch: return "catch"; case BlockType::TokThrow: return "throw";
        case BlockType::TokNew: return "new"; case BlockType::TokDelete: return "del"; case BlockType::TokStatic: return "stat";
        case BlockType::TokVirtual: return "virt"; case BlockType::TokOverride: return "over"; case BlockType::TokSizeof: return "size";
        case BlockType::TokMain: return "main"; case BlockType::TokStd: return "std"; case BlockType::TokCout: return "cout";
        case BlockType::TokCin: return "cin"; case BlockType::TokEndl: return "endl"; case BlockType::TokVector: return "vec";
        case BlockType::TokTrue: return "true"; case BlockType::TokFalse: return "false"; case BlockType::TokNullptr: return "null";
        case BlockType::TokIdentifier: return "name"; case BlockType::TokTypeName: return "tname";
        case BlockType::TokExpression: return "expr"; case BlockType::TokKeyword: return "kw"; case BlockType::TokNumber: return "num";
        case BlockType::TokStringLiteral: return "txt"; case BlockType::TokCharLiteral: return "chr";
        case BlockType::TokLParen: return "("; case BlockType::TokRParen: return ")"; case BlockType::TokLBrace: return "{";
        case BlockType::TokRBrace: return "}"; case BlockType::TokLBracket: return "["; case BlockType::TokRBracket: return "]";
        case BlockType::TokSemicolon: return ";"; case BlockType::TokComma: return ","; case BlockType::TokColon: return ":";
        case BlockType::TokQuestion: return "qst"; case BlockType::TokDot: return "."; case BlockType::TokScope: return "::";
        case BlockType::TokArrow: return "->"; case BlockType::TokHash: return "#"; case BlockType::TokSymbol: return "sym";
        case BlockType::TokAssign: return "="; case BlockType::TokPlus: return "+";
        case BlockType::TokMinus: return "-"; case BlockType::TokStar: return "*"; case BlockType::TokSlash: return "/";
        case BlockType::TokPercent: return "%"; case BlockType::TokEqual: return "=="; case BlockType::TokNotEqual: return "!=";
        case BlockType::TokLess: return "<"; case BlockType::TokGreater: return ">"; case BlockType::TokLessEqual: return "<=";
        case BlockType::TokGreaterEqual: return ">="; case BlockType::TokAnd: return "&&"; case BlockType::TokOr: return "||";
        case BlockType::TokNot: return "!"; case BlockType::TokAmp: return "&"; case BlockType::TokPipe: return "|";
        case BlockType::TokCaret: return "^"; case BlockType::TokTilde: return "~"; case BlockType::TokShiftLeft: return "<<";
        case BlockType::TokShiftRight: return ">>"; case BlockType::TokIncrement: return "++"; case BlockType::TokDecrement: return "--";
        case BlockType::TokPlusAssign: return "+="; case BlockType::TokMinusAssign: return "-="; case BlockType::TokMultiplyAssign: return "*=";
        case BlockType::TokDivideAssign: return "/="; case BlockType::TokModAssign: return "%=";
        case BlockType::ActionSetNumber: return "set#"; case BlockType::ActionSetText: return "setT";
        case BlockType::ActionPrintText: return "print"; case BlockType::ActionShowValue: return "show";
        case BlockType::ActionReadNumber: return "read"; case BlockType::ActionChangeNumber: return "change";
        case BlockType::ActionComment: return "note";
    }
    return "?";
}

inline bool is_branch(BlockType type) {
    return type == BlockType::ElseIf || type == BlockType::Else;
}

inline bool opens_scope(BlockType type) {
    return type == BlockType::If || type == BlockType::ElseIf || type == BlockType::Else ||
           type == BlockType::Repeat || type == BlockType::ForLoop || type == BlockType::While;
}

inline bool is_loop(BlockType type) {
    return type == BlockType::Repeat || type == BlockType::ForLoop || type == BlockType::While;
}

inline Block make_block(BlockType type) {
    Block b;
    b.type = type;
    switch (type) {
        case BlockType::IncludeHeader: b.value = "vector"; break;
        case BlockType::PrintText: b.value = "Hello, C++!"; break;
        case BlockType::PrintExpression: b.value = "2 + 3 * 4"; break;
        case BlockType::DeclareInt: b.value = "score"; b.extra = "0"; break;
        case BlockType::DeclareDouble: b.value = "distance"; b.extra = "2.5"; break;
        case BlockType::DeclareText: b.value = "player"; b.extra = "Ada"; break;
        case BlockType::DeclareBool: b.value = "ready"; b.extra = "true"; break;
        case BlockType::DeclareChar: b.value = "grade"; b.extra = "A"; break;
        case BlockType::DeclareIntList: b.value = "scores"; b.extra = "10, 20, 30"; break;
        case BlockType::ShowVariable: b.value = "score"; break;
        case BlockType::SetVariable: b.value = "score"; b.extra = "score + 1"; break;
        case BlockType::ReadInput: b.value = "score"; break;
        case BlockType::If: b.value = "score == 5"; break;
        case BlockType::ElseIf: b.value = "score == 0"; break;
        case BlockType::Else: break;
        case BlockType::Repeat: b.value = "5"; break;
        case BlockType::ForLoop: b.value = "int i = 0; i < 5; ++i"; break;
        case BlockType::While: b.value = "score < 10"; break;
        case BlockType::Break: break;
        case BlockType::Continue: break;
        case BlockType::Return: b.value = "0"; break;
        case BlockType::RawStatement: b.value = "int result = 10;"; break;
        case BlockType::RawProgram:
            b.value = "#include <iostream>\n\nint main() {\n    std::cout << \"Hello, C++!\\n\";\n    return 0;\n}\n";
            break;
        case BlockType::Comment: b.value = "Change a value and run again."; break;
        case BlockType::TokIdentifier: b.value = "score"; break;
        case BlockType::TokTypeName: b.value = "int"; break;
        case BlockType::TokExpression: b.value = "score + 1"; break;
        case BlockType::TokKeyword: b.value = "constexpr"; break;
        case BlockType::TokSymbol: b.value = "..."; break;
        case BlockType::TokNumber: b.value = "0"; break;
        case BlockType::TokStringLiteral: b.value = "Hello, C++!"; break;
        case BlockType::TokCharLiteral: b.value = "A"; break;
        case BlockType::ActionSetNumber: b.value = "score"; b.extra = "0"; break;
        case BlockType::ActionSetText: b.value = "message"; b.extra = "Hello!"; break;
        case BlockType::ActionPrintText: b.value = "Hello!"; break;
        case BlockType::ActionShowValue: b.value = "score"; break;
        case BlockType::ActionReadNumber: b.value = "score"; break;
        case BlockType::ActionChangeNumber: b.value = "score"; b.extra = "1"; break;
        case BlockType::ActionComment: b.value = "Try changing a value."; break;
        default: break;
    }
    return b;
}

// Turn source code into one visual tile per lexical token. This is intentionally a
// lexer, not a C++ parser: the original source is retained separately by the UI so
// importing never silently reformats the file. gridX/gridY provide an editable map.
inline bool decode_cpp_quoted_body(const std::string& body, std::string& decoded) {
    decoded.clear();
    for (size_t i = 0; i < body.size(); ++i) {
        const char c = body[i];
        if (c != '\\') { decoded.push_back(c); continue; }
        if (++i >= body.size()) return false;
        const char e = body[i];
        switch (e) {
            case 'n': decoded.push_back('\n'); break;
            case 'r': decoded.push_back('\r'); break;
            case 't': decoded.push_back('\t'); break;
            case '0': decoded.push_back('\0'); break;
            case '\\': decoded.push_back('\\'); break;
            case '"': decoded.push_back('"'); break;
            case '\'': decoded.push_back('\''); break;
            case 'a': decoded.push_back('\a'); break;
            case 'b': decoded.push_back('\b'); break;
            case 'f': decoded.push_back('\f'); break;
            case 'v': decoded.push_back('\v'); break;
            case '?': decoded.push_back('?'); break;
            // Leave hex, octal, and universal character escapes for the caller to
            // preserve as raw source instead of accidentally changing their meaning.
            case 'x': case 'u': case 'U': return false;
            default: return false;
        }
    }
    return true;
}

inline BlockType cpp_word_tile_type(const std::string& word, bool followsHash) {
    if (word == "include" && followsHash) return BlockType::TokInclude;
    static const std::map<std::string, BlockType> named = {
        {"int",BlockType::TokInt},{"double",BlockType::TokDouble},{"float",BlockType::TokFloat},
        {"string",BlockType::TokString},{"bool",BlockType::TokBool},{"char",BlockType::TokChar},
        {"void",BlockType::TokVoid},{"auto",BlockType::TokAuto},{"const",BlockType::TokConst},
        {"if",BlockType::TokIf},{"else",BlockType::TokElse},{"for",BlockType::TokFor},
        {"while",BlockType::TokWhile},{"do",BlockType::TokDo},{"switch",BlockType::TokSwitch},
        {"case",BlockType::TokCase},{"default",BlockType::TokDefault},{"break",BlockType::TokBreak},
        {"continue",BlockType::TokContinue},{"return",BlockType::TokReturn},{"class",BlockType::TokClass},
        {"struct",BlockType::TokStruct},{"public",BlockType::TokPublic},{"private",BlockType::TokPrivate},
        {"protected",BlockType::TokProtected},{"template",BlockType::TokTemplate},{"typename",BlockType::TokTypename},
        {"namespace",BlockType::TokNamespace},{"using",BlockType::TokUsing},{"try",BlockType::TokTry},
        {"catch",BlockType::TokCatch},{"throw",BlockType::TokThrow},{"new",BlockType::TokNew},
        {"delete",BlockType::TokDelete},{"static",BlockType::TokStatic},{"virtual",BlockType::TokVirtual},
        {"override",BlockType::TokOverride},{"sizeof",BlockType::TokSizeof},{"main",BlockType::TokMain},
        {"std",BlockType::TokStd},{"cout",BlockType::TokCout},{"cin",BlockType::TokCin},
        {"endl",BlockType::TokEndl},{"vector",BlockType::TokVector},{"true",BlockType::TokTrue},
        {"false",BlockType::TokFalse},{"nullptr",BlockType::TokNullptr}
    };
    auto it = named.find(word);
    if (it != named.end()) return it->second;
    static const std::set<std::string> keywords = {
        "alignas","alignof","and","and_eq","asm","atomic_cancel","atomic_commit","atomic_noexcept",
        "bitand","bitor","char8_t","char16_t","char32_t","concept","const_cast","consteval",
        "constexpr","constinit","co_await","co_return","co_yield","decltype","delete","dynamic_cast",
        "explicit","export","extern","friend","goto","inline","long","mutable","not","not_eq",
        "noexcept","operator","or","or_eq","private","protected","public","register","reinterpret_cast",
        "requires","restrict","short","signed","static_assert","static_cast","thread_local","typeid",
        "typedef","union","unsigned","wchar_t","xor","xor_eq","volatile"
    };
    if (keywords.count(word)) return BlockType::TokKeyword;
    return BlockType::TokIdentifier;
}

inline BlockType cpp_symbol_tile_type(const std::string& token) {
    static const std::map<std::string, BlockType> named = {
        {"(",BlockType::TokLParen},{")",BlockType::TokRParen},{"{",BlockType::TokLBrace},{"}",BlockType::TokRBrace},
        {"[",BlockType::TokLBracket},{"]",BlockType::TokRBracket},{";",BlockType::TokSemicolon},{",",BlockType::TokComma},
        {":",BlockType::TokColon},{"?",BlockType::TokQuestion},{".",BlockType::TokDot},{"::",BlockType::TokScope},
        {"->",BlockType::TokArrow},{"#",BlockType::TokHash},{"=",BlockType::TokAssign},{"+",BlockType::TokPlus},
        {"-",BlockType::TokMinus},{"*",BlockType::TokStar},{"/",BlockType::TokSlash},{"%",BlockType::TokPercent},
        {"==",BlockType::TokEqual},{"!=",BlockType::TokNotEqual},{"<",BlockType::TokLess},{">",BlockType::TokGreater},
        {"<=",BlockType::TokLessEqual},{">=",BlockType::TokGreaterEqual},{"&&",BlockType::TokAnd},{"||",BlockType::TokOr},
        {"!",BlockType::TokNot},{"&",BlockType::TokAmp},{"|",BlockType::TokPipe},{"^",BlockType::TokCaret},
        {"~",BlockType::TokTilde},{"<<",BlockType::TokShiftLeft},{">>",BlockType::TokShiftRight},
        {"++",BlockType::TokIncrement},{"--",BlockType::TokDecrement},{"+=",BlockType::TokPlusAssign},
        {"-=",BlockType::TokMinusAssign},{"*=",BlockType::TokMultiplyAssign},{"/=",BlockType::TokDivideAssign},
        {"%=",BlockType::TokModAssign}
    };
    auto it = named.find(token);
    return it == named.end() ? BlockType::TokSymbol : it->second;
}

inline std::vector<Block> tokenize_cpp_source(const std::string& source, size_t maxTokens = 20000) {
    std::vector<Block> blocks;
    size_t i = 0;
    int row = 0, column = 0;
    bool inBlockComment = false;
    if (source.size() >= 3 && static_cast<unsigned char>(source[0]) == 0xEF &&
        static_cast<unsigned char>(source[1]) == 0xBB && static_cast<unsigned char>(source[2]) == 0xBF) i = 3;

    auto add = [&](BlockType type, const std::string& value, int y, int x) {
        if (blocks.size() >= maxTokens) return false;
        Block b = make_block(type);
        if (type == BlockType::TokIdentifier || type == BlockType::TokTypeName || type == BlockType::TokExpression ||
            type == BlockType::TokKeyword || type == BlockType::TokNumber || type == BlockType::TokStringLiteral ||
            type == BlockType::TokCharLiteral || type == BlockType::TokSymbol) b.value = value;
        b.gridX = x; b.gridY = y; b.depth = 0;
        blocks.push_back(std::move(b));
        return true;
    };
    auto advance_raw_position = [&](const std::string& raw) {
        for (char ch : raw) {
            if (ch == '\n') { ++row; column = 0; }
        }
        // A multi-line literal is shown as one tile at its opening position.
        if (raw.find('\n') != std::string::npos) column = 0;
    };
    auto is_ident_start = [](unsigned char c) { return std::isalpha(c) || c == '_' || c >= 128; };
    auto is_ident_char = [](unsigned char c) { return std::isalnum(c) || c == '_' || c >= 128; };

    while (i < source.size()) {
        const char c = source[i];
        if (c == '\r') { ++i; continue; }
        if (c == '\n') { ++i; ++row; column = 0; continue; }
        if (std::isspace(static_cast<unsigned char>(c))) { ++i; continue; }

        if (inBlockComment) {
            const size_t start = i;
            bool closes = false;
            while (i < source.size() && source[i] != '\n') {
                if (source[i] == '*' && i + 1 < source.size() && source[i + 1] == '/') { i += 2; closes = true; break; }
                ++i;
            }
            std::string part = source.substr(start, i - start);
            if (!part.empty() && !add(BlockType::TokSymbol, part, row, column)) return {};
            if (!part.empty()) ++column;
            if (closes) inBlockComment = false;
            continue;
        }
        if (c == '/' && i + 1 < source.size() && source[i + 1] == '/') {
            size_t end = source.find('\n', i);
            if (end == std::string::npos) end = source.size();
            if (!add(BlockType::TokSymbol, source.substr(i, end - i), row, column)) return {};
            ++column; i = end; continue;
        }
        if (c == '/' && i + 1 < source.size() && source[i + 1] == '*') {
            const size_t start = i;
            i += 2;
            bool closes = false;
            while (i < source.size() && source[i] != '\n') {
                if (source[i] == '*' && i + 1 < source.size() && source[i + 1] == '/') { i += 2; closes = true; break; }
                ++i;
            }
            std::string part = source.substr(start, i - start);
            if (!add(BlockType::TokSymbol, part, row, column)) return {};
            ++column; inBlockComment = !closes; continue;
        }

        // Detect raw and encoded string/character literal prefixes before identifiers.
        size_t quotePos = std::string::npos;
        bool rawLiteral = false;
        for (const std::string& prefix : {std::string("u8R\""), std::string("uR\""), std::string("UR\""), std::string("LR\""), std::string("R\"")}) {
            if (source.compare(i, prefix.size(), prefix) == 0) { quotePos = i + prefix.size() - 1; rawLiteral = true; break; }
        }
        if (quotePos == std::string::npos) {
            for (const std::string& prefix : {std::string("u8\""), std::string("u\""), std::string("U\""), std::string("L\""),
                                               std::string("u8'"), std::string("u'"), std::string("U'"), std::string("L'")}) {
                if (source.compare(i, prefix.size(), prefix) == 0) { quotePos = i + prefix.size() - 1; break; }
            }
        }
        const bool beginsWithQuote = quotePos != std::string::npos || c == '"' || c == '\'';
        if (beginsWithQuote) {
            const size_t start = i;
            const char quote = quotePos == std::string::npos ? c : source[quotePos];
            bool closed = false;
            size_t end = start;
            if (rawLiteral) {
                const size_t delimStart = quotePos + 1;
                const size_t paren = source.find('(', delimStart);
                if (paren != std::string::npos && paren - delimStart <= 16) {
                    const std::string delim = source.substr(delimStart, paren - delimStart);
                    const std::string terminator = ")" + delim + "\"";
                    const size_t term = source.find(terminator, paren + 1);
                    if (term != std::string::npos) { end = term + terminator.size(); closed = true; }
                }
            } else {
                size_t p = quotePos == std::string::npos ? start + 1 : quotePos + 1;
                while (p < source.size()) {
                    if (source[p] == '\\') { p += std::min<size_t>(2, source.size() - p); continue; }
                    if (source[p] == quote) { end = p + 1; closed = true; break; }
                    if (source[p] == '\n') break;
                    ++p;
                }
            }
            if (!closed) {
                // Keep malformed or unusual literal spelling as one editable raw symbol;
                // the exact original text remains visible and runnable until edited.
                size_t p = source.find('\n', start);
                end = p == std::string::npos ? source.size() : p;
            }
            const std::string spelling = source.substr(start, end - start);
            const size_t bodyStart = quotePos == std::string::npos ? start + 1 : quotePos + 1;
            BlockType type = BlockType::TokSymbol;
            std::string value = spelling;
            if (closed && !rawLiteral && quotePos == std::string::npos) {
                std::string decoded;
                if (quote == '"' && decode_cpp_quoted_body(spelling.substr(1, spelling.size() - 2), decoded)) {
                    type = BlockType::TokStringLiteral; value = decoded;
                } else if (quote == '\'' && decode_cpp_quoted_body(spelling.substr(1, spelling.size() - 2), decoded)) {
                    type = BlockType::TokCharLiteral; value = decoded;
                }
            }
            if (!add(type, value, row, column)) return {};
            if (spelling.find('\n') != std::string::npos) advance_raw_position(spelling); else ++column;
            i = end;
            (void)bodyStart;
            continue;
        }

        const unsigned char uc = static_cast<unsigned char>(c);
        if (is_ident_start(uc)) {
            const size_t start = i++;
            while (i < source.size() && is_ident_char(static_cast<unsigned char>(source[i]))) ++i;
            const std::string word = source.substr(start, i - start);
            const bool followsHash = !blocks.empty() && blocks.back().gridY == row && blocks.back().type == BlockType::TokHash;
            const BlockType type = cpp_word_tile_type(word, followsHash);
            if (!add(type, word, row, column)) return {};
            ++column; continue;
        }

        const bool numberStart = std::isdigit(uc) || (c == '.' && i + 1 < source.size() && std::isdigit(static_cast<unsigned char>(source[i + 1])));
        if (numberStart) {
            const size_t start = i++;
            while (i < source.size()) {
                const char n = source[i];
                if (std::isalnum(static_cast<unsigned char>(n)) || n == '_' || n == '\'') { ++i; continue; }
                if (n == '.') { ++i; continue; }
                if ((n == '+' || n == '-') && i > start && std::string("eEpP").find(source[i - 1]) != std::string::npos) { ++i; continue; }
                break;
            }
            if (!add(BlockType::TokNumber, source.substr(start, i - start), row, column)) return {};
            ++column; continue;
        }

        static const std::vector<std::string> operators = {
            "%:%:",">>=","<<=",">>=","->*","...","<=>","##","::","->","++","--","+=","-=","*=","/=","%=","==","!=","<=",">=","&&","||","<<",">>",".*","<:",":>","<%","%>"
        };
        std::string token(1, c);
        for (const auto& op : operators) {
            if (source.compare(i, op.size(), op) == 0 && op.size() > token.size()) { token = op; break; }
        }
        if (!add(cpp_symbol_tile_type(token), token, row, column)) return {};
        i += token.size(); ++column;
    }
    return blocks;
}

inline std::string cpp_string_literal(const std::string& input) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : input) {
        switch (c) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            case '\0': out << "\\000"; break;
            default:
                if (c < 32) {
                    out << '\\' << std::oct << std::setw(3) << std::setfill('0')
                        << static_cast<int>(c) << std::dec;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    out << '"';
    return out.str();
}

inline std::string cpp_char_literal(const std::string& input) {
    if (input == "\\n") return "'\\n'";
    if (input == "\\r") return "'\\r'";
    if (input == "\\t") return "'\\t'";
    if (input == "\\0") return "'\\0'";
    if (input.empty()) return "'\\0'";
    const unsigned char c = static_cast<unsigned char>(input.front());
    switch (c) {
        case '\\': return "'\\\\'";
        case '\'': return "'\\\''";
        case '\n': return "'\\n'";
        case '\r': return "'\\r'";
        case '\t': return "'\\t'";
        case '\0': return "'\\0'";
        default: return std::string("'") + static_cast<char>(c) + "'";
    }
}

inline std::string trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

inline std::string normalize_header(std::string header) {
    header = trim(std::move(header));
    if (header.empty()) return {};
    if ((header.front() == '<' && header.back() == '>') ||
        (header.front() == '"' && header.back() == '"')) return header;
    return "<" + header + ">";
}

struct GenerateResult {
    std::string code;
    std::vector<std::string> errors;
    // One owner index per generated source line. -1 means a generated header/footer.
    std::vector<int> lineOwners;
};

inline bool is_atomic_mode_type(BlockType t) {
    switch (t) {
        case BlockType::TokInclude: case BlockType::TokInt: case BlockType::TokDouble: case BlockType::TokFloat:
        case BlockType::TokString: case BlockType::TokBool: case BlockType::TokChar: case BlockType::TokVoid:
        case BlockType::TokAuto: case BlockType::TokConst: case BlockType::TokIf: case BlockType::TokElse:
        case BlockType::TokFor: case BlockType::TokWhile: case BlockType::TokDo: case BlockType::TokSwitch:
        case BlockType::TokCase: case BlockType::TokDefault: case BlockType::TokBreak: case BlockType::TokContinue:
        case BlockType::TokReturn: case BlockType::TokClass: case BlockType::TokStruct: case BlockType::TokPublic:
        case BlockType::TokPrivate: case BlockType::TokProtected: case BlockType::TokTemplate: case BlockType::TokTypename:
        case BlockType::TokNamespace: case BlockType::TokUsing: case BlockType::TokTry: case BlockType::TokCatch:
        case BlockType::TokThrow: case BlockType::TokNew: case BlockType::TokDelete: case BlockType::TokStatic:
        case BlockType::TokVirtual: case BlockType::TokOverride: case BlockType::TokSizeof: case BlockType::TokMain:
        case BlockType::TokStd: case BlockType::TokCout: case BlockType::TokCin: case BlockType::TokEndl:
        case BlockType::TokVector: case BlockType::TokTrue: case BlockType::TokFalse: case BlockType::TokNullptr:
        case BlockType::TokIdentifier: case BlockType::TokKeyword: case BlockType::TokNumber: case BlockType::TokStringLiteral:
        case BlockType::TokTypeName: case BlockType::TokExpression: case BlockType::TokCharLiteral: case BlockType::TokLParen: case BlockType::TokRParen: case BlockType::TokLBrace:
        case BlockType::TokRBrace: case BlockType::TokLBracket: case BlockType::TokRBracket: case BlockType::TokSemicolon:
        case BlockType::TokComma: case BlockType::TokColon: case BlockType::TokQuestion: case BlockType::TokDot:
        case BlockType::TokScope: case BlockType::TokArrow: case BlockType::TokHash: case BlockType::TokSymbol:
        case BlockType::TokAssign: case BlockType::TokPlus:
        case BlockType::TokMinus: case BlockType::TokStar: case BlockType::TokSlash: case BlockType::TokPercent:
        case BlockType::TokEqual: case BlockType::TokNotEqual: case BlockType::TokLess: case BlockType::TokGreater:
        case BlockType::TokLessEqual: case BlockType::TokGreaterEqual: case BlockType::TokAnd: case BlockType::TokOr:
        case BlockType::TokNot: case BlockType::TokAmp: case BlockType::TokPipe: case BlockType::TokCaret:
        case BlockType::TokTilde: case BlockType::TokShiftLeft: case BlockType::TokShiftRight: case BlockType::TokIncrement:
        case BlockType::TokDecrement: case BlockType::TokPlusAssign: case BlockType::TokMinusAssign:
        case BlockType::TokMultiplyAssign: case BlockType::TokDivideAssign: case BlockType::TokModAssign:
        case BlockType::ActionSetNumber: case BlockType::ActionSetText: case BlockType::ActionPrintText:
        case BlockType::ActionShowValue: case BlockType::ActionReadNumber: case BlockType::ActionChangeNumber:
        case BlockType::ActionComment:
            return true;
        default: return false;
    }
}

inline bool is_atomic_action_type(BlockType t) {
    return t == BlockType::ActionSetNumber || t == BlockType::ActionSetText || t == BlockType::ActionPrintText ||
           t == BlockType::ActionShowValue || t == BlockType::ActionReadNumber || t == BlockType::ActionChangeNumber ||
           t == BlockType::ActionComment;
}

inline std::string atomic_token_text(const Block& b) {
    switch (b.type) {
        case BlockType::TokInclude: return "include";
        case BlockType::TokInt: return "int"; case BlockType::TokDouble: return "double"; case BlockType::TokFloat: return "float";
        case BlockType::TokString: return "string"; case BlockType::TokBool: return "bool"; case BlockType::TokChar: return "char";
        case BlockType::TokVoid: return "void"; case BlockType::TokAuto: return "auto"; case BlockType::TokConst: return "const";
        case BlockType::TokIf: return "if"; case BlockType::TokElse: return "else"; case BlockType::TokFor: return "for";
        case BlockType::TokWhile: return "while"; case BlockType::TokDo: return "do"; case BlockType::TokSwitch: return "switch";
        case BlockType::TokCase: return "case"; case BlockType::TokDefault: return "default"; case BlockType::TokBreak: return "break";
        case BlockType::TokContinue: return "continue"; case BlockType::TokReturn: return "return"; case BlockType::TokClass: return "class";
        case BlockType::TokStruct: return "struct"; case BlockType::TokPublic: return "public"; case BlockType::TokPrivate: return "private";
        case BlockType::TokProtected: return "protected"; case BlockType::TokTemplate: return "template"; case BlockType::TokTypename: return "typename";
        case BlockType::TokNamespace: return "namespace"; case BlockType::TokUsing: return "using"; case BlockType::TokTry: return "try";
        case BlockType::TokCatch: return "catch"; case BlockType::TokThrow: return "throw"; case BlockType::TokNew: return "new";
        case BlockType::TokDelete: return "delete"; case BlockType::TokStatic: return "static"; case BlockType::TokVirtual: return "virtual";
        case BlockType::TokOverride: return "override"; case BlockType::TokSizeof: return "sizeof"; case BlockType::TokMain: return "main";
        case BlockType::TokStd: return "std"; case BlockType::TokCout: return "cout"; case BlockType::TokCin: return "cin";
        case BlockType::TokEndl: return "endl"; case BlockType::TokVector: return "vector"; case BlockType::TokTrue: return "true";
        case BlockType::TokFalse: return "false"; case BlockType::TokNullptr: return "nullptr";
        case BlockType::TokIdentifier: case BlockType::TokTypeName: case BlockType::TokExpression:
        case BlockType::TokKeyword: case BlockType::TokNumber: return b.value;
        case BlockType::TokStringLiteral: return cpp_string_literal(b.value);
        case BlockType::TokCharLiteral: return cpp_char_literal(b.value);
        case BlockType::TokLParen: return "("; case BlockType::TokRParen: return ")"; case BlockType::TokLBrace: return "{";
        case BlockType::TokRBrace: return "}"; case BlockType::TokLBracket: return "["; case BlockType::TokRBracket: return "]";
        case BlockType::TokSemicolon: return ";"; case BlockType::TokComma: return ","; case BlockType::TokColon: return ":";
        case BlockType::TokQuestion: return "?"; case BlockType::TokDot: return "."; case BlockType::TokScope: return "::";
        case BlockType::TokArrow: return "->"; case BlockType::TokHash: return "#"; case BlockType::TokSymbol: return b.value;
        case BlockType::TokAssign: return "="; case BlockType::TokPlus: return "+";
        case BlockType::TokMinus: return "-"; case BlockType::TokStar: return "*"; case BlockType::TokSlash: return "/";
        case BlockType::TokPercent: return "%"; case BlockType::TokEqual: return "=="; case BlockType::TokNotEqual: return "!=";
        case BlockType::TokLess: return "<"; case BlockType::TokGreater: return ">"; case BlockType::TokLessEqual: return "<=";
        case BlockType::TokGreaterEqual: return ">="; case BlockType::TokAnd: return "&&"; case BlockType::TokOr: return "||";
        case BlockType::TokNot: return "!"; case BlockType::TokAmp: return "&"; case BlockType::TokPipe: return "|";
        case BlockType::TokCaret: return "^"; case BlockType::TokTilde: return "~"; case BlockType::TokShiftLeft: return "<<";
        case BlockType::TokShiftRight: return ">>"; case BlockType::TokIncrement: return "++"; case BlockType::TokDecrement: return "--";
        case BlockType::TokPlusAssign: return "+="; case BlockType::TokMinusAssign: return "-="; case BlockType::TokMultiplyAssign: return "*=";
        case BlockType::TokDivideAssign: return "/="; case BlockType::TokModAssign: return "%=";
        default: return {};
    }
}

inline bool atomic_word_token(BlockType t) {
    switch (t) {
        case BlockType::TokInt: case BlockType::TokDouble: case BlockType::TokFloat: case BlockType::TokString:
        case BlockType::TokBool: case BlockType::TokChar: case BlockType::TokVoid: case BlockType::TokAuto:
        case BlockType::TokConst: case BlockType::TokIf: case BlockType::TokElse: case BlockType::TokFor:
        case BlockType::TokWhile: case BlockType::TokDo: case BlockType::TokSwitch: case BlockType::TokCase:
        case BlockType::TokDefault: case BlockType::TokBreak: case BlockType::TokContinue: case BlockType::TokReturn:
        case BlockType::TokClass: case BlockType::TokStruct: case BlockType::TokPublic: case BlockType::TokPrivate:
        case BlockType::TokProtected: case BlockType::TokTemplate: case BlockType::TokTypename: case BlockType::TokNamespace:
        case BlockType::TokUsing: case BlockType::TokTry: case BlockType::TokCatch: case BlockType::TokThrow:
        case BlockType::TokNew: case BlockType::TokDelete: case BlockType::TokStatic: case BlockType::TokVirtual:
        case BlockType::TokOverride: case BlockType::TokSizeof: case BlockType::TokMain: case BlockType::TokStd:
        case BlockType::TokCout: case BlockType::TokCin: case BlockType::TokEndl: case BlockType::TokVector:
        case BlockType::TokTrue: case BlockType::TokFalse: case BlockType::TokNullptr: case BlockType::TokIdentifier:
        case BlockType::TokTypeName: case BlockType::TokExpression: case BlockType::TokKeyword: case BlockType::TokNumber:
            return true;
        default: return false;
    }
}

inline bool atomic_needs_space(const Block& left, const Block& right, bool insideInclude) {
    const auto a = left.type, b = right.type;
    if (a == BlockType::TokInclude) return true;
    if (a == BlockType::TokHash) return false;
    if (a == BlockType::TokSymbol || b == BlockType::TokSymbol) return true;
    if (insideInclude && (a == BlockType::TokLess || b == BlockType::TokGreater)) return false;
    if (b == BlockType::TokRParen || b == BlockType::TokRBracket || b == BlockType::TokSemicolon ||
        b == BlockType::TokComma || b == BlockType::TokDot || b == BlockType::TokScope || b == BlockType::TokArrow)
        return false;
    if (a == BlockType::TokLParen || a == BlockType::TokLBracket || a == BlockType::TokDot ||
        a == BlockType::TokScope || a == BlockType::TokArrow) return false;
    if (b == BlockType::TokLParen) return a == BlockType::TokIf || a == BlockType::TokFor || a == BlockType::TokWhile ||
        a == BlockType::TokSwitch || a == BlockType::TokCatch || a == BlockType::TokSizeof;
    if (b == BlockType::TokLBrace) return true;
    if (a == BlockType::TokRBrace) return true;
    if (b == BlockType::TokRBrace) return false;
    if (a == BlockType::TokComma || a == BlockType::TokSemicolon || a == BlockType::TokColon) return true;
    if (atomic_word_token(a) && (atomic_word_token(b) || b == BlockType::TokStringLiteral || b == BlockType::TokCharLiteral)) return true;
    if ((a == BlockType::TokStringLiteral || a == BlockType::TokCharLiteral) && atomic_word_token(b)) return true;
    if (b == BlockType::TokAssign || b == BlockType::TokPlus || b == BlockType::TokMinus || b == BlockType::TokStar ||
        b == BlockType::TokSlash || b == BlockType::TokPercent || b == BlockType::TokEqual || b == BlockType::TokNotEqual ||
        b == BlockType::TokLess || b == BlockType::TokGreater || b == BlockType::TokLessEqual || b == BlockType::TokGreaterEqual ||
        b == BlockType::TokAnd || b == BlockType::TokOr || b == BlockType::TokShiftLeft || b == BlockType::TokShiftRight ||
        b == BlockType::TokPlusAssign || b == BlockType::TokMinusAssign || b == BlockType::TokMultiplyAssign ||
        b == BlockType::TokDivideAssign || b == BlockType::TokModAssign) return true;
    if (a == BlockType::TokAssign || a == BlockType::TokPlus || a == BlockType::TokMinus || a == BlockType::TokStar ||
        a == BlockType::TokSlash || a == BlockType::TokPercent || a == BlockType::TokEqual || a == BlockType::TokNotEqual ||
        a == BlockType::TokLess || a == BlockType::TokGreater || a == BlockType::TokLessEqual || a == BlockType::TokGreaterEqual ||
        a == BlockType::TokAnd || a == BlockType::TokOr || a == BlockType::TokShiftLeft || a == BlockType::TokShiftRight ||
        a == BlockType::TokPlusAssign || a == BlockType::TokMinusAssign || a == BlockType::TokMultiplyAssign ||
        a == BlockType::TokDivideAssign || a == BlockType::TokModAssign) return true;
    if (a == BlockType::TokRParen && b == BlockType::TokLBrace) return true;
    return false;
}

inline std::string action_source(const Block& b) {
    switch (b.type) {
        case BlockType::ActionSetNumber: return "int " + b.value + " = " + (b.extra.empty() ? "0" : b.extra) + ";";
        case BlockType::ActionSetText: return "std::string " + b.value + " = " + cpp_string_literal(b.extra) + ";";
        case BlockType::ActionPrintText: return "std::cout << " + cpp_string_literal(b.value) + " << '\\n';";
        case BlockType::ActionShowValue: return "std::cout << " + b.value + " << '\\n';";
        case BlockType::ActionReadNumber: return "std::cin >> " + b.value + ";";
        case BlockType::ActionChangeNumber: return b.value + " += " + (b.extra.empty() ? "1" : b.extra) + ";";
        case BlockType::ActionComment: return "// " + b.value;
        default: return {};
    }
}

inline GenerateResult generate_atomic_cpp(const std::vector<Block>& blocks) {
    GenerateResult result;
    std::map<int, std::vector<size_t>> rows;
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (!is_atomic_mode_type(blocks[i].type)) {
            result.errors.push_back("Block " + std::to_string(i + 1) + " (" + block_name(blocks[i].type) + ") is a legacy whole-line block. Remove it or replace it with individual syntax tiles.");
            continue;
        }
        if (blocks[i].gridX < 0 || blocks[i].gridY < 0) {
            result.errors.push_back("Block " + std::to_string(i + 1) + " has a negative grid position. Move it back onto the grid.");
            continue;
        }
        rows[blocks[i].gridY].push_back(i);
    }
    if (rows.empty()) {
        result.errors.push_back("Your program is empty. Add tiles to build a complete C++ program.");
        return result;
    }

    std::vector<std::pair<std::string, int>> generatedLines;
    int braceDepth = 0;
    for (int y = 0; y <= rows.rbegin()->first; ++y) {
        auto it = rows.find(y);
        if (it == rows.end()) { generatedLines.emplace_back("", -1); continue; }
        auto indexes = it->second;
        std::sort(indexes.begin(), indexes.end(), [&](size_t a, size_t b) {
            if (blocks[a].gridX != blocks[b].gridX) return blocks[a].gridX < blocks[b].gridX;
            return a < b;
        });
        bool duplicate = false;
        for (size_t k = 1; k < indexes.size(); ++k) {
            if (blocks[indexes[k - 1]].gridX == blocks[indexes[k]].gridX) {
                duplicate = true;
                result.errors.push_back("Two tiles overlap at grid row " + std::to_string(y + 1) + ", column " +
                    std::to_string(blocks[indexes[k]].gridX + 1) + ". Move one tile to another cell.");
            }
        }
        const int owner = static_cast<int>(indexes.front());
        int actions = 0;
        size_t actionIndex = 0;
        for (size_t idx : indexes) if (is_atomic_action_type(blocks[idx].type)) { ++actions; actionIndex = idx; }
        if (actions) {
            if (actions != 1 || indexes.size() != 1) {
                result.errors.push_back("A helper action such as Set number or Print text needs its own row. Put syntax tiles on a different row.");
                generatedLines.emplace_back("// ERROR: move helper actions to their own row", owner);
                continue;
            }
            std::string text = action_source(blocks[actionIndex]);
            const int indent = std::max(0, braceDepth) * 4;
            if (blocks[actionIndex].type == BlockType::ActionComment) generatedLines.emplace_back(std::string(static_cast<size_t>(indent), ' ') + text, owner);
            else generatedLines.emplace_back(std::string(static_cast<size_t>(indent), ' ') + text, owner);
            continue;
        }
        if (duplicate) generatedLines.emplace_back("// ERROR: overlapping tiles", owner);

        std::string text;
        bool inInclude = false;
        for (size_t k = 0; k < indexes.size(); ++k) {
            const Block& b = blocks[indexes[k]];
            if (k > 0 && atomic_needs_space(blocks[indexes[k - 1]], b, inInclude)) text.push_back(' ');
            text += atomic_token_text(b);
            if (b.type == BlockType::TokInclude) inInclude = true;
        }
        int lineIndent = braceDepth;
        if (!indexes.empty() && blocks[indexes.front()].type == BlockType::TokRBrace) lineIndent = std::max(0, braceDepth - 1);
        if (!text.empty() && text.rfind("#include", 0) != 0) text = std::string(static_cast<size_t>(std::max(0, lineIndent) * 4), ' ') + text;
        generatedLines.emplace_back(text, owner);
        for (size_t idx : indexes) {
            if (blocks[idx].type == BlockType::TokLBrace) ++braceDepth;
            else if (blocks[idx].type == BlockType::TokRBrace) braceDepth = std::max(0, braceDepth - 1);
        }
        (void)duplicate;
    }

    // Friendly action blocks bring in the standard headers they need, without adding any hidden statements.
    bool needsIostream = false, needsString = false;
    for (const auto& b : blocks) {
        if (b.type == BlockType::ActionPrintText || b.type == BlockType::ActionShowValue || b.type == BlockType::ActionReadNumber) needsIostream = true;
        if (b.type == BlockType::ActionSetText) needsString = true;
    }
    auto hasHeader = [&](const std::string& h) {
        const std::string wanted = "#include <" + h + ">";
        return std::any_of(generatedLines.begin(), generatedLines.end(), [&](const auto& l) { return trim(l.first) == wanted; });
    };
    std::vector<std::pair<std::string, int>> prefix;
    if (needsIostream && !hasHeader("iostream")) prefix.emplace_back("#include <iostream>", -1);
    if (needsString && !hasHeader("string")) prefix.emplace_back("#include <string>", -1);
    if (!prefix.empty() && !generatedLines.empty() && !generatedLines.front().first.empty()) prefix.emplace_back("", -1);
    prefix.insert(prefix.end(), generatedLines.begin(), generatedLines.end());

    std::ostringstream out;
    for (const auto& item : prefix) {
        out << item.first << '\n';
        result.lineOwners.push_back(item.second);
    }
    result.code = out.str();
    return result;
}

inline std::vector<Block> atomic_example_program() {
    // The starter is intentionally assembled from individual syntax tiles and reusable value slots.
    // No single tile secretly creates an entire line of source code.
    std::vector<Block> blocks;
    auto put = [&](BlockType type, int x, int y, const std::string& value = {}, const std::string& extra = {}) {
        Block b = make_block(type); b.gridX = x; b.gridY = y;
        if (!value.empty()) b.value = value;
        if (!extra.empty()) b.extra = extra;
        blocks.push_back(std::move(b));
    };

    // #include <iostream>
    put(BlockType::TokHash, 0, 0); put(BlockType::TokInclude, 1, 0); put(BlockType::TokLess, 2, 0);
    put(BlockType::TokIdentifier, 3, 0, "iostream"); put(BlockType::TokGreater, 4, 0);
    // int main() {
    put(BlockType::TokInt, 0, 1); put(BlockType::TokMain, 1, 1); put(BlockType::TokLParen, 2, 1);
    put(BlockType::TokRParen, 3, 1); put(BlockType::TokLBrace, 4, 1);
    // int score = 0;
    put(BlockType::TokInt, 0, 2); put(BlockType::TokIdentifier, 1, 2, "score");
    put(BlockType::TokAssign, 2, 2); put(BlockType::TokNumber, 3, 2, "0"); put(BlockType::TokSemicolon, 4, 2);
    // std::cout << "Change the Number tile and run again." << '\n';
    put(BlockType::TokStd, 0, 3); put(BlockType::TokScope, 1, 3); put(BlockType::TokCout, 2, 3);
    put(BlockType::TokShiftLeft, 3, 3); put(BlockType::TokStringLiteral, 4, 3, "Change the Number tile and run again.");
    put(BlockType::TokShiftLeft, 5, 3); put(BlockType::TokCharLiteral, 6, 3, "\\n"); put(BlockType::TokSemicolon, 7, 3);
    // if (score == 0) {
    put(BlockType::TokIf, 0, 4); put(BlockType::TokLParen, 1, 4); put(BlockType::TokIdentifier, 2, 4, "score");
    put(BlockType::TokEqual, 3, 4); put(BlockType::TokNumber, 4, 4, "0"); put(BlockType::TokRParen, 5, 4);
    put(BlockType::TokLBrace, 6, 4);
    // std::cout << "The score is zero." << '\n';
    put(BlockType::TokStd, 0, 5); put(BlockType::TokScope, 1, 5); put(BlockType::TokCout, 2, 5);
    put(BlockType::TokShiftLeft, 3, 5); put(BlockType::TokStringLiteral, 4, 5, "The score is zero.");
    put(BlockType::TokShiftLeft, 5, 5); put(BlockType::TokCharLiteral, 6, 5, "\\n"); put(BlockType::TokSemicolon, 7, 5);
    // }
    put(BlockType::TokRBrace, 0, 6);
    // return 0;
    put(BlockType::TokReturn, 0, 7); put(BlockType::TokNumber, 1, 7, "0"); put(BlockType::TokSemicolon, 2, 7);
    // }
    put(BlockType::TokRBrace, 0, 8);
    return blocks;
}

inline GenerateResult generate_cpp(const std::vector<Block>& blocks) {
    GenerateResult result;
    if (blocks.empty()) {
        result.errors.push_back("Your program is empty. Add individual C++ tiles to build it, starting with #include and int main ( ) {.");
        return result;
    }
    std::ostringstream out;
    auto line = [&](const std::string& text, int owner) {
        out << text << '\n';
        result.lineOwners.push_back(owner);
    };

    std::vector<size_t> rawProgramIndices;
    for (size_t i = 0; i < blocks.size(); ++i) if (blocks[i].type == BlockType::RawProgram) rawProgramIndices.push_back(i);
    if (!rawProgramIndices.empty()) {
        const size_t rawIndex = rawProgramIndices.front();
        if (rawProgramIndices.size() != 1 || blocks.size() != 1) {
            result.errors.push_back("Block " + std::to_string(rawIndex + 1) + " (C++ file) must be used by itself. Remove the other blocks first, or undo to return to the visual program.");
        }
        result.code = blocks[rawIndex].value;
        if (result.code.empty()) result.errors.push_back("Block " + std::to_string(rawIndex + 1) + " (C++ file) is empty. Edit it and enter a complete C++ source file.");
        if (!result.code.empty() && result.code.back() != '\n') result.code.push_back('\n');
        result.lineOwners.assign(static_cast<size_t>(std::count(result.code.begin(), result.code.end(), '\n')),
                                 static_cast<int>(rawIndex));
        return result;
    }

    if (std::any_of(blocks.begin(), blocks.end(), [](const Block& b) { return is_atomic_mode_type(b.type); })) {
        return generate_atomic_cpp(blocks);
    }

    // Headers are file-level in C++, so Include blocks work wherever they appear in the workspace.
    std::vector<std::pair<std::string, int>> headers = {{"<iostream>", -1}, {"<string>", -1}};
    std::set<std::string> seen = {"<iostream>", "<string>"};
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].type == BlockType::DeclareIntList) {
            if (seen.insert("<vector>").second) headers.emplace_back("<vector>", -1);
        }
        if (blocks[i].type == BlockType::IncludeHeader) {
            const std::string header = normalize_header(blocks[i].value);
            if (header.empty()) {
                result.errors.push_back("Block " + std::to_string(i + 1) + " (Include) is empty. Enter a header such as vector or cmath.");
            } else if (seen.insert(header).second) {
                headers.emplace_back(header, static_cast<int>(i));
            }
        }
    }
    for (const auto& h : headers) line("#include " + h.first, h.second);
    line("", -1);
    line("int main() {", -1);
    line("    // Built by C++ Circuit Lab. Change a block, then run again.", -1);

    struct Scope { int depth; BlockType type; int blockIndex; };
    std::vector<Scope> scopes;
    unsigned int repeatId = 0;

    auto close_scope = [&](const Scope& scope) {
        line(std::string(static_cast<size_t>(std::max(0, scope.depth + 1)) * 4, ' ') + "}", scope.blockIndex);
    };
    auto close_to = [&](int depth) {
        while (!scopes.empty() && scopes.back().depth >= depth) {
            close_scope(scopes.back());
            scopes.pop_back();
        }
    };
    auto in_loop = [&]() {
        return std::any_of(scopes.begin(), scopes.end(), [](const Scope& s) { return is_loop(s.type); });
    };

    for (size_t i = 0; i < blocks.size(); ++i) {
        const Block& b = blocks[i];
        const int d = std::max(0, b.depth);
        const int owner = static_cast<int>(i);
        const std::string pad(static_cast<size_t>(d + 1) * 4, ' ');

        if (b.type == BlockType::IncludeHeader) {
            if (d != 0) result.errors.push_back("Block " + std::to_string(i + 1) + " (Include) belongs at the left edge. Headers are placed at the top of the generated file.");
            continue;
        }
        if (b.depth < 0) {
            result.errors.push_back("Block " + std::to_string(i + 1) + " has a negative indent. Move it back to the left edge.");
        }

        if (b.type == BlockType::Else || b.type == BlockType::ElseIf) {
            while (!scopes.empty() && scopes.back().depth > d) {
                close_scope(scopes.back());
                scopes.pop_back();
            }
            const bool previousBranch = !scopes.empty() && scopes.back().depth == d &&
                (scopes.back().type == BlockType::If || scopes.back().type == BlockType::ElseIf);
            if (!previousBranch) {
                result.errors.push_back("Block " + std::to_string(i + 1) + " (" + block_name(b.type) + ") needs a matching If or Else if directly before it at the same indentation. Move it to the matching branch level.");
                line(pad + "// ERROR: branch has no matching If", owner);
                continue;
            }
            if (b.type == BlockType::Else) {
                line(std::string(static_cast<size_t>(d + 1) * 4, ' ') + "} else {", owner);
                scopes.back().type = BlockType::Else;
            } else {
                line(std::string(static_cast<size_t>(d + 1) * 4, ' ') + "} else if (" + b.value + ") {", owner);
                scopes.back().type = BlockType::ElseIf;
            }
            scopes.back().blockIndex = owner;
            continue;
        }

        close_to(d);
        if (d > static_cast<int>(scopes.size())) {
            result.errors.push_back("Block " + std::to_string(i + 1) + " (" + block_name(b.type) + ") is indented too far. Only " +
                std::to_string(scopes.size()) + " open block(s) contain it. Add an If/Repeat/For loop/While above it, or snap it left.");
        } else if (d != static_cast<int>(scopes.size())) {
            result.errors.push_back("Block " + std::to_string(i + 1) + " has an indentation gap. Snap it to the next grid column or use Outdent.");
        }

        if (opens_scope(b.type) && b.type != BlockType::Else && b.type != BlockType::ElseIf) {
            std::string opening;
            switch (b.type) {
                case BlockType::If: opening = "if (" + b.value + ") {"; break;
                case BlockType::Repeat: {
                    ++repeatId;
                    const std::string id = "cpp_lab_repeat_" + std::to_string(repeatId);
                    opening = "for (int " + id + " = 0; " + id + " < (" + b.value + "); ++" + id + ") {";
                    break;
                }
                case BlockType::ForLoop: opening = "for (" + b.value + ") {"; break;
                case BlockType::While: opening = "while (" + b.value + ") {"; break;
                default: break;
            }
            line(pad + opening, owner);
            scopes.push_back({d, b.type, owner});
            continue;
        }

        switch (b.type) {
            case BlockType::PrintText:
                line(pad + "std::cout << " + cpp_string_literal(b.value) + " << '\\n';", owner);
                break;
            case BlockType::PrintExpression:
                line(pad + "std::cout << (" + b.value + ") << '\\n';", owner);
                break;
            case BlockType::DeclareInt:
                line(pad + "int " + b.value + " = " + (b.extra.empty() ? "0" : b.extra) + ";", owner);
                break;
            case BlockType::DeclareDouble:
                line(pad + "double " + b.value + " = " + (b.extra.empty() ? "0.0" : b.extra) + ";", owner);
                break;
            case BlockType::DeclareText:
                line(pad + "std::string " + b.value + " = " + cpp_string_literal(b.extra) + ";", owner);
                break;
            case BlockType::DeclareBool:
                line(pad + "bool " + b.value + " = " + (b.extra.empty() ? "false" : b.extra) + ";", owner);
                break;
            case BlockType::DeclareChar:
                line(pad + "char " + b.value + " = " + cpp_char_literal(b.extra) + ";", owner);
                break;
            case BlockType::DeclareIntList:
                line(pad + "std::vector<int> " + b.value + " = {" + b.extra + "};", owner);
                break;
            case BlockType::ShowVariable:
                line(pad + "std::cout << " + b.value + " << '\\n';", owner);
                break;
            case BlockType::SetVariable:
                line(pad + b.value + " = " + b.extra + ";", owner);
                break;
            case BlockType::ReadInput:
                line(pad + "std::cin >> " + b.value + ";", owner);
                break;
            case BlockType::Break:
                if (!in_loop()) result.errors.push_back("Block " + std::to_string(i + 1) + " (Break) must be nested inside a Repeat, For loop, or While loop.");
                line(pad + "break;", owner);
                break;
            case BlockType::Continue:
                if (!in_loop()) result.errors.push_back("Block " + std::to_string(i + 1) + " (Continue) must be nested inside a Repeat, For loop, or While loop.");
                line(pad + "continue;", owner);
                break;
            case BlockType::Return:
                line(pad + "return " + (b.value.empty() ? "0" : b.value) + ";", owner);
                break;
            case BlockType::RawStatement: {
                std::istringstream raw(b.value);
                std::string rawLine;
                bool wrote = false;
                while (std::getline(raw, rawLine)) {
                    line(pad + rawLine, owner);
                    wrote = true;
                }
                if (!wrote) line(pad + "// (empty custom statement)", owner);
                break;
            }
            case BlockType::Comment: {
                std::istringstream note(b.value);
                std::string noteLine;
                bool wrote = false;
                while (std::getline(note, noteLine)) {
                    line(pad + "// " + noteLine, owner);
                    wrote = true;
                }
                if (!wrote) line(pad + "//", owner);
                break;
            }
            case BlockType::IncludeHeader:
            case BlockType::If:
            case BlockType::ElseIf:
            case BlockType::Else:
            case BlockType::Repeat:
            case BlockType::ForLoop:
            case BlockType::While:
            case BlockType::RawProgram:
                break;
            default:
                break; // Atomic tiles are handled by generate_atomic_cpp before this legacy generator.
        }
    }

    while (!scopes.empty()) {
        close_scope(scopes.back());
        scopes.pop_back();
    }
    line("    return 0;", -1);
    line("}", -1);
    result.code = out.str();
    return result;
}

inline std::vector<Block> example_program() {
    std::vector<Block> b;
    auto push = [&](BlockType t, int depth, std::string v = {}, std::string e = {}) {
        Block item = make_block(t);
        item.depth = depth;
        if (!v.empty() || t == BlockType::PrintText || t == BlockType::PrintExpression || t == BlockType::If ||
            t == BlockType::ElseIf || t == BlockType::Repeat || t == BlockType::ForLoop || t == BlockType::While ||
            t == BlockType::Comment || t == BlockType::ShowVariable || t == BlockType::Return || t == BlockType::IncludeHeader) item.value = std::move(v);
        if (!e.empty() || t == BlockType::DeclareInt || t == BlockType::DeclareDouble || t == BlockType::DeclareText ||
            t == BlockType::DeclareBool || t == BlockType::DeclareChar || t == BlockType::DeclareIntList || t == BlockType::SetVariable) item.extra = std::move(e);
        b.push_back(std::move(item));
    };
    push(BlockType::Comment, 0, "Try changing the starting score, then press Run.");
    push(BlockType::DeclareInt, 0, "score", "0");
    push(BlockType::PrintText, 0, "Let's learn by experimenting!");
    push(BlockType::PrintExpression, 0, "2 + 3 * 4");
    push(BlockType::Repeat, 0, "5");
    push(BlockType::SetVariable, 1, "score", "score + 1");
    push(BlockType::PrintText, 1, "The repeat body ran one time.");
    push(BlockType::ShowVariable, 0, "score");
    push(BlockType::If, 0, "score == 5");
    push(BlockType::PrintText, 1, "The test is true: five repeats happened.");
    push(BlockType::Else, 0);
    push(BlockType::PrintText, 1, "The test was false. Try changing the repeat count.");
    push(BlockType::PrintText, 0, "Now change a block, predict the result, and run it again.");
    return b;
}

inline bool self_test(std::string& report) {
    std::ostringstream log;
    int passed = 0;
    int failed = 0;
    auto check = [&](bool ok, const std::string& label) {
        log << (ok ? "[PASS] " : "[FAIL] ") << label << '\n';
        ok ? ++passed : ++failed;
    };

    const auto& catalog = block_catalog();
    check(catalog.size() >= 80, "the library contains atomic C++ syntax tiles plus beginner action blocks");
    bool descriptionsOk = true;
    for (const auto& info : catalog) {
        if (std::string(info.name).empty() || std::string(info.shortDescription).empty() ||
            std::string(info.longDescription).empty() || std::string(info.example).empty() || std::string(info.warning).empty() ||
            std::string(info.name).size() > 20 || std::string(info.shortDescription).size() > 48) descriptionsOk = false;
    }
    check(descriptionsOk, "every block has a short hover hint and a complete glossary description");

    bool tileLabelsOk = true;
    std::set<std::string> tileLabels;
    for (const auto& info : catalog) {
        const std::string label = tile_label(info.type);
        if (label.empty() || label.size() > 6 || label == "?" || !tileLabels.insert(label).second)
            tileLabelsOk = false;
    }
    check(tileLabelsOk && tileLabels.size() == catalog.size(),
          "all atomic tiles have unique compact labels for square grid cells");

    const auto atomicExample = atomic_example_program();
    const auto atomicGenerated = generate_cpp(atomicExample);
    check(atomicGenerated.errors.empty(), "the atomic starter program has no overlapping or incompatible tiles");
    check(atomicGenerated.code.find("#include <iostream>") != std::string::npos &&
          atomicGenerated.code.find("int main() {") != std::string::npos &&
          atomicGenerated.code.find("return 0;") != std::string::npos,
          "atomic grid positions generate a complete standalone C++ file");
    check(atomicGenerated.code.find("    int score = 0;") != std::string::npos &&
          atomicGenerated.code.find("if (score == 0) {") != std::string::npos &&
          atomicGenerated.code.find("std::cout << \"Change the Number tile and run again.\" << '\\n';") != std::string::npos,
          "the starter builds declarations, output, and conditions from separate word/symbol/value tiles");
    std::vector<Block> conditionTiles;
    auto putTile = [&](BlockType t, int x, int y, const std::string& value = {}) {
        Block b = make_block(t); b.gridX = x; b.gridY = y; if (!value.empty()) b.value = value; conditionTiles.push_back(b);
    };
    putTile(BlockType::TokInt, 0, 0); putTile(BlockType::TokMain, 1, 0); putTile(BlockType::TokLParen, 2, 0);
    putTile(BlockType::TokRParen, 3, 0); putTile(BlockType::TokLBrace, 4, 0);
    putTile(BlockType::TokIf, 0, 1); putTile(BlockType::TokLParen, 1, 1); putTile(BlockType::TokIdentifier, 2, 1, "score");
    putTile(BlockType::TokEqual, 3, 1); putTile(BlockType::TokNumber, 4, 1, "5"); putTile(BlockType::TokRParen, 5, 1);
    putTile(BlockType::TokLBrace, 6, 1); putTile(BlockType::TokReturn, 0, 2); putTile(BlockType::TokNumber, 1, 2, "0");
    putTile(BlockType::TokSemicolon, 2, 2); putTile(BlockType::TokRBrace, 0, 3); putTile(BlockType::TokRBrace, 0, 4);
    const auto conditionCode = generate_cpp(conditionTiles);
    check(conditionCode.errors.empty() && conditionCode.code.find("if (score == 5) {") != std::string::npos,
          "if, parentheses, identifier, comparison, number, and braces remain independent tiles");
    auto overlapping = atomicExample;
    overlapping.push_back(make_block(BlockType::TokNumber)); overlapping.back().gridX = 0; overlapping.back().gridY = 0;
    check(!generate_cpp(overlapping).errors.empty(), "overlapping grid tiles are explained before compilation");
    auto extraSymbol = make_block(BlockType::TokSymbol); extraSymbol.value = "<<="; extraSymbol.gridX = 3; extraSymbol.gridY = 0;
    check(atomic_token_text(extraSymbol) == "<<=", "the editable one-symbol tile covers less common C++ operators");
    Block actionNumber = make_block(BlockType::ActionSetNumber);
    actionNumber.value = "level"; actionNumber.extra = "17";
    check(action_source(actionNumber) == "int level = 17;",
          "Set number is one intentional beginner action with editable name and number fields");
    auto typeSlot = make_block(BlockType::TokTypeName); typeSlot.value = "std::vector<int>";
    auto nameSlot = make_block(BlockType::TokIdentifier); nameSlot.value = "scores";
    check(atomic_token_text(typeSlot) == "std::vector<int>" && atomic_token_text(nameSlot) == "scores",
          "Type name and Name are editable slots rather than a preset list of every possible value");
    auto expressionSlot = make_block(BlockType::TokExpression); expressionSlot.value = "score * 2 + 1";
    check(atomic_token_text(expressionSlot) == "score * 2 + 1",
          "Expression is a reusable editable value slot while Number remains independently editable");

    auto example = example_program();
    auto generated = generate_cpp(example);
    check(generated.errors.empty(), "the built-in example has valid block nesting");
    check(generated.code.find("int score = 0;") != std::string::npos, "whole-number variables generate C++ declarations");
    check(generated.code.find("for (int cpp_lab_repeat_1 = 0;") != std::string::npos, "Repeat becomes a real C++ for loop");
    check(generated.code.find("} else {") != std::string::npos, "Else connects to its matching If");
    check(generated.code.find("std::cout << (2 + 3 * 4) << '\\n';") != std::string::npos, "math expressions are preserved for the C++ compiler");
    check(generated.code.find("Let's learn by experimenting!") != std::string::npos, "text output is included in generated source");
    check(generated.lineOwners.size() == static_cast<size_t>(std::count(generated.code.begin(), generated.code.end(), '\n')), "every generated source line has an owner entry");

    auto escaped = make_block(BlockType::PrintText);
    escaped.value = "quote: \"hello\"\\ path\nsecond line";
    auto escapedCode = generate_cpp({escaped});
    check(escapedCode.code.find("\\\"hello\\\"") != std::string::npos, "quotes inside message text are escaped");
    check(escapedCode.code.find("\\n") != std::string::npos, "newlines inside message text become safe C++ escapes");
    check(escapedCode.lineOwners.size() == static_cast<size_t>(std::count(escapedCode.code.begin(), escapedCode.code.end(), '\n')),
        "embedded newlines do not break source line-to-block mapping");

    auto orphan = make_block(BlockType::PrintText);
    orphan.depth = 1;
    check(!generate_cpp({orphan}).errors.empty(), "a block cannot be indented without an open scope");
    check(!generate_cpp({make_block(BlockType::Else)}).errors.empty(), "Else without a matching If is explained before compilation");
    check(!generate_cpp({make_block(BlockType::ElseIf)}).errors.empty(), "Else if without a matching If is explained before compilation");
    auto nested = std::vector<Block>{make_block(BlockType::If), make_block(BlockType::Repeat), make_block(BlockType::PrintText)};
    nested[1].depth = 1; nested[2].depth = 2;
    check(generate_cpp(nested).errors.empty(), "nested If and Repeat scopes compile structurally");
    nested[2].depth = 3;
    check(!generate_cpp(nested).errors.empty(), "indentation gaps produce a beginner-facing error");

    auto stringVar = make_block(BlockType::DeclareText);
    stringVar.value = "player"; stringVar.extra = "Casey";
    check(generate_cpp({stringVar}).code.find("std::string player = \"Casey\";") != std::string::npos, "text variables use std::string and escaped text");
    auto boolVar = make_block(BlockType::DeclareBool);
    check(generate_cpp({boolVar}).code.find("bool ready = true;") != std::string::npos, "bool variables produce native true/false values");
    auto charVar = make_block(BlockType::DeclareChar);
    check(generate_cpp({charVar}).code.find("char grade = 'A';") != std::string::npos, "char variables safely quote one character");
    auto list = make_block(BlockType::DeclareIntList);
    auto listCode = generate_cpp({list});
    check(listCode.code.find("#include <vector>") != std::string::npos && listCode.code.find("std::vector<int> scores = {10, 20, 30};") != std::string::npos,
        "integer lists automatically include the vector library");
    auto header = make_block(BlockType::IncludeHeader); header.value = "<cmath>";
    check(generate_cpp({header}).code.find("#include <cmath>") != std::string::npos, "Include blocks add file-level headers");
    auto whileBlock = make_block(BlockType::While); whileBlock.value = "score < 3";
    check(generate_cpp({whileBlock}).code.find("while (score < 3) {") != std::string::npos, "While preserves the condition");
    auto forBlock = make_block(BlockType::ForLoop);
    check(generate_cpp({forBlock}).code.find("for (int i = 0; i < 5; ++i) {") != std::string::npos, "For loop accepts the native C++ loop header");
    auto read = make_block(BlockType::ReadInput); read.value = "age";
    check(generate_cpp({make_block(BlockType::DeclareInt), read}).code.find("std::cin >> age;") != std::string::npos, "Read input generates standard input code");
    auto loopWithBreak = std::vector<Block>{make_block(BlockType::While), make_block(BlockType::If), make_block(BlockType::Break)};
    loopWithBreak[1].depth = 1; loopWithBreak[2].depth = 2;
    check(generate_cpp(loopWithBreak).errors.empty(), "Break is allowed inside a loop, even within an If");
    check(!generate_cpp({make_block(BlockType::Break)}).errors.empty(), "Break outside a loop is caught before compilation");
    check(!generate_cpp({make_block(BlockType::Continue)}).errors.empty(), "Continue outside a loop is caught before compilation");

    auto chain = std::vector<Block>{make_block(BlockType::If), make_block(BlockType::PrintText), make_block(BlockType::ElseIf), make_block(BlockType::PrintText), make_block(BlockType::Else), make_block(BlockType::PrintText)};
    chain[1].depth = 1; chain[3].depth = 1; chain[5].depth = 1;
    auto chainCode = generate_cpp(chain);
    check(chainCode.errors.empty() && chainCode.code.find("} else if (score == 0) {") != std::string::npos,
        "If / Else if / Else form one valid branch chain");

    auto statement = make_block(BlockType::RawStatement); statement.value = "auto doubled = 2 * 2;\nstd::cout << doubled << '\\n';";
    auto statementCode = generate_cpp({statement});
    check(statementCode.code.find("auto doubled = 2 * 2;\n    std::cout << doubled") != std::string::npos,
        "custom statements preserve multiline C++ inside main");
    auto rawProgram = make_block(BlockType::RawProgram); rawProgram.value = "#include <iostream>\nint main() { return 0; }";
    auto rawCode = generate_cpp({rawProgram});
    check(rawCode.errors.empty() && rawCode.code.find("int main() { return 0; }") != std::string::npos && rawCode.code.find("Built by C++ Circuit Lab") == std::string::npos,
        "full-file block generates an exact standalone C++ program");
    check(!generate_cpp({rawProgram, make_block(BlockType::PrintText)}).errors.empty(), "full-file block warns when mixed with visual blocks");

    const std::string importedText = "#include <iostream>\nint main() {\n    int count = 2;\n    std::cout << \"count=\\n\" << count << '\\n';\n    return 0;\n}\n";
    auto importedTiles = tokenize_cpp_source(importedText);
    check(!importedTiles.empty() && importedTiles.size() > 25, "C++ loading tokenizes the file into individual syntax tiles");
    check(std::all_of(importedTiles.begin(), importedTiles.end(), [](const Block& b) {
        return is_atomic_mode_type(b.type) && b.gridX >= 0 && b.gridY >= 0;
    }), "imported C++ tokens all have valid grid locations");
    auto importedGenerated = generate_cpp(importedTiles);
    check(importedGenerated.errors.empty() && importedGenerated.code.find("#include <iostream>") != std::string::npos &&
          importedGenerated.code.find("int main() {") != std::string::npos &&
          importedGenerated.code.find("std::cout << \"count=\\n\"") != std::string::npos,
          "tokenized imported C++ can be regenerated into compilable source");
    auto bomTokens = tokenize_cpp_source(std::string("\xEF\xBB\xBF") + "int main() { return 0; }\n");
    check(!bomTokens.empty() && bomTokens.front().type == BlockType::TokInt,
          "loading a UTF-8 BOM does not create a junk block");

    log << "\n" << passed << " passed, " << failed << " failed.\n";
    report = log.str();
    return failed == 0;
}

} // namespace cpplab
