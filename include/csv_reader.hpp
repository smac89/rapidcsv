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
    template <typename _StreamT>
    std::shared_ptr<read::Reader<std::string>> fieldReader(_StreamT begin, _StreamT end) {
        return std::make_shared<read::CSVFieldReader<_StreamT>>(std::move(begin), std::move(end));
    }

    template <typename _StreamT>
    std::shared_ptr<read::Reader<std::vector<std::string>>> row_reader(_StreamT begin, _StreamT end) {
        return std::make_shared<read::CSVRowReader<_StreamT>>(std::move(begin), std::move(end));
    }
}

#endif // RAPIDCSV_CSV_READER_HPP
