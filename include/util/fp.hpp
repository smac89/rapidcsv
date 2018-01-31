#ifndef RAPIDCSV_FP_HPP
#define RAPIDCSV_FP_HPP

#include <utility>
#include <limits>
#include <functional>
#include "iterator/iterator.hpp"
#include "reader/simple_reader.hpp"

namespace rapidcsv {

    namespace read {
        template <typename T, typename R, typename FuncRT>
        class TransformReader: public Reader<R> {

            Reader<T> _source;
            FuncRT translator;
            R value;
        public:
            TransformReader(Reader<T>&& source,
                            FuncRT &&func = TransformReader<T, R, FuncRT>::ident):
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

        private:
            auto ident = [](Reader<T>& reader) -> T {
                return reader.next();
            };
        };
    }

    namespace read {
        template <class T, class Pred>
        class CopyIfReader: public Reader<T> {
            Pred _predicate;
            Reader<T> _reader;
       public:
            explicit CopyIfReader(Reader<T>&& reader, Pred predicate):
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

    namespace detail {
        template<std::size_t I>
        struct Apply {
            template<typename Func, typename T, typename ...Args>
            static constexpr auto apply(const Func &func, const T &tup, const Args &... args)
            -> decltype(Apply<I - 1>::apply(
                    func, tup, std::get<I - 1>(tup), args...)) {
                return Apply<I - 1>::apply(func, tup, std::get<I - 1>(tup), args...);
            }
        };

        template<>
        struct Apply<0> {
            template<typename Func, typename T, typename ...Args>
            static constexpr auto apply(const Func &func, const T &tup, const Args &... args)
            -> decltype(func(args...)) {
                return func(args...);
            }
        };

        template <typename Mapper, typename Arg, typename ...Args>
        struct Map {
            using type = decltype(std::tuple_cat(
                    std::make_tuple(std::declval<const Mapper&>()(std::declval<const Arg&>())),
                    std::declval<typename Map<Mapper, Args...>::type>()
            ));

            auto static constexpr map(const Mapper& mapper, const Arg& arg, const Args&...args)
            -> typename Map<Mapper, Arg, Args...>::type {
                return std::tuple_cat(std::make_tuple(mapper(arg)),
                                      Map<Mapper, Args...>::map(mapper, args...));
            }
        };

        template <typename Mapper, typename Arg>
        struct Map<Mapper, Arg> {
            using type = decltype(std::make_tuple(
                    std::declval<const Mapper&>()(std::declval<const Arg&>())
            ));

            auto static constexpr map(const Mapper& mapper, const Arg& arg)
            -> typename Map<Mapper, Arg>::type {
                return std::make_tuple(mapper(arg));
            }
        };

        template<typename Func, typename ...Ts>
        constexpr auto apply(const Func &func, const std::tuple<Ts...> &mtup)
        -> decltype(Apply<std::tuple_size<std::tuple<Ts...>>::value>::apply(func, mtup)) {
            return Apply<std::tuple_size<std::tuple<Ts...>>::value>::apply(func, mtup);
        }

        template<typename Fold, typename T>
        constexpr auto fold(const Fold &func, const T &arg)-> T {
            return arg;
        }

        template<typename Fold, typename T1, typename T2, typename ...Args>
        constexpr auto fold(const Fold &func, const T1 &arg1, const T2 &arg2, const Args &...rest)-> T1 {
            return fold(func, func(arg1, arg2), rest...);
        }

        template<typename Fold, typename R, typename ...Args>
        constexpr auto fold(const Fold &func, const R &initial, const std::tuple<Args...> &mtup)-> R {
            return apply([&func](const Args&... args) {
                             return fold(func, args...);
                         },
                         std::tuple_cat(std::make_tuple(initial), mtup));
        }

        template<typename Mapper, typename ...Args>
        constexpr auto map(const Mapper &func, const std::tuple<Args...> &mtup)
        -> typename Map<Mapper, Args...>::type {
            return apply([&func](const Args&... args) {
                return Map<Mapper, Args...>::map(func, args...);
            }, mtup);
        }
    }

    namespace read {
        template <typename ...Ts>
        class ZipReader: public Reader<std::tuple<Ts...>> {

            std::tuple<Reader<Ts>...> readers;
        public:
            template <typename T>
            ZipReader(Reader<T>&& reader): ZipReader(std::piecewise_construct, std::forward(reader)) { }
            explicit ZipReader(Reader<Ts>&&... readers): ZipReader(std::forward_as_tuple(readers...)) { }
            explicit ZipReader(std::tuple<Reader<Ts>...>&& rest): readers(std::move(rest)) { }

            bool has_next() const {
                return detail::fold(&ZipReader<std::tuple<Ts...>>::HaveNextFunc, true, std::forward(readers));
            }

            std::tuple<Ts...> next() {
                return detail::map(&ZipReader<std::tuple<Ts...>>::GetNextFunc, std::forward(readers));
            }

        private:
            constexpr static struct {
                template <typename T>
                auto operator()(bool currentResult, const Reader<T>& reader) const -> bool {
                    return currentResult && reader.has_next();
                }
            } HaveNextFunc;

            constexpr static struct {
                template <typename T>
                auto operator()(Reader<T>& reader) const -> T {
                    return std::move(reader.next());
                }
            } GetNextFunc;
        };
    }

    namespace read {
        template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
        class NumberSequenceReader: public Reader<T> {
            T current;
        public:
            explicit NumberSequenceReader(const T& start): current(start) { }

            bool has_next() const {
                return current < std::numeric_limits<T>::max();
            }

            T next() {
                return current++;
            }
        };
    }

    namespace read {
        template <typename T, typename R, typename Trans>
        auto r_transform(Reader<T>&& reader, Trans transformer)
        -> TransformReader<T, R, Trans> {
            return TransformReader<T, R, Trans>(std::move(reader), transformer);
        }

        template <typename T, typename Pred>
        auto r_copy_if(Reader<T>&& reader, Pred predicate)
        -> CopyIfReader<T, Pred> {
            return CopyIfReader<T, Pred>(std::move(reader), predicate);
        }

        template<typename T, typename R, typename InputIt, typename Pred, typename Trans>
        Reader<R> transform_if(Reader<T>&& reader, Trans trans, Pred pred) {
            return CopyIfReader<R, Pred>(TransformReader<T, R, Trans>(std::move(reader), trans), pred);
        }

        template <typename T>
        auto sequence(const T &start) -> NumberSequenceReader<T> {
            return NumberSequenceReader<T>(start);
        }

        template <typename T, typename ...Ts>
        auto zipped(Reader<T>&& reader, Reader<Ts>&&... readers)
        -> ZipReader<T, Ts...> {
            return ZipReader<T, Ts...>(std::move(reader), std::move(readers));
        }

        template <typename T>
        auto enumerate(Reader<T>&& reader)
            -> ZipReader<std::size_t, T> {
            return zipped(
                    std::forward<Reader<std::size_t>>(sequence(static_cast<std::size_t>(0))),
                    std::move(reader));
        }
    }
}

#endif //RAPIDCSV_FP_HPP
