#ifndef RAPIDCSV_FIELD_ITERATOR_HPP
#define RAPIDCSV_FIELD_ITERATOR_HPP

#include <string>
#include "csv_except.hpp"
#include "iterator.hpp"

namespace rapidcsv {
    namespace iterator {
        class CSVFieldIterator: public Iterator<std::string> {
            using Iterator::Iterator;
        };
    }
}

#endif //RAPIDCSV_FIELD_ITERATOR_HPP
