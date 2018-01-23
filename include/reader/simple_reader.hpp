#ifndef RAPIDCSV_SIMPLE_READER_HPP
#define RAPIDCSV_SIMPLE_READER_HPP

#include "reader_base.hpp"

namespace rapidcsv {
    namespace read {

        template <typename T, typename InputIt>
        class SimpleReader: public ReaderBase<T> {
        protected:
            InputIt _begin, _end;
        public:
            explicit SimpleReader(InputIt begin, InputIt end): _begin(begin), _end(end) { }

            virtual bool has_next() const {
                return _begin != _end;
            }

            virtual T next() {
                return *_begin++;
            }
        };
    }
}

#endif //RAPIDCSV_SIMPLE_READER_HPP
