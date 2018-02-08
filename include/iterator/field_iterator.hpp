#ifndef RAPIDCSV_FIELD_ITERATOR_HPP
#define RAPIDCSV_FIELD_ITERATOR_HPP

#include <string>
#include "iterator.hpp"

namespace rapidcsv {
    namespace iter {
        class CSVFieldIterator: public Iterator<std::string> {
            using Iterator::Iterator;
        };
    }
}

#endif //RAPIDCSV_FIELD_ITERATOR_HPP
