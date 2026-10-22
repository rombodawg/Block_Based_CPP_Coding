# IMPORTANT NOTE:
I myself do not know how to code, and Vibe coded this program so I could use it to learn C++. If that bothers you 
DO NOT use this software. This software is fully open source, all of the files can be inspected for safety and quality. This program does not require the internet to work
and does not collect, or use any personal information in order to run. The only thing the program will ask for is the location of your "MSYS2" C++ compiler so it can compile the code you build.

The above message was written by a human, everything bellow this was written by AI.
____________________________________________________________
# C++ Circuit Lab

**Learn C++ by building programs from blocks instead of typing the source code.** This is a native Windows desktop application that shows the C++ produced by the grid, compiles it with your local C++ compiler, runs it, and lets you experiment.

## The basic idea

The block library is an index of the pieces you can use to assemble C++.

There are three kinds of blocks:

1. **Syntax tiles** each contribute one C++ word or symbol. For example, `if`, `(`, `)`, `{`, `}`, `==`, `=`, `;`, `int`, and `return` are separate tiles. One `if` tile does not secretly build the whole `if` statement.
2. **Reusable value tiles** represent a kind of value, not one fixed value. Edit a **Number** tile to enter `0`, `42`, `-3`, `3.14`, or another number. Edit a **Name** tile to enter `score`, `lives`, or `player`. Each copy can hold a different value. The same idea is used for **Type name**, **Expression**, **Text**, **Character**, **C++ keyword**, and **Symbol**.
3. **Beginner action tiles** such as **Set number**, **Set text**, **Print text**, **Show value**, **Read number**, and **Change number** each represent one clearly named action. These are optional shortcuts for when you do not want to assemble every small syntax tile yet. You can build their underlying code from individual tiles instead.

For example, to build `if (score == 5) {`, use separate blocks:

`if` · `(` · `Name: score` · `==` · `Number: 5` · `)` · `{`

Then add the code inside on the next grid row and use a separate `}` block to close the group. There is only one Number block in the library; edit its value instead of looking for a special “5” block.

## The workspace

- **Block library (left):** the reusable index. Value slots and beginner actions appear near the top; language words, punctuation, and operators follow. Hover for the short hint. Open **Glossary** to see a visual preview of the exact same square tile (same label, color, and shape) beside its detailed explanation, example, and warning. The glossary previews do not need their own hover descriptions.
- **Your program (middle):** click a library tile, then click a grid cell to place it. Shift-click a library tile to keep placing copies. You can also drag a tile directly onto the grid. Double-click a placed editable tile or select it and click **Edit** to change its value.
- **Grid layout:** one horizontal grid row becomes one generated C++ source line. Columns order the syntax pieces from left to right. Move tiles vertically to change line order or horizontally to change their order on a line. Columns do not create code indentation; braces `{` and `}` establish C++ code groups, and the generated source is indented to match those braces.
- **Real C++ code (right, top):** shows the code made from your blocks. Read this while learning what each piece means.
- **Output & feedback (right, bottom):** shows the compiler selected for Run, build/run stages, program output, and compiler errors.

The hand is empty when the app starts. After placing one tile, the hand becomes empty again unless you Shift-clicked to keep it. Press `Esc` or right-click to empty the hand.

## Value blocks: edit one reusable tile

| Tile | What you enter | Example value |
|---|---|---|
| **Name** | A variable, function, or type name you choose | `score` |
| **Type name** | A type, including a type you created | `int`, `std::string`, `std::vector<int>` |
| **Expression** | A value or calculation, not a whole statement | `score + 1` |
| **Number** | Any numeric literal | `42`, `3.14`, `0xFF` |
| **Text** | Any words; the editor adds double quotes | `Hello!` |
| **Character** | One character; the editor adds single quotes | `A` |
| **C++ keyword** | A language keyword without its own dedicated tile | `constexpr`, `unsigned`, `requires` |
| **Symbol** | One less-common operator or punctuation token | `<<=`, `...`, `.*` |

These are reusable templates. You do not need a separate block for every name, number, sentence, type, expression, or operator spelling. Put different values into separate copies when needed. Text and Character are kept distinct because C++ uses double quotes for text strings and single quotes for individual characters.

## What the library covers

The library includes common built-in types, control-flow keywords, function/class/template words, standard input/output names, booleans and null pointer values, parentheses/braces/brackets, separators, member-access symbols, arithmetic and comparison operators, logical and bitwise operators, shifts, and assignment shortcuts.

C++ has many keywords and less-common symbols across its standards. Dedicated tiles make the common pieces easy to find; the editable **C++ keyword**, **Type name**, **Name**, **Expression**, and **Symbol** tiles cover spellings that do not need their own preset tile. In other words, the library indexes the kinds of pieces you need, while editable value tiles avoid an enormous list of near-identical blocks.

## Try the starter example

The starter program is built from individual tiles. It produces:

```text
Change the Number tile and run again.
The score is zero.
```

Try changing the **Number** tile in `int score = 0;` from `0` to `5`. Then change the Number tile in the `if (score == 0)` test to `5` too. Run the program and see why the message changes. You can also swap one syntax tile at a time and intentionally make a compiler error to learn what it means.

## Run it on Windows

The Windows app is built from source on your computer. It does not bundle a precompiled `.exe` or download a compiler.

### Recommended: MSYS2 UCRT64

1. Install MSYS2 from <https://www.msys2.org/>.
2. Open **MSYS2 UCRT64** from the Start menu and install the compiler:

   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
   ```

3. Extract the project to a writable folder.
4. Double-click **`Start_Cpp_Circuit_Lab.bat`**. On first use, the console asks for your MSYS2 installation folder—the folder containing `ucrt64\bin`. Enter that folder, not the `bin` folder itself. The build script saves your choice in a local `CppCircuitLab.toolchain.txt` file so the launcher can reuse it next time. That machine-specific file is generated only on your computer and is excluded from the distributed ZIP. Press Enter or type `PATH` to use a compiler already discoverable through your Windows PATH.

You can run **`Build_Cpp_Circuit_Lab.bat`** directly whenever you want to rebuild or change the saved compiler location. Then use **`Start_Cpp_Circuit_Lab.bat`** to launch the app. There is no separate Run batch file anymore: Start handles launching and automatically calls Build when the executable is missing. Visual Studio C++ Build Tools can also be used from a suitable developer command prompt by choosing the PATH option if `cl.exe` is available there.

## Controls

| Action | Mouse | Keyboard |
|---|---|---|
| Run | Click **Run** | `F5` |
| Edit selected block | Click **Edit** or double-click an editable tile | `Enter` |
| Remove selected block | Click **Remove** | `Delete` |
| Undo | Click **Undo** | `Ctrl+Z` |
| Pick up a tile | Click a library tile | — |
| Place one tile | With a tile in hand, click a grid cell | — |
| Keep a tile in hand | Shift-click a library tile | — |
| Empty the hand | Right-click or press `Esc` | `Esc` |
| Move a placed tile | Drag it on the grid or use **Up / Down / Move left / Move right** | Toolbar shortcuts |
| Load starter experiment | Click **Example** | — |
| Detailed explanations | Click **Glossary** | `G` |
| Help | Click **Help** | `F1` |
| Save generated source | Click **Save C++** | `Ctrl+S` |
| Load a C++ source file | Click **Load C++** | — |
| Clear the entire grid | Click **Clear all**, then click it again within 2 seconds | — |
| Close an overlay | Click **Close** | `Esc` |


### Loading and clearing programs

- **Load C++** opens a `.cpp`, `.cc`, `.cxx`, `.h`, or `.hpp` file and creates one tile for each recognized word, value, or symbol. The right-hand code pane initially shows the original source exactly as it was read, including its original spacing and comments. It remains the source used by Run and Save until you edit the blocks. After a block edit, the right pane shows the newly generated code; **Undo** can restore the untouched loaded file.
- The import currently accepts files up to 1 MB and at most 20,000 tokens to keep the desktop interface responsive. The visual side is a token view, not a full C++ parser: unusual or advanced syntax is still preserved in the original source while unedited, but editing the tiles can normalize spacing or represent some unusual constructs less perfectly.
- **Clear all** is guarded against accidental clicks. Click it once and the status line says “Click again in quick succession to clear all the blocks”; click it again within 2 seconds to empty the grid. Use **Undo** to restore the prior program.

## Build and test the portable core

The code generator and block model can be tested without the Windows GUI using a C++17 compiler:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The tests cover the block catalogue, short and long descriptions, tile labels, the individual-piece starter program, overlap detection, reusable Number/Name/Type name/Expression slots, source generation, string escaping, and end-to-end compilation and execution.

## Safety note

When you press Run, Circuit Lab compiles and executes the C++ you created as your Windows user account. C++ programs can read or modify files, start other programs, or do anything your account is allowed to do. Only run code you trust. The short run timeout helps stop endless loops but is **not a security sandbox**.
