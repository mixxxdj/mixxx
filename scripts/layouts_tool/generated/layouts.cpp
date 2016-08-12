#include <iostream>

extern "C" void getLayout(std::string layoutName) {
    std::cout << "I will look for layout: " << layoutName << '\n';
}