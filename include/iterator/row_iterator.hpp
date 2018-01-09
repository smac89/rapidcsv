#ifndef RAPIDCSV_ROW_ITERATOR_HPP
#define RAPIDCSV_ROW_ITERATOR_HPP

#include <string>
#include <vector>
#include "iterator.hpp"

namespace rapidcsv {
    namespace read {
        namespace iterator {
            class CSVRowIterator: public CSVIterator<std::vector<std::string>> {
                using CSVIterator::CSVIterator;
            };
        }
    }
}

#endif //RAPIDCSV_ROW_ITERATOR_HPP
