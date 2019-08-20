#include <util/exception/exception.h>

#include <iostream>
#include <string>

using namespace std::literals;

int main() {
    try {
        throw TException{"AAA"};
    } catch (const TException& exception) {
        std::cout << exception.what() << std::endl;
    } catch (...) {
        std::cout << "Wut?" << std::endl;
    }

    try {
        throw TException{"AAA", 3};
    } catch (const TException& exception) {
        std::cout << exception.what() << std::endl;
    } catch (...) {
        std::cout << "Wut?" << std::endl;
    }

    try {
        throw TException{"AAB"s};
    } catch (const TException& exception) {
        std::cout << exception.what() << std::endl;
    } catch (...) {
        std::cout << "Wut?" << std::endl;
    }
    return 0;
}