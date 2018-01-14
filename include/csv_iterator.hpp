#ifndef RAPIDCSV_CSV_ITERATOR_HPP
#define RAPIDCSV_CSV_ITERATOR_HPP

#include <memory>
#include "reader/reader.hpp"

namespace std {
    template <typename T>
    inline auto begin(std::shared_ptr<rapidcsv::read::Reader<T>>& reader_ptr) -> decltype(reader_ptr->begin()) {
        return reader_ptr->begin();
    }

    template <typename T>
    inline auto end(std::shared_ptr<rapidcsv::read::Reader<T>>& reader_ptr) -> decltype(reader_ptr->end()) {
        return reader_ptr->end();
    }
}

#endif //RAPIDCSV_CSV_ITERATOR_HPP
