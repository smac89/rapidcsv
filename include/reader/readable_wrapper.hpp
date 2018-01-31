#ifndef RAPIDCSV_READABLE_WRAPPER_HPP
#define RAPIDCSV_READABLE_WRAPPER_HPP

#include <iterator>
#include <type_traits>
#include <utility>
#include "reader.hpp"

namespace rapidcsv {
    namespace read {

        template <typename T, typename Readable, typename ReadableIterator>
        class ReadableWarpper: public Reader<T> {
        protected:
            Readable<T> &container;
            ReadableIterator _begin, _end;
        public:
            explicit ReadableWarpper(Readable<T>& container):
                    container(container), _begin(std::begin(container)), _end(std::end(container)) { }

            explicit ReadableWarpper(Readable<T>&& container):
                    container(std::move(container)), _begin(std::begin(container)), _end(std::end(container)) { }

            virtual bool has_next() const {
                return _begin != _end;
            }

            virtual T next() {
                return *_begin++;
            }
        };

        template <typename T, typename Readable,
                typename ReadableIterator = Readable::iterator
        >
        auto constexpr wrapReadable(Readable& container) -> ReadableWarpper<T, Readable, ReadableIterator> {
            return ReadableWarpper<T, Readable, ReadableIterator>(std::forward<Readable>(container));
        }
    }
}

#endif //RAPIDCSV_READABLE_WRAPPER_HPP
