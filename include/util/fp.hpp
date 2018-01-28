#ifndef RAPIDCSV_FP_HPP
#define RAPIDCSV_FP_HPP

#include <utility>
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

        template<class T> constexpr typename std::add_rvalue_reference<T>::type val();

        template <typename Mapper, typename Arg, typename ...Args>
        struct Map {
            using type = decltype(std::tuple_cat(
                    std::make_tuple(val<const Mapper&>()(val<const Arg&>())),
                    val<typename Map<Mapper, Args...>::type>()
            ));

            static constexpr auto map(const Mapper& mapper, const Arg& arg, const Args&...args)
            -> typename Map<Mapper, Arg, Args...>::type {
                return std::tuple_cat(std::make_tuple(mapper(arg)),
                                      Map<Mapper, Args...>::map(mapper, args...));
            }
        };

        template <typename Mapper, typename Arg>
        struct Map<Mapper, Arg> {
            using type = decltype(std::make_tuple(
                    val<const Mapper&>()(val<const Arg&>())
            ));

            static constexpr auto map(const Mapper& mapper, const Arg& arg)
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

    template <typename T, typename R, typename Trans>
    rapidcsv::read::TransformReader<T, R, Trans> r_transform(rapidcsv::read::Reader<T>&& reader, Trans transformer) {
        return rapidcsv::read::TransformReader<T, R, Trans>(std::move(reader), transformer);
    }

    template <typename T, typename Pred>
    rapidcsv::read::CopyIfReader<T, Pred> r_copy_if(rapidcsv::read::Reader<T>&& reader, Pred predicate) {
        return rapidcsv::read::CopyIfReader<T, Pred>(std::move(reader), predicate);
    }

    template<typename T, typename R, typename InputIt, typename Pred, typename Trans>
    rapidcsv::read::Reader<R> transform_if(rapidcsv::read::Reader<T>&& reader, Trans trans, Pred pred) {
        using rapidcsv::read::CopyIfReader;
        using rapidcsv::read::SimpleReader;
        using rapidcsv::read::TransformReader;
        return CopyIfReader<R, Pred>(TransformReader<T, R, Trans>(std::move(reader), trans), pred);
    }
}

#endif //RAPIDCSV_FP_HPP
