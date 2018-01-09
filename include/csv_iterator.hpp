#ifndef RAPIDCSV_CSV_ITERATOR_HPP
#define RAPIDCSV_CSV_ITERATOR_HPP

#include <memory>
#include <string>
#include "iterator/iterator.hpp"
#include "reader/reader.hpp"

using rapidcsv::read::iterator::CSVIterator;
using rapidcsv::read::Reader;

namespace std {
    template <class T>
    auto begin(std::unique_ptr<Reader<T>>& reader_ptr) -> decltype(reader_ptr->begin()) {
        return reader_ptr->begin();
    }

    template <class T>
    auto end(std::unique_ptr<Reader<T>>& reader_ptr) -> decltype(reader_ptr->end()) {
        return reader_ptr->end();
    }
}

#endif //RAPIDCSV_CSV_ITERATOR_HPP
