#ifndef RAPIDCSV_READER_HPP
#define RAPIDCSV_READER_HPP

#include <string>
#include <utility>

namespace rapidcsv {
    namespace read {
        namespace iterator {
            template <typename T> class CSVIterator;
        }

        using iterator::CSVIterator;

        // Reader interface
        template <typename T>
        class Reader {
        protected:
            Reader(CSVIterator<T>&& _iterator, CSVIterator<T>&& _end_iterator):
                    iterator(_iterator), end_iterator(_end_iterator) {}

        public:
            virtual bool has_next() = 0;
            virtual T next() = 0;
            virtual ~Reader() {}

            virtual CSVIterator<T>& begin() {
                return iterator;
            }

            virtual CSVIterator<T>& end() {
                return end_iterator;
            }

            virtual const CSVIterator<T>& begin() const {
                return iterator;
            }

            virtual const CSVIterator<T>& end() const {
                return end_iterator;
            }

        protected:
            CSVIterator<T> &iterator, &end_iterator;
        };
    }
}

#endif //RAPIDCSV_READER_HPP
