# C++ Circuit Lab — block index and glossary

This is the reference for the blocks in the C++ Circuit Lab palette. In the app, every Glossary entry displays the exact square tile used in the main palette—same compact label, color, shape, and size—beside the longer explanation, example, and warning. The glossary tile is a visual key only and has no separate hover description.

## How this block system works

- **Syntax tile:** one C++ word or symbol. `if`, `(`, a name, `==`, a number, `)`, and `{` are separate pieces.
- **Reusable value tile:** one editable kind of value. The single **Number** tile can hold `5`, `100`, or `3.14`; each copy can be edited differently. The same applies to **Name**, **Type name**, **Text**, and **Expression**. The library does not need a separate tile for every possible value.
- **Beginner action tile:** one clearly named action such as **Set number** or **Print text**. These are optional shortcuts; the underlying C++ can also be assembled from syntax tiles.

Example of a line made from individual tiles:

`if` + `(` + `Name: score` + `==` + `Number: 5` + `)` + `{`

Each horizontal grid row becomes one source line. Grid columns put tiles in left-to-right order. C++ braces `{` and `}` define groups; the code generator indents those groups in the preview.

The C++ language is large, and standards add new keywords over time. Dedicated tiles cover commonly used words/operators/punctuation; the editable **C++ keyword**, **Type name**, **Name**, **Expression**, and **Symbol** tiles let you enter spellings that do not need their own preset tile. These reusable slots cover broad language needs without thousands of nearly identical blocks.


## Reusable value tiles

### Name (`name`)

**Quick hint:** A name you choose.

Use this editable value tile for a variable, function, class, or your own identifier. Every copy can hold a different name; double-click the placed tile to edit it.


**Example**

```cpp
score
```


**Watch out:** Names cannot contain spaces and cannot be C++ keywords.


### Type name (`tname`)

**Quick hint:** A type such as int or MyType.

Use this editable slot where C++ expects a type. Examples include int, double, std::string, std::vector<int>, or a class you created. For complex types, you can also assemble the type from smaller tiles.


**Example**

```cpp
std::vector<int> scores;
```


**Watch out:** The type must exist and any needed library header must be included.


### Expression (`expr`)

**Quick hint:** A value or calculation you enter.

This is a reusable expression slot, not a whole statement. Type a value, variable, function call, or calculation such as score + 1. Use smaller operator/name/number tiles when you want to build the expression piece by piece.


**Example**

```cpp
score + 1
```


**Watch out:** Do not put a whole statement or trailing semicolon here.


### C++ keyword (`kw`)

**Quick hint:** A reusable language-word slot.

Use this editable tile for C++ keywords that do not have their own tile, including constexpr, extern, friend, unsigned, long, operator, requires, concept, co_await, and newer language words. Each copy can contain a different keyword.


**Example**

```cpp
constexpr
```


**Watch out:** Enter one keyword only; this is not for a variable name.


### Number (`num`)

**Quick hint:** A number you can edit.

This is one reusable number tile. Double-click it and type any numeric literal you need, such as 0, 42, -3, 3.14, or 0xFF; you do not need a separate block for every possible number.


**Example**

```cpp
42
```


**Watch out:** Do not add quotation marks unless you want text. A minus sign can also be its own tile.


### Text (`txt`)

**Quick hint:** Text that becomes a quoted value.

Type any words in this reusable Text tile. The editor adds safe double quotes for C++, so you do not need separate tiles for every phrase.


**Example**

```cpp
"Hello!"
```


**Watch out:** Text values use double quotes; the editor adds them for you.


### Character (`chr`)

**Quick hint:** One character as a value.

Enter one character, such as A or ?, in this reusable value tile. The editor surrounds it with C++ single quotes.


**Example**

```cpp
'A'
```


**Watch out:** Use Text for a word or sentence; Character is for one character.


### Symbol (`sym`)

**Quick hint:** An extra one-piece symbol.

Use this editable tile for a C++ operator or punctuator without a dedicated tile, such as &=, <<=, ..., .* or ->*. Enter exactly one C++ symbol token.


**Example**

```cpp
score <<= 1;
```


**Watch out:** Enter one symbol token only. Use the dedicated tiles for common operators to get their usual spacing automatically.


## One-purpose beginner actions

### Set number (`set#`)

**Quick hint:** Create a named number.

This action creates one integer variable. It is a single beginner action, unlike the tiny syntax tiles above.


**Example**

```cpp
int score = 0;
```


**Watch out:** Choose a name without spaces. Create it before you use it.


### Set text (`setT`)

**Quick hint:** Create a text value.

Creates one std::string variable and safely puts quotation marks around the text you enter.


**Example**

```cpp
std::string message = "Hello";
```


**Watch out:** A text value and a number are different types.


### Print text (`print`)

**Quick hint:** Show words on screen.

Prints the message you enter. This is one clear action; it does not hide other control-flow syntax.


**Example**

```cpp
std::cout << "Hello" << '\n';
```


**Watch out:** Use Show value to print a variable or calculation instead.


### Show value (`show`)

**Quick hint:** Display a variable's value.

Prints the current value of the named variable or expression.


**Example**

```cpp
std::cout << score << '\n';
```


**Watch out:** A variable name has no quotation marks around it.


### Read number (`read`)

**Quick hint:** Read a number from input.

Reads one numeric value into an existing integer variable from standard input.


**Example**

```cpp
std::cin >> score;
```


**Watch out:** Create the destination variable first.


### Change number (`change`)

**Quick hint:** Update a number.

Adds the amount you choose to an existing integer variable. This is useful for scores and loop counters.


**Example**

```cpp
score += 1;
```


**Watch out:** The variable must already have been created.


### Comment (`note`)

**Quick hint:** Leave a note in the code.

Adds a // comment line for people reading the program. It does not change how the program runs.


**Example**

```cpp
// Try changing score
```


**Watch out:** A comment is not displayed to the person running the program.


## Individual C++ syntax tiles

### include (`inc`)

**Quick hint:** Names a header to include.

This tile contributes only the word include. Place a separate # tile immediately before it, then add <, the header name, and > as individual tiles.


**Example**

```cpp
# include < iostream >
```


**Watch out:** The # and include tiles must be next to each other at the start of a source line.


### int (`int`)

**Quick hint:** Whole-number type.

The keyword int means a variable stores a whole number. It is only the type word; add a name, =, a starting number, and ; as separate tiles.


**Example**

```cpp
int score = 0;
```


**Watch out:** A declaration needs a variable name and usually ends with a semicolon.


### double (`dbl`)

**Quick hint:** Decimal-number type.

The keyword double stores numbers that can have a fractional part. This block contributes only the word double.


**Example**

```cpp
double height = 1.75;
```


**Watch out:** Decimal values can have tiny rounding differences.


### string (`str`)

**Quick hint:** Text type name.

Use this token in std::string. It is just one type name; std and :: are separate pieces too.


**Example**

```cpp
std::string name = "Ada";
```


**Watch out:** std::string requires the string header.


### bool (`bool`)

**Quick hint:** True-or-false type.

A bool variable can contain true or false. This tile contributes only the type keyword.


**Example**

```cpp
bool ready = true;
```


**Watch out:** true and false are not surrounded by quotes.


### char (`char`)

**Quick hint:** Single-character type.

The char type holds one character, usually written with single quotes.


**Example**

```cpp
char grade = 'A';
```


**Watch out:** Use std::string for words or sentences.


### auto (`auto`)

**Quick hint:** Let C++ infer a type.

auto asks C++ to work out a variable's type from the value assigned to it.


**Example**

```cpp
auto answer = 42;
```


**Watch out:** C++ still needs enough information to determine the type.


### if (`if`)

**Quick hint:** Starts a condition.

This tile is only the word if. Add (, a test such as score < 5, ), {, the body, and } as separate tiles.


**Example**

```cpp
if (score < 5) { ... }
```


**Watch out:** if needs a condition in parentheses.


### else (`else`)

**Quick hint:** Runs when the test is false.

else begins an alternative branch. It is a separate word; use } before it and { after it as separate tiles.


**Example**

```cpp
} else {
```


**Watch out:** else must match a previous if and cannot have its own condition.


### for (`for`)

**Quick hint:** Starts a counted loop.

for repeats code using a start action, a test, and an update. Each punctuation mark is a separate tile.


**Example**

```cpp
for (int i = 0; i < 5; ++i) { ... }
```


**Watch out:** The loop header has two semicolons inside the parentheses.


### while (`while`)

**Quick hint:** Repeats while a test is true.

while checks its condition before each repetition. You build the parentheses and body with separate blocks.


**Example**

```cpp
while (score < 5) { ... }
```


**Watch out:** Make sure the condition can eventually become false.


### return (`ret`)

**Quick hint:** Leaves a function.

return ends the current function, optionally providing a value to the caller.


**Example**

```cpp
return 0;
```


**Watch out:** The returned value must match the function's return type.


### main (`main`)

**Quick hint:** Program entry function name.

The operating system starts a normal C++ console program by calling main. Build int main ( ) { ... } from separate tiles.


**Example**

```cpp
int main() { return 0; }
```


**Watch out:** A program should have exactly one main function.


### std (`std`)

**Quick hint:** Standard-library namespace.

std contains much of the C++ standard library. The double-colon tile is separate.


**Example**

```cpp
std::cout
```


**Watch out:** Spelling and capitalization matter.


### cout (`cout`)

**Quick hint:** Writes to program output.

std::cout is the standard output stream. Combine it with << and a value or text.


**Example**

```cpp
std::cout << score;
```


**Watch out:** Use std::cout, or provide an appropriate using declaration.


### cin (`cin`)

**Quick hint:** Reads program input.

std::cin reads values from the program's input stream using >>.


**Example**

```cpp
std::cin >> age;
```


**Watch out:** The destination variable must already exist and have a compatible type.


### true (`true`)

**Quick hint:** The true boolean value.

true is the boolean value meaning yes or on.


**Example**

```cpp
bool ready = true;
```


**Watch out:** Do not put quotes around true.


### false (`false`)

**Quick hint:** The false boolean value.

false is the boolean value meaning no or off.


**Example**

```cpp
bool ready = false;
```


**Watch out:** Do not put quotes around false.


### # (`#`)

**Quick hint:** Starts a preprocessor line.

The # symbol starts directives such as #define, #ifdef, and #pragma. Place it first on the grid row, then add the directive word as another tile.


**Example**

```cpp
#define LIMIT 10
```


**Watch out:** Preprocessor directives must begin a source line.


### ( (`(`)

**Quick hint:** Opens parentheses.

This tile starts a parenthesized group. It is commonly used after if, while, for, or a function name.


**Example**

```cpp
if (ready)
```


**Watch out:** Every opening parenthesis needs a matching closing parenthesis.


### ) (`)`)

**Quick hint:** Closes parentheses.

This tile ends the parenthesized group started by (.


**Example**

```cpp
if (ready)
```


**Watch out:** Every closing parenthesis should match an opening one.


### { (`{`)

**Quick hint:** Opens a code group.

An opening brace begins a block of code, such as a function, class, loop, or if body. The editor indents later lines inside it.


**Example**

```cpp
int main() {
```


**Watch out:** Every opening brace needs a matching closing brace.


### } (`}`)

**Quick hint:** Closes a code group.

A closing brace ends the current function, class, loop, or conditional body.


**Example**

```cpp
}
```


**Watch out:** Match it to an earlier opening brace.


### ; (`;`)

**Quick hint:** Ends a statement.

Many C++ statements and declarations end with a semicolon. This punctuation is its own tile.


**Example**

```cpp
int score = 0;
```


**Watch out:** A missing semicolon is a common beginner compiler error.


### = (`=`)

**Quick hint:** Stores a value.

A single equals sign assigns the right-hand value to the left-hand variable.


**Example**

```cpp
score = 10;
```


**Watch out:** Use == to compare for equality.


### + (`+`)

**Quick hint:** Adds or indicates a positive value.

The plus operator adds values. It can also mean unary positive in some expressions.


**Example**

```cpp
score + 1
```


**Watch out:** The values must support addition.


### == (`==`)

**Quick hint:** Tests whether values match.

The equality operator is true when its left and right values are equal.


**Example**

```cpp
score == 5
```


**Watch out:** Do not confuse == with =, which stores a value.


### < (`<`)

**Quick hint:** Tests whether left is smaller.

The less-than operator compares values, except immediately after #include where it opens a header name.


**Example**

```cpp
score < 5
```


**Watch out:** The same symbol is also used in templates and include headers.


### > (`>`)

**Quick hint:** Tests whether left is larger.

The greater-than operator compares values, except when closing an angle-bracket header or template.


**Example**

```cpp
score > 5
```


**Watch out:** Two > symbols together form the right-shift operator.


### << (`<<`)

**Quick hint:** Prints or shifts bits left.

With std::cout, << sends a value to the output stream. For integers, it can also shift bits left.


**Example**

```cpp
std::cout << score;
```


**Watch out:** Meaning depends on the types on each side.


### float (`flt`)

**Quick hint:** Another decimal type.

The keyword float is a decimal-number type that usually uses less precision than double.


**Example**

```cpp
float speed = 2.5f;
```


**Watch out:** A float literal often uses an f suffix, such as 2.5f.


### void (`void`)

**Quick hint:** Means no return value.

Use void for a function that does not send a value back to its caller.


**Example**

```cpp
void sayHi() { ... }
```


**Watch out:** A void function should not return a value.


### const (`const`)

**Quick hint:** Marks a value as unchangeable.

const says a variable cannot be assigned a new value after initialization.


**Example**

```cpp
const int limit = 10;
```


**Watch out:** A const variable needs an initial value.


### do (`do`)

**Quick hint:** Starts a do-while loop.

A do-while loop runs its body at least once, then checks its condition at the end.


**Example**

```cpp
do { ... } while (ready);
```


**Watch out:** A do-while loop ends with a semicolon after the condition.


### switch (`sw`)

**Quick hint:** Chooses among matching cases.

switch compares one value with case labels. Each keyword and punctuation mark is a separate tile.


**Example**

```cpp
switch (choice) { case 1: break; }
```


**Watch out:** Remember break when you do not want a case to continue into the next case.


### case (`case`)

**Quick hint:** Starts one switch option.

case marks a value to match inside a switch block.


**Example**

```cpp
case 1:
```


**Watch out:** A case normally ends with a colon.


### default (`def`)

**Quick hint:** Fallback switch option.

default runs when no case value matches, if execution reaches it.


**Example**

```cpp
default:
```


**Watch out:** A switch should not contain multiple default labels.


### break (`brk`)

**Quick hint:** Exits a loop or switch.

break immediately leaves the nearest loop or switch statement.


**Example**

```cpp
break;
```


**Watch out:** break must be inside a loop or switch.


### continue (`next`)

**Quick hint:** Skips to the next loop pass.

continue stops the current pass of the nearest loop and starts its next check/update.


**Example**

```cpp
continue;
```


**Watch out:** continue must be inside a loop.


### class (`class`)

**Quick hint:** Begins a class definition.

A class groups data and functions into a new type. The class body is built using braces and semicolons.


**Example**

```cpp
class Player { ... };
```


**Watch out:** A class definition usually ends with }; including the semicolon.


### struct (`struct`)

**Quick hint:** Begins a struct definition.

A struct defines a type whose members are public by default.


**Example**

```cpp
struct Point { int x; };
```


**Watch out:** Remember the semicolon after the closing brace.


### public (`pub`)

**Quick hint:** Opens public access.

In a class or struct, public makes the following members accessible from outside, until another access label.


**Example**

```cpp
public:
```


**Watch out:** Use a colon after this access keyword.


### private (`priv`)

**Quick hint:** Opens private access.

In a class, private hides following members from ordinary outside access.


**Example**

```cpp
private:
```


**Watch out:** Class members are private by default.


### protected (`prot`)

**Quick hint:** Opens protected access.

protected members can be used inside the class and its derived classes.


**Example**

```cpp
protected:
```


**Watch out:** Use this inside a class or struct body.


### template (`tmpl`)

**Quick hint:** Begins a template declaration.

Templates let functions and classes work with types or values supplied later.


**Example**

```cpp
template <typename T>
```


**Watch out:** A template declaration must be followed by a matching function or type definition.


### typename (`type`)

**Quick hint:** Names a template type parameter.

typename is commonly used when declaring a type parameter in a template.


**Example**

```cpp
template <typename T>
```


**Watch out:** The template parameter must be given a name.


### namespace (`ns`)

**Quick hint:** Groups names together.

A namespace groups related names and helps prevent name conflicts.


**Example**

```cpp
namespace tools { ... }
```


**Watch out:** A namespace definition uses braces.


### using (`using`)

**Quick hint:** Introduces a name or alias.

using can create a type alias or bring a name into a scope.


**Example**

```cpp
using Count = int;
```


**Watch out:** Avoid using namespace std in large projects; explicit std:: names are clearer for beginners.


### try (`try`)

**Quick hint:** Starts error-handling code.

try marks code that may throw an exception and can be followed by one or more catch blocks.


**Example**

```cpp
try { ... } catch (...) { ... }
```


**Watch out:** A try block needs at least one matching catch block.


### catch (`catch`)

**Quick hint:** Handles a thrown exception.

catch introduces code that handles an exception from a matching try block.


**Example**

```cpp
catch (const std::exception& e) { ... }
```


**Watch out:** catch must follow a try or another catch handler.


### throw (`throw`)

**Quick hint:** Signals an exception.

throw reports an error by sending an exception to a matching catch block, or out of the current function.


**Example**

```cpp
throw std::runtime_error("Oops");
```


**Watch out:** The exception type and any required header must be available.


### new (`new`)

**Quick hint:** Creates a dynamic object.

new requests storage for an object and returns a pointer to it. Modern C++ often prefers smart pointers or containers.


**Example**

```cpp
auto p = new int(3);
```


**Watch out:** Raw new requires careful ownership; do not forget the matching delete when appropriate.


### delete (`del`)

**Quick hint:** Releases a raw new object.

delete releases one object created using raw new.


**Example**

```cpp
delete p;
```


**Watch out:** Only delete a pointer from a matching new; never delete the same object twice.


### static (`stat`)

**Quick hint:** Gives a declaration static storage or meaning.

static has different uses in C++; one common use is a local variable that keeps its value between calls.


**Example**

```cpp
static int count = 0;
```


**Watch out:** Its meaning depends on where it appears.


### virtual (`virt`)

**Quick hint:** Enables polymorphic methods.

virtual lets a derived class replace a base-class method when called through a base pointer or reference.


**Example**

```cpp
virtual void draw();
```


**Watch out:** Virtual functions usually appear in a class declaration.


### override (`over`)

**Quick hint:** Marks an overridden method.

override asks the compiler to verify that this method actually replaces a virtual base-class method.


**Example**

```cpp
void draw() override;
```


**Watch out:** A misspelled or mismatched method will cause a compiler error, which is useful feedback.


### sizeof (`size`)

**Quick hint:** Asks for a type or object's size.

sizeof produces the size in bytes of a type or object.


**Example**

```cpp
sizeof(int)
```


**Watch out:** The result is a size, not the number of items in a container.


### endl (`endl`)

**Quick hint:** Ends a line and flushes output.

std::endl writes a newline and flushes the output stream. A simple '\n' is often enough.


**Example**

```cpp
std::cout << score << std::endl;
```


**Watch out:** It belongs in std::endl, not by itself.


### vector (`vec`)

**Quick hint:** A growable list type.

std::vector stores a sequence of items and can grow as needed. Add #include <vector> above main.


**Example**

```cpp
std::vector<int> scores;
```


**Watch out:** List positions start at zero. Include the vector header.


### nullptr (`null`)

**Quick hint:** A pointer to nothing.

nullptr represents a null pointer: it does not point to an object.


**Example**

```cpp
int* p = nullptr;
```


**Watch out:** It is not the same as the number zero in every context.


### [ (`[`)

**Quick hint:** Opens an index or capture.

Square brackets access an array/list item or introduce a lambda capture list.


**Example**

```cpp
scores[0]
```


**Watch out:** Match it with a closing bracket.


### ] (`]`)

**Quick hint:** Closes a bracket group.

Square brackets close an index or capture list.


**Example**

```cpp
scores[0]
```


**Watch out:** Match it with an opening bracket.


### , (`,`)

**Quick hint:** Separates items.

A comma separates function arguments, list items, and some declarations.


**Example**

```cpp
max(a, b)
```


**Watch out:** Do not replace commas with semicolons inside an argument list.


### : (`:`)

**Quick hint:** Punctuation for labels and cases.

A colon follows case, default, public, private, and other labels; it also appears in conditional expressions and range-based loops.


**Example**

```cpp
case 1:
```


**Watch out:** Its exact meaning depends on the surrounding syntax.


### ? (`qst`)

**Quick hint:** Starts a short if/else expression.

The conditional operator chooses one of two values based on a condition. It is often called the ternary operator.


**Example**

```cpp
ready ? 1 : 0
```


**Watch out:** A ? expression also needs a matching colon.


### . (`.`)

**Quick hint:** Accesses a member.

The dot accesses a member of an object, such as a field or method.


**Example**

```cpp
player.score
```


**Watch out:** Use -> instead when accessing through a pointer.


### :: (`::`)

**Quick hint:** Accesses a named scope.

The scope operator selects a name inside a namespace or class.


**Example**

```cpp
std::cout
```


**Watch out:** Type two colons with no space between them.


### -> (`->`)

**Quick hint:** Accesses through a pointer.

The arrow operator accesses a member through a pointer.


**Example**

```cpp
pointer->value
```


**Watch out:** The expression on the left must support pointer member access.


### - (`-`)

**Quick hint:** Subtracts or negates.

The minus operator subtracts one value from another or makes a value negative.


**Example**

```cpp
score - 1
```


**Watch out:** The values must support subtraction.


### * (`*`)

**Quick hint:** Multiplies or works with pointers.

The star multiplies numbers in an expression; in declarations it can also indicate a pointer type.


**Example**

```cpp
2 * 3
```


**Watch out:** The meaning depends on where it appears.


### / (`/`)

**Quick hint:** Divides one value by another.

The slash divides the left value by the right value.


**Example**

```cpp
10 / 2
```


**Watch out:** Integer divided by integer discards the fractional part.


### % (`%`)

**Quick hint:** Gets a remainder.

The remainder operator gives what is left after integer division.


**Example**

```cpp
10 % 3
```


**Watch out:** Both operands should be integer types.


### != (`!=`)

**Quick hint:** Tests whether values differ.

The not-equal operator is true when the two values are different.


**Example**

```cpp
score != 0
```


**Watch out:** Both sides must be comparable.


### <= (`<=`)

**Quick hint:** Tests smaller than or equal.

True when the left value is less than or equal to the right value.


**Example**

```cpp
score <= 5
```


**Watch out:** Both sides must be comparable.


### >= (`>=`)

**Quick hint:** Tests larger than or equal.

True when the left value is greater than or equal to the right value.


**Example**

```cpp
score >= 5
```


**Watch out:** Both sides must be comparable.


### && (`&&`)

**Quick hint:** Requires both tests true.

Logical AND is true only when both conditions are true.


**Example**

```cpp
ready && score > 0
```


**Watch out:** Both sides should be conditions or values convertible to true/false.


### || (`||`)

**Quick hint:** Needs either test true.

Logical OR is true when at least one condition is true.


**Example**

```cpp
ready || overrideEnabled
```


**Watch out:** Use && when both conditions must be true.


### ! (`!`)

**Quick hint:** Reverses true and false.

Logical NOT changes true to false and false to true.


**Example**

```cpp
!ready
```


**Watch out:** A single ! is different from !=.


### & (`&`)

**Quick hint:** Reference or bitwise AND.

The ampersand can create a reference in a declaration or perform bitwise AND between integers.


**Example**

```cpp
int& alias = score;
```


**Watch out:** Its meaning depends on its position.


### | (`|`)

**Quick hint:** Bitwise OR.

Bitwise OR combines bits in integer values.


**Example**

```cpp
flags | mask
```


**Watch out:** For true/false logic, use || instead.


### ^ (`^`)

**Quick hint:** Bitwise exclusive OR.

Bitwise XOR sets a bit when the matching input bits differ.


**Example**

```cpp
flags ^ mask
```


**Watch out:** This is not the exponent operator in C++.


### ~ (`~`)

**Quick hint:** Bitwise NOT.

Bitwise NOT flips each bit in an integer value.


**Example**

```cpp
~flags
```


**Watch out:** It acts on bits, not on true/false logic.


### >> (`>>`)

**Quick hint:** Reads input or shifts bits right.

With std::cin, >> reads a value. For integers, it can also shift bits right.


**Example**

```cpp
std::cin >> score;
```


**Watch out:** Meaning depends on the types on each side.


### ++ (`++`)

**Quick hint:** Adds one to a value.

The increment operator adds one to a changeable variable.


**Example**

```cpp
++score
```


**Watch out:** The variable must be writable.


### -- (`--`)

**Quick hint:** Subtracts one from a value.

The decrement operator subtracts one from a changeable variable.


**Example**

```cpp
--score
```


**Watch out:** The variable must be writable.


### += (`+=`)

**Quick hint:** Adds and stores.

Adds the right-hand value to the left-hand variable and stores the result.


**Example**

```cpp
score += 2;
```


**Watch out:** The left side must be writable.


### -= (`-=`)

**Quick hint:** Subtracts and stores.

Subtracts the right-hand value from the left-hand variable and stores the result.


**Example**

```cpp
score -= 2;
```


**Watch out:** The left side must be writable.


### *= (`*=`)

**Quick hint:** Multiplies and stores.

Multiplies the left-hand variable by the right-hand value and stores the result.


**Example**

```cpp
score *= 2;
```


**Watch out:** The left side must be writable.


### /= (`/=`)

**Quick hint:** Divides and stores.

Divides the left-hand variable by the right-hand value and stores the result.


**Example**

```cpp
score /= 2;
```


**Watch out:** Do not divide by zero.


### %= (`%=`)

**Quick hint:** Stores the remainder.

Calculates the remainder and stores it in the left-hand variable.


**Example**

```cpp
score %= 3;
```


**Watch out:** Both values should be integers and the right side cannot be zero.

