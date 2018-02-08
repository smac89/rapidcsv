#ifndef RAPIDCSV_ROW_READER_HPP
#define RAPIDCSV_ROW_READER_HPP

#include <string>
#include <utility>
#include "field_reader.hpp"

namespace rapidcsv {
    namespace read {

        using VS = std::vector<std::string>;

        template <class _StreamT>
        class CSVRowReader: public Reader<VS> {
            CSVFieldReader<_StreamT> fieldReader;

        public:

            explicit CSVRowReader(_StreamT &&begin, _StreamT &&end):
                    fieldReader(std::forward<_StreamT>(begin), std::forward<_StreamT>(end)) {
            }

            bool has_next() const {
                return fieldReader.has_next();
            };

            auto next() -> VS {
                std::string rowSep(1, LF);
                VS row;
                for (std::string field; fieldReader.has_next(); row.emplace_back(field)) {
                    field = std::move(fieldReader.next());
                    if (field == rowSep) {
                        break;
                    }
                }
                return row;
            }
        };
    }
}

#endif //RAPIDCSV_ROW_READER_HPP
