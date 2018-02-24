#ifndef RAPIDCSV_READABLE_WRAPPER_HPP
#define RAPIDCSV_READABLE_WRAPPER_HPP

#include <iterator>
#include <type_traits>
#include <utility>
#include <functional>
#include "reader.hpp"

namespace rapidcsv {
    namespace read {

        template <typename T, typename Readable, typename ReadableIterator>
        class ReadableWarpper: public Reader<T> {
        protected:
            const Readable &container;
            ReadableIterator _begin, _end;
        public:
            explicit ReadableWarpper(const Readable& container):
                    container(container), _begin(std::begin(container)), _end(std::end(container)) { }

            virtual bool has_next() const {
                return _begin != _end;
            }

            virtual T& next() {
                return *_begin++;
            }
        };

//        template <typename T, typename Readable>
//        auto constexpr wrapped(Readable &&container) -> Reader<T> {
//            return ReadableWarpper<T, Readable, Readable::iterator>(std::forward<Readable>(container));
//        }

        template <typename T, typename Readable>
        auto constexpr wrapped(const Readable &container) -> Reader<T> {
            return ReadableWarpper<T, Readable, Readable::iterator>(container);
        }
    }
}

#endif //RAPIDCSV_READABLE_WRAPPER_HPP
