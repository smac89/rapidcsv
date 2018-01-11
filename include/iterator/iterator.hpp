#ifndef RAPIDCSV_ITERATOR_HPP
#define RAPIDCSV_ITERATOR_HPP

#include <string>
#include <iterator>
#include "reader/reader.hpp"
#include "csv_except.hpp"

namespace rapidcsv {
    namespace read {
        namespace iterator {
            template <typename T>
            class CSVIterator: public std::iterator<std::forward_iterator_tag, T> {
                explicit CSVIterator(): _reader(nullptr) {}
            protected:
                Reader<T>* _reader;
                T value;
            public:
                explicit CSVIterator(Reader<T>* reader): _reader(reader) { }

                virtual const T operator *() {
                    if (nullptr == _reader) {
                        throw empty_iterator_exception();
                    } else if (!has_init) {
                        this->operator++();
                        has_init = true;
                    }
                    return value;
                }

                virtual CSVIterator<T>& operator++ () {
                    if (nullptr == _reader) {
                        throw past_the_end_iterator_exception();
                    }
                    if (_reader->has_next()) {
                        value = std::move(_reader->next());
                    } else {
                        _reader = nullptr;
                    }
                    return *this;
                }

                virtual bool operator != (CSVIterator<T>& other) {
                    return _reader != other._reader;
                }

                static CSVIterator<T> end_iterator() {
                    return iterator_empty;
                }

                virtual ~CSVIterator() {}

            private:
                bool has_init = false;
                static CSVIterator<T> iterator_empty;
            };

            template <typename T>
            CSVIterator<T> CSVIterator<T>::iterator_empty;
        }
    }
}

#endif //RAPIDCSV_ITERATOR_HPP
