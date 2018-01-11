#include <iostream>
#include <sstream>
#include <iterator>
#include <rapidcsv.hpp>

auto main() -> int {
    std::istringstream in("\"dsfdsfsdfdsf\",sdfsdfdsf\"\n");

    auto csvFieldReader = rapidcsv::fieldReader(
            std::istreambuf_iterator<char>{in},
            std::istreambuf_iterator<char>{});

    for (auto& field: csvFieldReader) {
        std::cout << "read: " << '[' << field << "]\n";
    }


    return 0;
}
