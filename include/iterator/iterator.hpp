#ifndef RAPIDCSV_ITERATOR_HPP
#define RAPIDCSV_ITERATOR_HPP

#include <string>
#include "reader/reader.hpp"
#include "csv_except.hpp"

namespace rapidcsv {
    namespace read {
        namespace iterator {
            template <typename T>
            class CSVIterator {
                bool has_value;
                static CSVIterator<T> iterator_empty;

            protected:
                Reader<T>* _parser;
                T value;
            public:
                CSVIterator(Reader<T>* parser): _parser(parser) {}

                virtual const T operator *() const {
                    if (!has_value) {
                        throw empty_iterator_exception();
                    }
                    return value;
                }

                virtual CSVIterator<T>& operator++ () {
                    if (!_parser->has_next()) {
                        if (has_value) {
                            has_value = false;
                            return iterator_empty;
                        }
                        throw past_the_end_iterator_exception();
                    }
                    value = std::move(_parser->next());
                    return *this;
                }

                virtual bool operator != (CSVIterator<T>& other) {
                    return this != &other;
                }

                static CSVIterator<T> end_iterator() {
                    return iterator_empty;
                }

                virtual ~CSVIterator() {}

            protected:
                CSVIterator(): _parser(nullptr) {}
            };

            template <typename T>
            CSVIterator<T> CSVIterator<T>::iterator_empty;
        }
    }
}

#endif //RAPIDCSV_ITERATOR_HPP
