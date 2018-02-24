#ifndef RAPIDCSV_DOCUMENT_OPERATOR_HPP
#define RAPIDCSV_DOCUMENT_OPERATOR_HPP

#include <vector>
#include <algorithm>
#include <iterator>
#include <string>
#include <cstddef>
#include "detail/util/fp.hpp"
#include "detail/reader/readable_wrapper.hpp"

namespace rapidcsv {
    namespace doc {
        template <typename Base>
        class DocumentOperator: protected Base {
            using std::size_t;

        public:
            //////////////////////////////////////////////////////////
            /////////////////////// COLUMNS //////////////////////////
            //////////////////////////////////////////////////////////

            //GET
            template<typename T>
            std::vector<T> get_column(const size_t &columnIndex, const T& fillValue) const {
                auto str_column = _get_column(get_column_index(columnName), rapidcsv::convert::convert_to_string(fillValue));
                std::vector<T> column{str_column.size()};
                std::transform(std::make_move_iterator(std::begin(str_column)),
                               std::make_move_iterator(std::end(str_column)),
                               std::begin(column),
                               &rapidcsv::convert::convert_to_val<T>);

                return column;
            }

            template<typename T>
            std::vector<T> get_column(const size_t columnIndex) const {
                auto str_column = _get_column(get_column_index(columnName));
                std::vector<T> column{str_column.size()};
                std::transform(std::make_move_iterator(std::begin(str_column)),
                               std::make_move_iterator(std::end(str_column)),
                               std::begin(column),
                               &rapidcsv::convert::convert_to_val<T>);

                return column;
            }

            template<typename T>
            std::vector<T> get_column(const std::string& columnName, const T& fillValue) const {
                return get_column(get_column_index(columnName), fillValue);
            }

            template<typename T>
            std::vector<T> get_column(const std::string& columnName) const {
                return get_column(get_column_index(columnName));
            }

            // SET
            template<typename T>
            size_t set_column(const size_t& columnIndex, const std::vector<T>& colData) {
                std::vector<std::string> converted(colData.size());
                std::transform(std::begin(colData), std::end(colData),
                               std::begin(converted),
                               &rapidcsv::convert::convert_to_string<T>);
                return set_column(columnIndex, std::move(converted));
            }

            template <>
            size_t set_column(const size_t &columnIndex, const std::vector<std::string>& colData) {
                using rapidcsv::read::wrapped;
                using rapidcsv::read::enumerate;

                for (const auto& e_row : enumerate(wrapped(document()))) {
                    Base::RowType& row = std::get<1>(e_row);
                    size_t index = std::get<0>(e_row);
                    row[columnIndex] = colData[index];
                }

                return size();
            }

            template<>
            size_t set_column(const size_t columnIndex, std::vector<std::string>&& colData) {
                using rapidcsv::read::wrapped;
                using rapidcsv::read::enumerate;

                for (const auto& e_row : enumerate(wrapped(documentMesh))) {
                    Base::RowType& row = std::get<1>(e_row);
                    size_t index = std::get<0>(e_row);
                    row.emplace(std::piecewise_construct,
                                std::forward_as_tuple(index),
                                std::forward_as_tuple(std::move(colData)[index]));
                }

                return size();
            }

            template<typename T>
            size_t set_column(const std::string &columnName, const std::vector<T>& colData) {
                return set_column(get_column_index(columnName), colData);
            }

            template <>
            size_t set_column(const std::string &columnName, const std::vector<std::string>& colData) {
                return set_column(get_column_index(columnName), colData);
            }

            template<>
            size_t set_column(const std::string &columnName, std::vector<std::string>&& colData) {
                return set_column(get_column_index(columnName), std::forward<std::vector<std::string>>(colData));

            }

            // REMOVE
            size_t remove_column(const size_t columnIndex) {
                size_t realColumnIndex = get_column_index(columnIndex);

                for (const auto& row : documentMesh) {
                    row.erase(realColumnIndex);
                }

                return size();
            }

            size_t remove_column(const std::string &columnName) {
                return remove_column(get_column_index(columnName));
            }

            //////////////////////////////////////////////////////////
            ///////////////////////// ROWS ///////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            std::vector<std::string> get_row(const size_t& rowIndex) const {
                auto row = _get_row(get_row_index(rowIndex));

            }

            std::vector<std::string> get_row(const std::string &rowName) const {
                return get_row(get_row_index(rowName));
            }

            // SET
            void set_row(const size_t rowIndex, const std::vector<std::string> &row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                Base::RowType meshRow(row.size());
                auto&& seq = enumerate(wrapped(row));
                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)), std::begin(meshRow));

                documentMesh[get_row_index(rowIndex)] = std::move(meshRow);
            }

            void set_row(const size_t rowIndex, std::vector<std::string> &&row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                Base::RowType meshRow(row.size());
                auto&& seq = enumerate(wrapped(std::forward<std::vector<std::string>>(std::move(row))));
                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)), std::begin(meshRow));

                documentMesh[get_row_index(rowIndex)] = std::move(meshRow);
            }

            void set_row(const std::string& rowName, const std::vector<std::string> &row) {
                set_row(get_row_index(rowName), row);
            }

            void set_row(const std::string& rowName, std::vector<std::string> &&row) {
                set_row(get_row_index(rowName), std::forward<std::vector<std::string>>(row));
            }

            // REMOVE
            std::vector<std::string> remove_row (const size_t rowIndex) {
                const normalizedIndex = get_row_index(rowIndex);
                MeshRow& meshRow = documentMesh[normalizedIndex];

                std::vector<std::string> rowData(meshRow.size());
                std::transform(std::make_move_iterator(std::begin(meshRow)),
                               std::make_move_iterator(std::end(meshRow)),
                               std::begin(rowData), std::get<1>);

                return rowData;
            }

            std::vector<std::string> remove_row(const std::string &rowName) {
                return remove_row(get_row_index(rowName));
            }

        protected:
            rapidcsv::read::Reader<std::string> _get_column(const size_t columnIndex) const {
                using rapidcsv::read::simpleReader;
                using rapidcsv::read::transform_if;
                using rapidcsv::read::r_transform;

                auto& doc = *this;

                auto begin = (properties().hasHeader() ? std::next(std::begin(doc))
                                                       : std::begin(doc));
                auto end = std::end(doc);

                auto reader = transform_if(simpleReader(begin, end), [&columnIndex](const Base::RowType &row) {
                    auto finder = row.find(columnIndex);
                    return finder != std::end(row) ? std::make_pair(true, finder->second)
                                                   : std::make_pair(false, "");
                }, &std::get<0>);

                return r_transform(std::forward<std::decay<decltype(reader)>::type>(reader), [](std::pair<bool, std::string>&& data) {
                    return std::get<1>(std::forward<std::decay<decltype(data)>::type>(data));
                });
            }

            rapidcsv::read::Reader<std::string> _get_column(const size_t &columnIndex, const std::string& fillValue) const {
                using Interim = std::pair<bool, std::string>;
                using rapidcsv::read::r_transform;
                using rapidcsv::read::simpleReader;

                auto &doc = *this;

                auto begin = (properties().hasHeader() ? std::next(std::begin(doc))
                                                       : std::begin(doc));
                auto end = std::end(doc);

                return r_transform(simpleReader(begin, end), [&columnIndex, &fillValue](const Base::RowType &row) {
                    auto finder = row.find(columnIndex);
                    return finder != std::end(row) ? finder->second: fillValue;
                });
            }

            rapidcsv::read::Reader<std::string> _get_row (const size_t& rowIndex) const {
                using rapidcsv::read::r_transform;
                using rapidcsv::read::sequence;

                RowType &row = this->operator[](rowIndex);
                return r_transform(sequence(static_cast<size_t>(0), max_column_count()), [](size_t const &index) {
                    auto found_it = row.find(index);
                    if (found_it != std::end(row)) {
                        return found_it->second;
                    }
                    return "";
                });
            }

            template<typename F, typename T, typename Cvt>
            inline static std::vector <T> reader_to_vector(rapidcsv::read::Reader<F> &&reader, Cvt cvt) {
                std::vector <T> vec;
                std::transform(std::make_move_iterator(std::begin(reader)),
                               std::make_move_iterator(std::end(reader)),
                               std::back_inserter(vec), cvt);

                return vec;
            }
        };
    }
}

#endif //RAPIDCSV_DOCUMENT_OPERATOR_HPP
