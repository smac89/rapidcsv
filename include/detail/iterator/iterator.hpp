#ifndef RAPIDCSV_ITERATOR_HPP
#define RAPIDCSV_ITERATOR_HPP

#include "detail/iterator/iterator_base.hpp"
#include "detail/reader/reader.hpp"
#include "detail/csv_except.hpp"

namespace rapidcsv {
    namespace iter {
        template <typename T>
        class Iterator: public IteratorBase<T> {
            using rapidcsv::read::Reader;

            explicit Iterator(): _reader(nullptr) {}
        protected:
            Reader<T>* _reader;
            T value;
        public:
            explicit Iterator(Reader<T>* reader): _reader(reader) {
                if (_reader != nullptr && !_reader->has_next()) {
                    _reader = nullptr;
                }
            }

            virtual const T operator *() {
                if (nullptr == _reader) {
                    throw empty_iterator_exception();
                } else if (!has_init) {
                    std::advance(*this, 1);
                    has_init = true;
                }
                return value;
            }

            virtual Iterator<T>& operator++ () {
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

            virtual bool operator != (Iterator<T>& other) noexcept {
                return _reader != other._reader;
            }

            ~Iterator<T>() {
                _reader = nullptr;
            }

            static Iterator<T> &end_iterator() {
                return iterator_empty;
            }

        private:
            bool has_init = false;
            static Iterator<T> iterator_empty;
        };

        template <typename T> Iterator<T> Iterator<T>::iterator_empty;
    }
}

#endif //RAPIDCSV_ITERATOR_HPP
