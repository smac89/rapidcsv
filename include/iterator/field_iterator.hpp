#ifndef RAPIDCSV_FIELD_ITERATOR_HPP
#define RAPIDCSV_FIELD_ITERATOR_HPP

#include <string>
#include "csv_except.hpp"
#include "iterator.hpp"

namespace rapidcsv {
    namespace read {
        namespace iterator {
            class CSVFieldIterator: public CSVIterator<std::string> {
                using CSVIterator::CSVIterator;
            };
        }
    }
}

#endif //RAPIDCSV_FIELD_ITERATOR_HPP
