#ifndef RAPIDCSV_CSV_READER_HPP
#define RAPIDCSV_CSV_READER_HPP

#include <memory>
#include <utility>
#include <istream>
#include <iterator>

#include "reader/reader.hpp"
#include "reader/field_reader.hpp"
#include "reader/row_reader.hpp"
#include "reader/simple_reader.hpp"

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

    auto row_reader(const std::istream& stream) -> std::shared_ptr<rapidcsv::read::Reader<std::vector<std::string>>> {
        return std::make_shared<rapidcsv::read::CSVRowReader<std::istreambuf_iterator<char>>>(
                std::istreambuf_iterator<char>{stream.rdbuf()},
                std::istreambuf_iterator<char>{});
    }

    auto row_reader(std::istream&& stream) -> std::shared_ptr<read::Reader<std::vector<std::string>>> {
        return std::make_shared<rapidcsv::read::CSVRowReader<std::istreambuf_iterator<char>>>(
                std::istreambuf_iterator<char>{std::move(stream).rdbuf()},
                std::istreambuf_iterator<char>{});
    }
}

#endif // RAPIDCSV_CSV_READER_HPP
