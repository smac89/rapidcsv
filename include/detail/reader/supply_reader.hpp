#ifndef RAPIDCSV_SUPPLY_READER_HPP
#define RAPIDCSV_SUPPLY_READER_HPP

#include <utility>
#include "reader.hpp"

namespace rapidcsv {
    namespace read {

        template <typename T, typename Supp>
        class SupplyReader: public Reader<T> {
        protected:
            Supp& supplier;
        public:
            explicit SupplyReader(Supp&& supplier): supplier(std::move(supplier)) { }
            explicit SupplyReader(const Supp& supplier): supplier(supplier) {}

            bool has_next() const {
                return true;
            }

            T next() {
                return supplier();
            }
        };

        template <typename T, typename Supply>
        auto constexpr supplyReader(Supply supply) -> SupplyReader<T, Supply> {
            return SupplyReader<T, Supply>(std::forward<Supply>(supply));
        }
    }
}

#endif //RAPIDCSV_SUPPLY_READER_HPP
