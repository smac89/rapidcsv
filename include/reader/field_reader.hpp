#ifndef RAPIDCSV_FIELD_READER_HPP
#define RAPIDCSV_FIELD_READER_HPP

#include <string>
#include <utility>
#include "simple_reader.hpp"
#include "csv_except.hpp"

namespace rapidcsv {
    namespace read {

        using except::csv_unescaped_quote_exception;
        using except::csv_quote_inside_non_quote_field_exception;
        using except::csv_unterminated_quote_exception;

        template <typename _StreamT>
        class CSVFieldReader: public SimpleReader<std::string, _StreamT> {
        protected:
            std::string current;

        private:
            bool _start_quoted_field, _end_quoted_field,
                    _is_quoted_field, _is_return, _is_next_line, _is_comma;

        public:
            explicit CSVFieldReader(_StreamT &&begin, _StreamT &&end) :
                    SimpleReader(std::forward(begin), std::forward(end)) {
                reset();
            }

            std::string next() {
                if (!has_next()) {
                    throw csv_nothing_to_read_exception();
                }

                if (_is_return) {
                    if (*_begin == LF) {
                        _begin++;
                    }
                    _is_next_line = true;
                }

                if (_is_next_line) {
                    reset();
                    return std::string() + LF;
                }

                if (_is_comma && stream_empty()) {
                    reset();
                    return "";
                }
                reset();
                parseNext();
                return current;
            }

            bool has_next() const {
                return _is_next_line || _is_comma || _is_return || !stream_empty();
            }

        private:
            bool stream_empty() const {
                return !SimpleReader::has_next();
            }

            void parseNext() {
                while (!stream_empty()) {
                    char byte = *_begin++;
                    switch (byte) {
                        case '"':
                            if (current.empty()) {
                                _is_quoted_field = true;
                                _start_quoted_field = true;
                                _end_quoted_field = false;
                            } else if (!_is_quoted_field) {
                                throw csv_quote_inside_non_quote_field_exception();
                            } else if (!_end_quoted_field) {
                                _end_quoted_field = true;
                            } else {
                                _end_quoted_field = false;
                                continue;
                            }
                            current += byte;
                            break;

                        case ',':
                            if (!_is_quoted_field || _end_quoted_field) {
                                _is_comma = true;
                                return;
                            }
                            current += byte;
                            break;

                        case CR:
                            if (!_is_quoted_field || _end_quoted_field) {
                                _is_return = true;
                                return;
                            }
                            current += byte;
                            break;

                        case LF:
                            if (!_is_quoted_field || _end_quoted_field) {
                                _is_next_line = true;
                                return;
                            }
                            current += byte;
                            break;

                        default:
                            if (!current.empty() && _is_quoted_field && _end_quoted_field) {
                                throw csv_unescaped_quote_exception();
                            }
                            current += byte;
                    }
                }

                if (_is_quoted_field && !_end_quoted_field) {
                    throw csv_unterminated_quote_exception();
                }

                _is_next_line = true;
            }

            void reset() {
                _start_quoted_field = false;
                _end_quoted_field = true;
                _is_quoted_field = false;
                _is_return = false;
                _is_next_line = false;
                _is_comma = false;
                current.erase();
            }
        };
    }
}

#endif //RAPIDCSV_FIELD_READER_HPP
