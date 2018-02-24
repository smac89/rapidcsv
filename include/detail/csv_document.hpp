#ifndef RAPIDCSV_CSV_DOCUMENT_HPP
#define RAPIDCSV_CSV_DOCUMENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <map>
#include <utility>
#include <fstream>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <numeric>

#include "detail/reader/simple_reader.hpp"
#include "detail/reader/supply_reader.hpp"
#include "detail/reader/readable_wrapper.hpp"
#include "detail/document/properties.hpp"
#include "detail/document/document.hpp"
#include "detail/csv_reader.hpp"
#include "detail/csv_convert.hpp"
#include "detail/csv_iterator.hpp"
#include "detail/util/fp.hpp"

namespace rapidcsv {
    namespace doc {
        class CSVDocument : public Document {
            friend Document rapidcsv::load(const std::string&);
            friend Document rapidcsv::load(const Properties&);
            friend void rapidcsv::save(const doc::CSVDocument&);
            friend void rapidcsv::save(const doc::CSVDocument&, const std::string&);

            using Document::Document;
            using rapidcsv::read::SimpleReader;

            using MeshRow = std::unordered_map<std::size_t, std::string>;
            using Entry = MeshRow::value_type;

            CSVDocument(CSVDocument&&) = default;
            CSVDocument(const CSVDocument&) = default;

            explicit CSVDocument(std::vector<MeshRow>&& data, Properties properties)
                    :Document(std::move(properties)), documentMesh(std::move(data)) {}

        public:

            //////////////////////////////////////////////////////////
            /////////////////////// COLUMNS //////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            template<typename T>
            std::vector<T> GetColumn(const size_t columnIndex, const T& fillValue) const {
                return _GetColumn(getColumnIndex(columnIndex), fillValue);
            }

            template<typename T>
            std::vector<T> GetColumn(const std::string &columnName, const T& fillValue) const {
                return _GetColumn(getColumnIndex(columnName), fillValue);
            }

            template<typename T>
            std::vector<T> GetColumn(const size_t columnIndex) const {
                return reader_to_vector(_GetColumn(getColumnIndex(columnIndex)),
                                        &rapidcsv::convert::convert_to_val<T>);
            }

            template<typename T>
            std::vector<T> GetColumn(const std::string &columnName) const {
                return reader_to_vector(_GetColumn(getColumnIndex(columnName)),
                                        &rapidcsv::convert::convert_to_val<T>);
            }

            std::vector<std::string> GetColumn(const std::string &columnName, const std::string& fillValue) const {

            }

            std::vector<std::string> GetColumn(const std::size_t &columnIndex, const std::string& fillValue) const {

            }

            std::vector<std::string> GetColumn(const std::string &columnName) {

            }

            std::vector<std::string> GetColumn(const std::size_t &columnIndex) {

            }

            // SET
            template<>
            std::size_t SetColumn(const size_t columnIndex, const std::vector<std::string>& colData) {
                using rapidcsv::read::wrapped;
                using rapidcsv::read::enumerate;

                for (const auto& e_row : enumerate(wrapped(documentMesh))) {
                    MeshRow& row = std::get<1>(e_row);
                    std::size_t index = std::get<0>(e_row);
                    row[columnIndex] = colData[index];
                }

                return documentMesh.size();
            }

            template<>
            std::size_t SetColumn(const size_t columnIndex, std::vector<std::string>&& colData) {
                using rapidcsv::read::wrapped;
                using rapidcsv::read::enumerate;

                for (const auto& e_row : enumerate(wrapped(documentMesh))) {
                    MeshRow& row = std::get<1>(e_row);
                    std::size_t index = std::get<0>(e_row);
                    row.emplace(std::piecewise_construct,
                                std::forward_as_tuple(index),
                                std::forward_as_tuple(std::move(colData)[index]));
                }

                return documentMesh.size();
            }

            template<>
            std::size_t SetColumn(const std::string &columnName, const std::vector<std::string>& colData) {
                return SetColumn(getColumnIndex(columnName), colData);
            }

            template<>
            std::size_t SetColumn(const std::string &columnName, std::vector<std::string>&& colData) {
                return SetColumn(getColumnIndex(columnName), std::forward<std::vector<std::string>>(colData));
            }

            // REMOVE
            std::size_t RemoveColumn(const size_t columnIndex) {
                std::size_t realColumnIndex = getColumnIndex(columnIndex);

                for (const auto& row : documentMesh) {
                    row.erase(realColumnIndex);
                }

                return documentMesh.size();
            }

            std::size_t RemoveColumn(const std::string &columnName) {
                return RemoveColumn(getColumnIndex(columnName));
            }

            //////////////////////////////////////////////////////////
            ///////////////////////// ROWS ///////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            std::vector<std::string> GetRow(const size_t rowIndex) const {
                return _GetRow(getRowIndex(rowIndex));
            }

            std::vector<std::string> GetRow(const std::string &rowName) const {
                return _GetRow(getRowIndex(rowName));
            }

            // SET
            void SetRow(const size_t rowIndex, const std::vector<std::string> &row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                MeshRow meshRow(row.size());
                auto&& seq = enumerate(wrapped(row));
                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)), std::begin(meshRow));

                documentMesh[getRowIndex(rowIndex)] = std::move(meshRow);
            }

            void SetRow(const size_t rowIndex, std::vector<std::string> &&row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                MeshRow meshRow(row.size());
                auto&& seq = enumerate(wrapped(std::forward<std::vector<std::string>>(std::move(row))));
                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)), std::begin(meshRow));

                documentMesh[getRowIndex(rowIndex)] = std::move(meshRow);
            }

            void SetRow(const std::string& rowName, const std::vector<std::string> &row) {
                SetRow(getRowIndex(rowName), row);
            }

            void SetRow(const std::string& rowName, std::vector<std::string> &&row) {
                SetRow(getRowIndex(rowName), std::forward<std::vector<std::string>>(row));
            }

            std::vector<std::string> RemoveRow (const size_t rowIndex) {
                const normalizedIndex = getRowIndex(rowIndex);
                MeshRow& meshRow = documentMesh[normalizedIndex];

                std::vector<std::string> rowData(meshRow.size());
                std::transform(std::make_move_iterator(std::begin(meshRow)),
                               std::make_move_iterator(std::end(meshRow)),
                               std::begin(rowData), std::get<1>);

                return rowData;
            }

            std::vector<std::string> RemoveRow(const std::string &rowName) {
                return RemoveRow(getRowIndex(rowName));
            }

            //////////////////////////////////////////////////////////
            //////////////////////// CELLS ///////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            template<>
            std::string GetCell(const std::size_t rowIndex, const std::size_t columnIndex) const {
                auto normalizedRowIndex = getRowIndex(rowIndex);
                auto normalizedColumnIndex = getColumnIndex(columnIndex);
                return documentMesh[normalizedRowIndex][normalizedColumnIndex];
            }

            template<>
            std::string GetCell(const std::string &rowName, const std::string &columnName) const {
                return GetCell(getRowIndex(rowName), getColumnIndex(columnName));
            }

            // SET
            template<>
            void SetCell(const std::size_t rowIndex, const std::size_t columnIndex, const std::string& value) {
                auto normalizedRowIndex = getRowIndex(rowIndex);
                auto normalizedColumnIndex = getColumnIndex(columnIndex);
                documentMesh[normalizedRowIndex][normalizedColumnIndex] = value;
            }

            template<>
            void SetCell(const std::string &rowName, const std::string &columnName, const std::string& value) {
                SetCell(getRowIndex(rowName), getColumnIndex(columnName), value);
            }

            // REMOVE
            std::string RemoveCell(const std::size_t rowIndex, const std::size_t columnIndex) {
                auto normalizedRowIndex = getRowIndex(rowIndex);
                auto normalizedColumnIndex = getColumnIndex(columnIndex);

                MeshRow& row = documentMesh[normalizedRowIndex];
                auto cellValue = std::move(row[normalizedColumnIndex]);
                row.erase(normalizedColumnIndex);

                return cellValue;
            }

            std::string RemoveCell(const std::string &rowName, const std::string &columnName) {
                return RemoveCell(getRowIndex(rowName), getColumnIndex(columnName));
            }

            //////////////////////////////////////////////////////////
            //////////////////////// LABELS //////////////////////////
            //////////////////////////////////////////////////////////

            // SET
            void SetColumnLabel(const std::string &columnLabel, const std::string &newColumnLabel) {
                auto normalizedColumnIndex = getColumnIndex(columnLabel);

                documentMesh[0][normalizedColumnIndex] = newColumnLabel;

                columnNames.erase(columnLabel);
                columnNames[newColumnLabel] = normalizedColumnIndex;
            }

            // GET
            std::string GetColumnLabel(std::size_t columnIndex) {
                auto normalizedColumnIndex = getColumnIndex(columnIndex);
                return documentMesh[0][normalizedColumnIndex];
            }

            //////////////////////////////////////////////////////////
            //////////////////////// SIZING //////////////////////////
            //////////////////////////////////////////////////////////
            std::size_t rowCount(const std::size_t rowIndex) const {
                auto normalizedRowIndex = getRowIndex(rowIndex);
                return documentMesh[normalizedRowIndex].size();
            }

            std::size_t rowCount(const std::string& rowName) const {
                return rowCount(getRowIndex(rowName));
            }

            std::size_t maxRowCount() const {
                return _rowCount;
            }

            std::size_t columnCount() const {
                return _columnCount;
            }

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

            template<typename T>
            std::vector<T> _GetColumn(const size_t columnIndex, const T& fillValue) const {
                using Interim = std::pair<bool, std::string>;

                std::vector<T> column;
                auto begin = (documentProperties.hasHeader() ? std::next(std::begin(documentMesh))
                                                             : std::begin(documentMesh));
                auto end = std::end(documentMesh);

                std::transform(begin, end, std::back_inserter(column), [&columnIndex, &fillValue](const MeshRow &row) {
                    auto finder = row.find(columnIndex);
                    return finder != std::end(row) ? convert::convert_to_val<T>(finder->second): fillValue;
                });

                return column;
            }

            inline std::size_t getColumnIndex(const std::string &columnName) const {
                auto columnIter = columnNames.find(columnName);
                if (columnIter == columnNames.end()) {
                    throw std::out_of_range("column not found: " + columnName);
                }
                return columnIter->second;
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

            template <typename F, typename T, typename Cvt>
            inline static std::vector<T> reader_to_vector(rapidcsv::read::Reader<F>&& reader, Cvt cvt) {
                std::vector<T> vec;
                std::transform(std::make_move_iterator(std::begin(reader)),
                               std::make_move_iterator(std::end(reader)),
                               std::back_inserter(vec), cvt);

                return vec;
            }

        private:
            std::vector<MeshRow> documentMesh;
            std::unordered_map<std::string, std::size_t> columnNames;
            std::unordered_map<std::string, std::size_t> rowNames;
            std::size_t _rowCount = 0;
            std::size_t _columnCount = 0;
        };

        class Document::Impl: public CSVDocument {
            using CSVDocument::CSVDocument;
        };
    }
}

namespace rapidcsv {
    void save(const doc::CSVDocument& document) {
        using rapidcsv::operators::to_string;
        using rapidcsv::read::wrapped;
        using rapidcsv::read::zipped;

        std::ofstream file(path, std::ios::out | std::ios::binary);
        if (documentProperties.hasHeader()) {
            std::vector<std::string> header(columnNames.size());

            std::transform(std::begin(columnNames), std::end(columnNames), std::begin(header), [](const Title& title) {
                return title.first;
            });

            if (documentProperties.hasRowLabel() && !(columnNames.empty() || documentMesh.empty())) {
                file << documentProperties.fieldSep();
            }

            file << to_csv_row(std::begin(header), std::end(header),
                               [&columnNames](const std::string& value) { return columnNames[value]; })
                 << to_string(documentProperties.rowSep());
        }

//    std::vector<std::string> rowsName(rowNames.size());
//    std::transform(std::begin(rowNames), std::end(rowNames), std::begin(rowsName), [](const Title& title) {
//        return title.first;
//    });

//    auto cmp = [&rowNames](const std::string& s1, const std::string& s2) {
//        return rowNames[s1] < rowNames[s2];
//    };
//
//    std::map<std::string, std::size_t, decltype(cmp)> rowsNames(cmp);
//    rowsNames.insert(std::begin(rowNames), std::end(rowNames));
//
//    auto


        std::transform(std::begin(documentMesh), std::end(documentMesh),
                       std::ostream_iterator<std::string>(file, to_string(documentProperties.rowSep())),
                       [](const MeshRow& row) {
                           return to_csv_row()
                       }
        std::bind(&CSVDocument::toCsvRow, this));
    }

    doc::Document load(const Properties &properties) {
        using rapidcsv::read::wrapped;
        using rapidcsv::read::enumerate;
        using ParamType = std::tuple<std::size_t, std::string>;
        std::ifstream file (properties.filePath(), std::ios::in | std::ios::binary);

        auto reader =  r_transform(row_reader(file), [](std::vector<std::string>&& row) {
            rapidcsv::doc::CSVDocument::MeshRow transformRow;

            auto zip = enumerate(wrapped(row));

            std::transform(std::make_move_iterator(std::begin(zip)),
                           std::make_move_iterator(std::end(zip)), std::inserter(transformRow,
                                                                                 std::begin(transformRow)),
                           [](ParamType&& out) {
                               return std::make_pair(
                                       std::forward<std::size_t>(std::get<0>(std::forward<ParamType>(out))),
                                       std::forward<std::string>(std::get<1>(std::forward<ParamType>(out))));
                           }
            );
            return transformRow;
        });
    }

    template <typename In, typename Pos>
    std::string to_csv_row(In begin, In end, Pos func) {
        std::ostringstream oss;
        auto columnCount = columnCount();

        for (auto i = 0; i < columnCount; i++) {
            if (begin != end) {
                if (func(*begin) == i) {
                    oss << *begin++;
                }
            }

            if (i < columnCount - 1) {
                oss << documentProperties.fieldSep();
            }
        }

        return oss.str();
    }

    doc::Document load(const std::string& path) {
        return load(PropertiesBuilder().filePath(path));
    }

    void save(const doc::CSVDocument& document, const std::string& path) {
        PropertiesBuilder builder = document.documentProperties;
        document.documentProperties = std::move(builder.filePath(path));
        save(document);
    }
}

void rapidcsv::CSVDocument::load(const std::string& path) {
    using rapidcsv::read::wrapped;
    using rapidcsv::read::enumerate;
    using ParamType = std::tuple<std::size_t, std::string>;
    std::ifstream file (path, std::ios::in | std::ios::binary);

    auto reader =  r_transform(row_reader(file), [](std::vector<std::string>&& row) {
        MeshRow transformRow;

        auto zip = enumerate(wrapped(row));

        std::transform(std::make_move_iterator(std::begin(zip)),
                       std::make_move_iterator(std::end(zip)), std::inserter(transformRow,
                                                                             std::begin(transformRow)),
                       [](ParamType&& out) {
                           return std::make_pair(
                                   std::forward<std::size_t>(std::get<0>(std::forward<ParamType>(out))),
                                   std::forward<std::string>(std::get<1>(std::forward<ParamType>(out))));
                       }
        );
        return transformRow;
    });
}

#endif //RAPIDCSV_CSV_DOCUMENT_HPP
