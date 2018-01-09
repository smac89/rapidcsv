#ifndef RAPIDCSV_CSV_READER_HPP
#define RAPIDCSV_CSV_READER_HPP

#include <memory>
#include <utility>

#include "reader/reader.hpp"
#include "reader/field_reader.hpp"
#include "reader/row_reader.hpp"

#if __cplusplus == 201103L || (defined(_MSC_VER) && _MSC_VER == 1900)
namespace std {
    template<typename T, typename... Args>
    constexpr std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

namespace rapidcsv {

    using read::Reader;
    using read::CSVFieldReader;
    using read::CSVRowReader;


    template <typename _StreamT>
    std::unique_ptr<Reader<std::string>> fieldReader(_StreamT&& begin, _StreamT&& end) {
        return std::make_unique<CSVFieldReader<_StreamT>>(std::forward<_StreamT>(begin), std::forward<_StreamT>(end));
    }

    template <typename _StreamT>
    std::unique_ptr<Reader<std::vector<std::string>>>&& rowReader(const _StreamT& begin, const _StreamT& end) {
        return std::make_unique<CSVRowReader<_StreamT>>(std::forward<_StreamT>(begin), std::forward<_StreamT>(end));
    }

//    template <typename T, typename _StreamT, typename _ReaderT>
//    std::unique_ptr<Reader<T>> reader(_StreamT&& begin, _StreamT&& end) {
//        return std::make_unique<_ReaderT>(std::forward<_StreamT>(begin), std::forward<_StreamT>(end));
//    };
}

#endif // RAPIDCSV_CSV_READER_HPP
