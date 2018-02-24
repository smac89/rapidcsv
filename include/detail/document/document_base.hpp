#ifndef RAPIDCSV_DOCUMENT_BASE_HPP
#define RAPIDCSV_DOCUMENT_BASE_HPP

#include <unordered_map>
#include <exception>
#include <string>
#include <iterator>
#include <vector>
#include <utility>
#include <cstddef>
#include <functional>
#include <memory>
#include "detail/csv_iterator.hpp"
#include "detail/document/properties.hpp"
#include "detail/reader/readable_wrapper.hpp"

namespace rapidcsv {
    namespace doc {
        class DocumentBase {
            using std::size_t;

            using RowType = std::unordered_map<size_t, std::string>;
            using Entry = RowType::value_type;

            friend decltype(std::begin(documentBase.documentMesh))
                std::begin(const rapidcsv::doc::DocumentBase& documentBase);

            friend auto std::end(const rapidcsv::doc::DocumentBase& documentBase)
                -> decltype(std::begin(documentBase.documentMesh));

            static const std::string EMPTY = "";

        protected:
            inline size_t get_column_index(const size_t& columnIndex) const {
                auto normalizedColumn = columnIndex + (documentProperties.hasRowLabel() ? 1 : 0);
                if (columnIndex >= column_count(0) || normalizedColumn >= column_count(0)) {
                    throw std::out_of_range(std::string("column out of range : ") + columnIndex);
                }
                return normalizedColumn;
            }

            inline size_t get_column_index(const std::string &columnName) const {
                auto columnIter = columnNames.find(columnName);
                if (columnIter == columnNames.end()) {
                    throw std::out_of_range("column not found: " + columnName);
                }
                return columnIter->second;
            }

            inline size_t get_row_index(const std::string &rowName) const {
                auto rowIter = rowNames.find(rowName);
                if (rowIter == std::end(rowNames)) {
                    throw std::out_of_range("Row label not found");
                }
                return rowIter->second;
            }

            inline size_t get_row_index(const size_t& rowIndex) const {
                auto normalizedRow = rowIndex + (documentProperties.hasHeader() ? 1 : 0);
                if (rowIndex >= size() || normalizedRow >= size()) {
                    throw std::out_of_range(std::string("Row index out of range ") + rowIndex);
                }
                return normalizedRow;
            }

        public:
            virtual ~DocumentBase() {}

            //////////////////////////////////////////////////////////
            //////////////////////// LABELS //////////////////////////
            //////////////////////////////////////////////////////////

            // SET
            void set_column_label(size_t columnIndex, const std::string &columnLabel) {
                set_column_label(get_column_label(columnIndex), columnLabel);
            }

            void set_column_label(const std::string &columnLabel, const std::string &newColumnLabel) {
                // TODO
            }

            // GET
            std::string get_column_label(size_t columnIndex) {
                auto normalizedColumnIndex = get_column_index(columnIndex);
                return documentMesh[0][normalizedColumnIndex];
            }

            std::string get_row_label(size_t rowIndex) const {
                // TODO
                return "";
            }

            //////////////////////////////////////////////////////////
            //////////////////////// SIZING //////////////////////////
            //////////////////////////////////////////////////////////

            inline size_t size() const {
                return documentMesh.size();
            }

            inline size_t column_count(const size_t& row_index) const {
                return column_count(get_row_label(row_index));
            }

            inline size_t column_count(const std::string &row_name) const {
                // TODO
                return 0;
            }

            inline size_t max_column_count() const {
                // TODO
                return 0;
            }

            //////////////////////////////////////////////////////////
            //////////////////////// CELLS ///////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            std::string& get_cell(const std::size_t rowIndex, const std::size_t columnIndex) const {
                auto normalizedRowIndex = get_row_index(rowIndex);
                auto normalizedColumnIndex = get_column_index(columnIndex);
                return documentMesh[normalizedRowIndex][normalizedColumnIndex];
            }

            std::string& get_cell(const std::string &rowName, const std::string &columnName) const {
                return get_cell(get_row_index(rowName), get_column_index(columnName));
            }

            // SET
            void set_cell(const std::size_t rowIndex, const std::size_t columnIndex, const std::string& value) {
                auto normalizedRowIndex = get_row_index(rowIndex);
                auto normalizedColumnIndex = get_column_index(columnIndex);
                documentMesh[normalizedRowIndex][normalizedColumnIndex] = value;
            }

            void set_cell(const std::string &rowName, const std::string &columnName, const std::string& value) {
                set_cell(get_row_index(rowName), get_column_index(columnName), value);
            }

            // REMOVE
            std::string remove_cell(const std::size_t rowIndex, const std::size_t columnIndex) {
                auto normalizedRowIndex = get_row_index(rowIndex);
                auto normalizedColumnIndex = get_column_index(columnIndex);

                RowType& row = documentMesh[normalizedRowIndex];
                auto&& cellValue = std::move(row[normalizedColumnIndex]);
                row.erase(normalizedColumnIndex);

                return cellValue;
            }

            std::string remove_cell(const std::string &rowName, const std::string &columnName) {
                return remove_cell(get_row_index(rowName), get_column_index(columnName));
            }

            //////////////////////////////////////////////////////////
            /////////////////////// COLUMNS //////////////////////////
            //////////////////////////////////////////////////////////

            //GET
            template<typename T>
            std::vector<T> get_column(const size_t &columnIndex, const T& fillValue) const {
                auto str_column = _get_column(get_column_index(columnIndex), rapidcsv::convert::convert_to_string(fillValue));
                return reader_to_vector(std::move(str_column), &rapidcsv::convert::convert_to_val<T>);
            }

            template<typename T>
            std::vector<T> get_column(const size_t& columnIndex) const {
                auto str_column = _get_column(get_column_index(columnIndex));
                return reader_to_vector(std::move(str_column), &rapidcsv::convert::convert_to_val<T>);
            }

            template<>
            std::vector<std::string> get_column(const std::string& columnName, const std::string& fillValue) const {
                auto str_column = _get_column(get_column_index(columnName), rapidcsv::convert::convert_to_string(fillValue));
                std::vector<std::string> column;
                std::copy(std::make_move_iterator(std::begin(str_column)),
                          std::make_move_iterator(std::end(str_column)),
                          std::back_inserter(column));
                return column;
            }

            template<>
            std::vector<std::string> get_column(const std::string& columnName) const {
                auto str_column = _get_column(get_column_index(columnName));
                std::vector<std::string> column;
                std::copy(std::make_move_iterator(std::begin(str_column)),
                          std::make_move_iterator(std::end(str_column)),
                          std::back_inserter(column));
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

                for (const auto& e_row : enumerate(wrapped(documentMesh))) {
                    RowType& row = std::get<1>(e_row);
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
                    RowType& row = std::get<1>(e_row);
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
                auto str_row = _get_row(get_row_index(rowIndex));
                std::vector<std::string> row;

                std::copy(std::make_move_iterator(std::begin(str_row)),
                          std::make_move_iterator(std::end(str_row)),
                          std::back_inserter(row));

                return row;
            }

            std::vector<std::string> get_row(const std::string &rowName) const {
                return get_row(get_row_index(rowName));
            }

            // SET
            void set_row(const size_t rowIndex, const std::vector<std::string> &row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                RowType meshRow(row.size());
                auto seq = enumerate(wrapped(row));
                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)),
                          std::begin(meshRow));

                documentMesh[get_row_index(rowIndex)] = std::move(meshRow);
            }

            void set_row(const size_t rowIndex, std::vector<std::string> &&row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                RowType meshRow(row.size());
                auto&& seq = enumerate(wrapped(std::forward<std::vector<std::string>>(std::move(row))));

                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)),
                          std::begin(meshRow));

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
                RowType& meshRow = documentMesh[normalizedIndex];

                std::vector<std::string> rowData(meshRow.size());
                std::transform(std::make_move_iterator(std::begin(meshRow)),
                               std::make_move_iterator(std::end(meshRow)),
                               std::begin(rowData), std::get<1>);

                return rowData;
            }

            std::vector<std::string> remove_row(const std::string &rowName) {
                return remove_row(get_row_index(rowName));
            }

        private:
            rapidcsv::read::Reader<std::string> _get_column(const size_t& columnIndex) const {
                using rapidcsv::read::simpleReader;
                using rapidcsv::read::transform_if;
                using rapidcsv::read::r_transform;

                auto& doc = documentMesh;

                auto begin = (documentProperties.hasHeader() ? std::next(std::begin(doc))
                                                       : std::begin(doc));
                auto end = std::end(doc);

                auto reader = transform_if(simpleReader(begin, end), [&columnIndex](const RowType &row) {
                    auto finder = row.find(columnIndex);
                    return finder != std::end(row) ? std::make_pair(true, std::cref(finder->second))
                                                   : std::make_pair(false, std::cref(EMPTY));
                }, &std::get<0>);

                return r_transform(std::move(reader), [](std::pair<bool, std::string>& data) {
                    return std::get<1>(data);
                });
            }

            rapidcsv::read::Reader<std::string> _get_column(const size_t &columnIndex, const std::string& fillValue) const {
                using Interim = std::pair<bool, std::string>;
                using rapidcsv::read::r_transform;
                using rapidcsv::read::simpleReader;

                auto &doc = documentMesh;

                auto begin = (documentProperties.hasHeader() ? std::next(std::begin(doc))
                                                       : std::begin(doc));
                auto end = std::end(doc);

                return r_transform(simpleReader(begin, end), [&columnIndex, &fillValue](const RowType &row) {
                    auto finder = row.find(columnIndex);
                    return finder != std::end(row) ? finder->second: fillValue;
                });
            }

            rapidcsv::read::Reader<std::string> _get_row (const size_t& rowIndex) const {
                using rapidcsv::read::r_transform;
                using rapidcsv::read::sequence;

                RowType &row = documentMesh[rowIndex];
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
                std::transform(std::begin(std::forward<rapidcsv::read::Reader<F>>(reader)),
                               std::end(std::forward<rapidcsv::read::Reader<F>>(reader)),
                               std::back_inserter(vec), cvt);
                return vec;
            }

        private:
            Properties documentProperties;
            std::vector <RowType> documentMesh;
            std::unordered_map <std::string, size_t> columnNames;
            std::unordered_map <std::string, size_t> rowNames;
            size_t _rowCount = 0;
            size_t _columnCount = 0;
        };
    }
}

namespace std {
    auto begin(const rapidcsv::doc::DocumentBase& documentBase)
        -> decltype(std::begin(documentBase.documentMesh)) {
        return std::begin(documentBase.documentMesh);
    }

    auto std::end(const rapidcsv::doc::DocumentBase& documentBase)
        -> decltype(std::end(documentBase.documentMesh)) {
        return std::end(documentBase.documentMesh);
    }
}

#endif //RAPIDCSV_DOCUMENT_BASE_HPP
