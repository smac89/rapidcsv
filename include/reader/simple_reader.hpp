#ifndef RAPIDCSV_SIMPLE_READER_HPP
#define RAPIDCSV_SIMPLE_READER_HPP

#include "reader.hpp"

namespace rapidcsv {
    namespace read {

        template <typename T, typename InputIt>
        class SimpleReader: public Reader<T> {
        protected:
            InputIt _begin, _end;
        public:
            explicit SimpleReader(InputIt begin, InputIt end):
                    _begin(std::move(begin)), _end(std::move(end)) { }

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
