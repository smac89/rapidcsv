#include <iostream>
#include <vector>
#include <rapidcsv.hpp>

auto main() -> int {
    auto &doc = rapidcsv::load("msft.csv");

    std::vector<float> close = doc.GetColumn<float>("Close");
    std::cout << "Read " << close.size() << " values." << std::endl;

    auto volume = doc.GetCell<long long>("Volume", "2011-03-09");
    std::cout << "Volume " << volume << " on 2011-03-09." << std::endl;
}
