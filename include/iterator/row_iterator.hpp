#ifndef RAPIDCSV_ROW_ITERATOR_HPP
#define RAPIDCSV_ROW_ITERATOR_HPP

#include <string>
#include <vector>
#include "iterator.hpp"

namespace rapidcsv {
    namespace iterator {
        class CSVRowIterator: public Iterator<std::vector<std::string>> {
            using Iterator::Iterator;
        };
    }
}

#endif //RAPIDCSV_ROW_ITERATOR_HPP
