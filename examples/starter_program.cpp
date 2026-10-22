#include <iostream>
#include <string>

int main() {
    // Built by C++ Circuit Lab. Change a block, then run again.
    // Try changing the starting score, then press Run.
    int score = 0;
    std::cout << "Let's learn by experimenting!" << '\n';
    std::cout << (2 + 3 * 4) << '\n';
    for (int cpp_lab_repeat_1 = 0; cpp_lab_repeat_1 < (5); ++cpp_lab_repeat_1) {
        score = score + 1;
        std::cout << "The repeat body ran one time." << '\n';
    }
    std::cout << score << '\n';
    if (score == 5) {
        std::cout << "The test is true: five repeats happened." << '\n';
    } else {
        std::cout << "The test was false. Try changing the repeat count." << '\n';
    }
    std::cout << "Now change a block, predict the result, and run it again." << '\n';
    return 0;
}
