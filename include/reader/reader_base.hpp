#ifndef RAPIDCSV_READER_BASE_HPP
#define RAPIDCSV_READER_BASE_HPP

#include "iterator/iterator.hpp"

namespace rapidcsv {
    namespace read {

        // Reader interface
        template <typename T>
        class ReaderBase {
            using iterator::Iterator;
        protected:
            explicit ReaderBase(): _end_iterator(Iterator<T>::end_iterator()), _iterator(this) {}

        public:
            virtual bool has_next() const {
                return begin() != end();
            }

            virtual T next() = 0;

            virtual Iterator<T>& begin() {
                return _iterator;
            }

            virtual const Iterator<T>& begin() const {
                return _iterator;
            }

            virtual Iterator<T>& end() {
                return _end_iterator;
            }

            virtual const Iterator<T>& end() const {
                return _end_iterator;
            }

            virtual ~ReaderBase<T>() {}

        protected:
            virtual const T& peek() const {
                return *begin();
            }

        private:
            Iterator<T> &_end_iterator, _iterator;
        };
    }
}

#endif //RAPIDCSV_READER_BASE_HPP
