#ifndef RAPIDCSV_FP_HPP
#define RAPIDCSV_FP_HPP

#include <utility>
#include <functional>
#include "iterator/iterator.hpp"
#include "reader/simple_reader.hpp"

namespace rapidcsv {

    namespace read {
        template <typename T, typename R, typename FuncRT>
        class TransformReader: public ReaderBase<R> {

            ReaderBase<T> _source;
            FuncRT translator;
            R value;
        public:
            TransformReader(ReaderBase<T>&& source,
                            FuncRT &&func = std::bind(&ReaderBase<T>::next, &source, std::placeholders::_1)):
                    _source(std::move(source)), translator(std::move(func)) {}

            virtual bool has_next() const {
                return _source.has_next();
            }

            virtual R next() {
                value = translator(_source);
                return value;
            };

            template <typename N, typename FuncTN>
            TransformReader<T, N, FuncTN> operator >> (FuncTN nextTranslator) {
                return TransformReader(*this, nextTranslator);
            }
        };
    }

    namespace read {
        template <class T, class Pred>
        class CopyIfReader: public ReaderBase<T> {
            Pred _predicate;
            ReaderBase<T> _reader;
       public:
            explicit CopyIfReader(ReaderBase<T>&& reader, Pred predicate):
                    _reader(std::move(reader)), _predicate(std::move(predicate)) { }

            bool has_next() const {
                while (_reader.has_next()) {
                    if (_predicate(_reader.peek())) {
                        return true;
                    }
                    _reader.begin()++;
                }
                return false;
            }

            T next() {
                return _reader.peek();
            }

        private:
            void run_to_next() {
                while (_reader.has_next()) {
                    if (_predicate(peek())) {
                        return;
                    }
                    _reader.begin()++;
                }
            }
        };
    }

    template<typename T, typename R, typename InputIt, typename Pred, typename Trans>
    rapidcsv::read::ReaderBase<R> transform_if(InputIt begin, InputIt end, Pred pred) {
        using rapidcsv::read::CopyIfReader;
        using rapidcsv::read::SimpleReader;
        using rapidcsv::read::TransformReader;
        return TransformReader<T, R, Trans>(CopyIfReader<T, Pred>(SimpleReader(begin, end), pred));
    }
}

#endif //RAPIDCSV_FP_HPP
