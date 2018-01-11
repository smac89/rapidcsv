#ifndef RAPIDCSV_FIELD_READER_HPP
#define RAPIDCSV_FIELD_READER_HPP

#include <string>
#include <utility>
#include "reader.hpp"
#include "iterator/field_iterator.hpp"
#include "csv_except.hpp"
#include "csv_constants.hpp"

namespace rapidcsv {
    namespace read {

        using except::csv_unescaped_quote_exception;
        using except::csv_quote_inside_non_quote_field_exception;
        using except::csv_unterminated_quote_exception;
        using iterator::CSVFieldIterator;
        using iterator::CSVIterator;

        template <typename _StreamT>
        class CSVFieldReader: public Reader<std::string> {
        protected:
            std::string current;
            _StreamT _begin, _end;

        private:
            bool _start_quoted_field, _end_quoted_field, _is_quoted_field, _is_next_line;

        public:
            explicit CSVFieldReader(_StreamT &&begin, _StreamT &&end):
                    Reader(CSVFieldIterator(this), CSVIterator<std::string>::end_iterator()),
                    _begin(std::move(begin)), _end(std::move(end)) {
                reset();
            }

            std::string next() {
                if (_is_next_line) {
                    _is_next_line = false;
                    current = LF;
                    return current;
                }

                if (!has_next()) {
                    throw std::out_of_range("The reader has run out of bytes!");
                }
                reset();
                parseNext();
                return current;
            }

            bool has_next() const {
                return _begin != _end;
            }

        private:
            void parseNext() {
                if (_is_next_line) {
                    if (*_begin == LF) {
                        _begin++;
                    }
                    current = LF;
                    _is_next_line = false;
                    return;
                }

                while (_begin != _end) {
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
                                return;
                            }
                            current += byte;
                            break;

                        case CR:
                            if (!_is_quoted_field || _end_quoted_field) {
                                _is_next_line = true;
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
                _is_next_line = false;
                current.erase();
            }
        };
    }
}

#endif //RAPIDCSV_FIELD_READER_HPP
