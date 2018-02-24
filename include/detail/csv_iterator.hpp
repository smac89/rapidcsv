#ifndef RAPIDCSV_CSV_ITERATOR_HPP
#define RAPIDCSV_CSV_ITERATOR_HPP

#include <iterator>
#include "detail/reader/reader.hpp"

namespace std {
    template <typename T>
    inline auto begin(const rapidcsv::read::Reader<T>& reader_ptr)
        -> decltype(std::make_move_iterator(std::move(reader_ptr).begin())) {
        return std::make_move_iterator(std::move(reader_ptr).begin());
    }

    template <typename T>
    inline auto end(const rapidcsv::read::Reader<T>& reader_ptr)
        -> decltype(std::make_move_iterator(std::move(reader_ptr).end())) {
        return std::make_move_iterator(std::move(reader_ptr).end());
    }

//    template <typename T>
//    inline auto begin(rapidcsv::read::Reader<T> reader_ptr)
//        -> decltype(reader_ptr.begin()) {
//        return reader_ptr.begin();
//    }
//
//    template <typename T>
//    inline auto end(rapidcsv::read::Reader<T> reader_ptr)
//        -> decltype(reader_ptr.end()) {
//        return reader_ptr.end();
//    }
}

#endif //RAPIDCSV_CSV_ITERATOR_HPP
