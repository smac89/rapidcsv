#include <iostream>
#include <iterator>
#include <rapidcsv.hpp>

auto main() -> int {
    auto csvFieldReader = rapidcsv::fieldReader(
            std::istreambuf_iterator<char>{std::cin},
            std::istreambuf_iterator<char>{});

    for (auto& field: csvFieldReader) {
        std::cout << "read: " << '[' << field << "]\n";
    }

    return 0;
}
