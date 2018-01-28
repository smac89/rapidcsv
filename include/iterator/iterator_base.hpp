#ifndef RAPIDCSV_ITERATOR_BASE_HPP
#define RAPIDCSV_ITERATOR_BASE_HPP

#include <iterator>

namespace rapidcsv {
    namespace iterator {
        template <typename T>
        class IteratorBase: public std::iterator<std::forward_iterator_tag, T> {
        public:
            virtual const T operator *() = 0;
            virtual IteratorBase<T>& operator++ () = 0;
            virtual bool operator != (IteratorBase<T>& other) = 0;
            virtual ~IteratorBase() {}
        };
    }
}

#endif //RAPIDCSV_ITERATOR_BASE_HPP
