#ifndef RAPIDCSV_ROW_READER_HPP
#define RAPIDCSV_ROW_READER_HPP

#include <string>
#include <utility>
#include "reader.hpp"
#include "field_reader.hpp"
#include "iterator/row_iterator.hpp"
#include "csv_constants.hpp"

namespace rapidcsv {
    namespace read {

        using iterator::CSVIterator;
        using iterator::CSVRowIterator;

        using VS = std::vector<std::string>;

        template <class _StreamT>
        class CSVRowReader: public Reader<VS> {
            CSVFieldReader<_StreamT> fieldReader;
            CSVIterator<std::string> &_begin, &_end;

        public:

            explicit CSVRowReader(_StreamT &&begin, _StreamT &&end):
                    Reader(CSVRowIterator(this), CSVIterator<VS>::end_iterator()),
                    fieldReader(std::forward<_StreamT>(begin), std::forward<_StreamT>(end)),
                    _begin(fieldReader.begin()), _end(fieldReader.end()) {
            }

            bool has_next() const {
                return fieldReader.has_next();
            };

            auto next() -> VS {
                std::string lf = "" + LF;
                VS row;
                for (std::string field; _begin != _end;) {
                    field = *_begin;
                    ++_begin;
                    if (field != lf) {
                        row.emplace_back(field);
                    } else {
                        break;
                    }
                }
                return row;
            }
        };
    }
}

#endif //RAPIDCSV_ROW_READER_HPP
