#ifndef RAPIDCSV_READER_HPP
#define RAPIDCSV_READER_HPP

#include "iterator/iterator.hpp"

namespace rapidcsv {
    namespace read {

        // Reader interface
        template <typename T>
        class Reader {
            using iterator::Iterator;
            using iterator::IteratorBase;
        protected:
            explicit Reader(): _end_iterator(Iterator<T>::end_iterator()), _iterator(this) {}

        public:
            virtual bool has_next() const {
                return begin() != end();
            }

            virtual T next() = 0;

            virtual IteratorBase<T>& begin() {
                return _iterator;
            }

            virtual const IteratorBase<T>& begin() const {
                return _iterator;
            }

            virtual IteratorBase<T>& end() {
                return _end_iterator;
            }

            virtual const IteratorBase<T>& end() const {
                return _end_iterator;
            }

            virtual ~Reader<T>() {}

        protected:
            virtual const T& peek() const {
                return *begin();
            }

        private:
            Iterator<T> &_end_iterator, _iterator;
        };
    }
}

#endif //RAPIDCSV_READER_HPP
