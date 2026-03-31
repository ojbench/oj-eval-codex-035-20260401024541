#include "MyString.hpp"
#include <iostream>

int main() {
    // The OJ may provide commands. If no input, do nothing (no output).
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string cmd;
    if (!(std::cin >> cmd)) return 0;
    // Simple interpreter for testing
    MyString s;
    while (true) {
        if (cmd == "set") {
            std::string x; std::cin >> x;
            s = MyString(x.c_str());
        } else if (cmd == "append") {
            std::string x; std::cin >> x;
            s.append(x.c_str());
        } else if (cmd == "size") {
            std::cout << s.size() << "\n";
        } else if (cmd == "capacity") {
            std::cout << s.capacity() << "\n";
        } else if (cmd == "at") {
            size_t i; std::cin >> i;
            try { std::cout << s.at(i) << "\n"; }
            catch (const std::out_of_range&) { std::cout << "OOR\n"; }
        } else if (cmd == "resize") {
            size_t n; std::cin >> n; s.resize(n);
        } else if (cmd == "reserve") {
            size_t n; std::cin >> n; s.reserve(n);
        } else if (cmd == "print") {
            std::cout << s.c_str() << "\n";
        } else if (cmd == "quit") {
            break;
        }
        if (!(std::cin >> cmd)) break;
    }
    return 0;
}
