#ifndef RAPIDCSV_DOCUMENT_HPP
#define RAPIDCSV_DOCUMENT_HPP

#include <cstddef> // std::size_t
#include <vector>
#include <string>
#include <memory>
#include <exception>
#include <iterator>
#include <unordered_map>
#include "properties.hpp"
#include "detail/csv_convert.hpp"
#include "detail/util/fp.hpp"

namespace rapidcsv {
    namespace doc {
        class Document {
            using MeshRow = std::unordered_map<std::size_t, std::string>;
            using Entry = MeshRow::value_type;

        private:
            Properties documentProperties;
            Document(Properties properties): documentProperties(std::move(properties)) {}

        public:
            Document(Document&&) = default;
            Document(const Document&) = default;

//            Document&operator = (Document&&) = default;
//            Document&operator = (const Document&) = default;

            //////////////////////////////////////////////////////////
            /////////////////////// COLUMNS //////////////////////////
            //////////////////////////////////////////////////////////

            //GET
            template<typename T>
            std::vector<T> GetColumn(const std::size_t &columnIndex, const T& fillValue) const {
                auto str_column = GetColumn(columnIndex, rapidcsv::convert::convert_to_string(fillValue));
                std::vector<T> column{str_column.size()};
                std::transform(std::make_move_iterator(std::begin(str_column)),
                               std::make_move_iterator(std::end(str_column)),
                               std::begin(column),
                               &rapidcsv::convert::convert_to_val<T>);

                return column;
            }

            template<typename T>
            std::vector<T> GetColumn(const std::string &columnName, const T& fillValue) const {
                auto str_column = GetColumn(columnName, rapidcsv::convert::convert_to_string<T>(fillValue));
                std::vector<T> column{str_column.size()};
                std::transform(std::make_move_iterator(std::begin(str_column)),
                               std::make_move_iterator(std::end(str_column)),
                               std::begin(column),
                               &rapidcsv::convert::convert_to_val<T>);

                return column;
            }

            template<typename T>
            std::vector<T> GetColumn(const size_t columnIndex) const {
                auto str_column = GetColumn(columnIndex);
                std::vector<T> column{str_column.size()};
                std::transform(std::make_move_iterator(std::begin(str_column)),
                               std::make_move_iterator(std::end(str_column)),
                               std::begin(column),
                               &rapidcsv::convert::convert_to_val<T>);

                return column;
            }

            template<typename T>
            std::vector<T> GetColumn(const std::string columnName) const {
                auto str_column = GetColumn(columnName);
                std::vector<T> column{str_column.size()};
                std::transform(std::make_move_iterator(std::begin(str_column)),
                               std::make_move_iterator(std::end(str_column)),
                               std::begin(column),
                               &rapidcsv::convert::convert_to_val<T>);

                return column;
            }

            virtual std::vector<std::string> GetColumn(const std::string &columnName, const std::string& fillValue) const = 0;

            virtual std::vector<std::string> GetColumn(const std::size_t &columnIndex, const std::string& fillValue) const = 0;

            virtual std::vector<std::string> GetColumn(const std::string &columnName) const = 0;

            virtual std::vector<std::string> GetColumn(const std::size_t &columnIndex) const = 0;

            // SET
            template<typename T>
            std::size_t SetColumn(const size_t columnIndex, const std::vector<T>& colData) {
                std::vector<std::string> converted(colData.size());
                std::transform(std::begin(colData), std::end(colData),
                               std::make_move_iterator(std::begin(converted)),
                               &rapidcsv::convert::convert_to_string<T>);
                return SetColumn(columnIndex, std::move(converted));
            }

            template<typename T>
            std::size_t SetColumn(const std::string &columnName, const std::vector<T>& colData) {
                std::vector<std::string> converted(colData.size());
                std::transform(std::begin(colData), std::end(colData),
                               std::make_move_iterator(std::begin(converted)),
                               &rapidcsv::convert::convert_to_string<T>);
                return SetColumn(columnName, std::move(converted));
            }

            virtual std::size_t SetColumn(const size_t columnIndex, const std::vector<std::string>& colData) = 0;

            virtual std::size_t SetColumn(const std::string &columnName, const std::vector<std::string>& colData) = 0;

            virtual std::size_t SetColumn(const size_t columnIndex, std::vector<std::string>&& colData) = 0;

            virtual std::size_t SetColumn(const std::string &columnName, std::vector<std::string>&& colData) = 0;

            // REMOVE
            virtual std::size_t RemoveColumn(const size_t columnIndex) = 0;
            virtual std::size_t RemoveColumn(const std::string &columnName) = 0;

            //////////////////////////////////////////////////////////
            ///////////////////////// ROWS ///////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            virtual std::vector<std::string> GetRow(const size_t) const = 0;

            // Throw if user did not define which column is the row label
            virtual std::vector<std::string> GetRow(const std::string &) const = 0;

            // SET
            virtual void SetRow(const size_t, const std::vector<std::string> &) = 0;
            virtual void SetRow(const size_t, std::vector<std::string> &&) = 0;

            virtual void SetRow(const std::string&, const std::vector<std::string> &) = 0;
            virtual void SetRow(const std::string&, std::vector<std::string> &&) = 0;

            // REMOVE return values removed
            virtual std::vector<std::string> RemoveRow (const size_t rowIndex) = 0;
            virtual std::vector<std::string> RemoveRow(const std::string &rowName) = 0;

            //////////////////////////////////////////////////////////
            //////////////////////// CELLS ///////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            template<typename T>
            T GetCell(const size_t rowIndex, const size_t columnIndex) const {
                return rapidcsv::convert::convert_to_val<T>(GetCell(rowIndex, columnIndex));
            }

            template<typename T>
            T GetCell(const std::string &rowName, const std::string &columnName) const {
                return rapidcsv::convert::convert_to_val<T>(GetCell(rowName, columnName));
            }

            virtual std::string GetCell(const std::size_t &rowIndex, const std::size_t &columnIndex) const = 0;

            virtual std::string GetCell(const std::string &rowName, const std::string &columnName) const = 0;

            // SET
            virtual void SetCell(const std::size_t rowIndex, const std::size_t columnIndex, const std::string&) = 0;

            virtual void SetCell(const std::string &rowName, const std::string &columnName, const std::string&) = 0;

            template<typename T>
            void SetCell(const std::size_t rowIndex, const std::size_t columnIndex, const T& tVal) {
                SetCell(columnIndex, rowIndex, rapidcsv::convert::convert_to_string(tVal));
            }

            template<typename T>
            void SetCell(const std::string &rowName, const std::string &columnName, const T& tVal) {
                SetCell(columnName, rowName, rapidcsv::convert::convert_to_string(tVal));
            }

            // REMOVE
            virtual std::string RemoveCell(const std::size_t rowIndex, const std::size_t columnIndex) = 0;
            virtual std::string RemoveCell(const std::string &rowName, const std::string &columnName) = 0;

            //////////////////////////////////////////////////////////
            //////////////////////// LABELS //////////////////////////
            //////////////////////////////////////////////////////////

            // SET
            virtual void SetColumnLabel(std::size_t columnIndex, const std::string &columnLabel) {
                SetColumnLabel(GetColumnLabel(columnIndex), columnLabel);
            }

            virtual void SetColumnLabel(const std::string &columnLabel, const std::string &newColumnLabel) = 0;

            // GET
            virtual std::string GetColumnLabel(std::size_t columnIndex) const = 0;
            virtual std::string GetRowLabel(std::size_t rowIndex) const = 0;

            //////////////////////////////////////////////////////////
            //////////////////////// SIZING //////////////////////////
            //////////////////////////////////////////////////////////
            virtual std::size_t size() const = 0;

            virtual std::size_t max_size() const = 0;

            virtual std::size_t column_count(const std::size_t row_index) const {
                return column_count(GetRowLabel(row_index));
            }

            virtual std::size_t column_count(const std::string& row_name) const = 0;

            virtual ~Document() {}

        protected:
            //////////////////////////////////////////////////////////
            ///////////////////////// INDEX //////////////////////////
            //////////////////////////////////////////////////////////

//            virtual std::size_t getColumnIndex(const std::string &columnName) const = 0;
//            virtual std::size_t getColumnIndex(const std::size_t columnIndex) const = 0;
//            virtual std::size_t getRowIndex(const std::string &rowName) const = 0;
//            virtual std::size_t getRowIndex(const std::size_t rowIndex) const = 0;

        private:
            rapidcsv::read::Reader<std::string> _GetColumn(const size_t columnIndex) const {
                using rapidcsv::read::simpleReader;
                using rapidcsv::read::transform_if;
                using rapidcsv::read::r_transform;

                auto begin = (documentProperties.hasHeader() ? std::next(std::begin(documentMesh))
                                                             : std::begin(documentMesh));
                auto end = std::end(documentMesh);

                auto reader = std::move(transform_if(simpleReader(begin, end), [&columnIndex](const MeshRow &row) {
                    auto finder = row.find(columnIndex);
                    return finder != std::end(row) ? std::make_pair(true, finder->second)
                                                   : std::make_pair(false, "");
                }, &std::get<0>));

                return r_transform(std::forward<std::decay<decltype(reader)>::type>(reader), [](std::pair<bool, std::string>&& data) {
                    return std::get<1>(std::forward<std::decay<decltype(data)>::type>(data));
                });
            }

            std::vector<std::string> _GetRow(const std::size_t rowIndex) const {
                using rapidcsv::read::sequence;
                using rapidcsv::read::r_copy_if;
                using Tup = std::tuple<std::size_t, std::size_t>;

                MeshRow& row = documentMesh[rowIndex];
                std::vector<std::string> data(row.size());

                for (auto&& seq : sequence(r_copy_if(sequence(), [&row](const std::size_t index) {
                    return row.count(index) > 0;
                }))) {
                    data[std::get<0>(std::forward<Tup>(seq))] = row[std::get<1>(std::forward<Tup>(seq))];
                }

                return data;
            }

            template <typename F, typename T, typename Cvt>
            inline static std::vector<T> reader_to_vector(rapidcsv::read::Reader<F>&& reader, Cvt cvt) {
                std::vector<T> vec;
                std::transform(std::make_move_iterator(std::begin(reader)),
                               std::make_move_iterator(std::end(reader)),
                               std::back_inserter(vec), cvt);

                return vec;
            }

            inline std::size_t getColumnIndex(const std::size_t columnIndex) const {
                auto normalizedColumn = columnIndex + (documentProperties.hasRowLabel() ? 1 : 0);
                if (columnIndex >= this->columnCount() || normalizedColumn >= columnCount()) {
                    throw std::out_of_range(std::string("column out of range : ") + columnIndex);
                }
                return normalizedColumn;
            }

            inline std::size_t getRowIndex(const std::string &rowName) const {
                auto rowIter = rowNames.find(rowName);
                if (rowIter == std::end(rowNames)) {
                    throw std::out_of_range("Row label not found");
                }
                return rowIter->second;
            }

            inline std::size_t getRowIndex(const std::size_t rowIndex) const {
                auto normalizedRow = rowIndex + (documentProperties.hasHeader() ? 1 : 0);
                if (rowIndex >= maxRowCount() || normalizedRow >= maxRowCount()) {
                    throw std::out_of_range(std::string("Row index out of range ") + rowIndex);
                }
                return normalizedRow;
            }

        private:
            std::vector<MeshRow> documentMesh;
            std::unordered_map<std::string, std::size_t> columnNames;
            std::unordered_map<std::string, std::size_t> rowNames;
            std::size_t _rowCount = 0;
            std::size_t _columnCount = 0;
        };
    }
}

#endif //RAPIDCSV_DOCUMENT_HPP
