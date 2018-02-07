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

#include "reader/simple_reader.hpp"
#include "reader/supply_reader.hpp"
#include "reader/readable_wrapper.hpp"
#include "document/properties.hpp"
#include "document/document.hpp"
#include "csv_reader.hpp"
#include "util/fp.hpp"

namespace rapidcsv {
    namespace doc {
        class CSVDocument : public Document {
            using Document::Document;
            using rapidcsv::read::SimpleReader;

            using MeshRow = std::unordered_map<std::size_t, std::string>;
            using Entry = MeshRow::value_type;

        public:

            //////////////////////////////////////////////////////////
            /////////////////////// COLUMNS //////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            template<typename T>
            virtual std::vector<T> GetColumn(const size_t columnIndex, const T& fillValue) const override {
                return _GetColumn(getColumnIndex(columnIndex), fillValue);
            }

            template<typename T>
            virtual std::vector<T> GetColumn(const std::string &columnName, const T& fillValue) const override {
                return _GetColumn(getColumnIndex(columnName), fillValue);
            }

            template<typename T>
            virtual std::vector<T> GetColumn(const size_t columnIndex) const {
                return _GetColumn(getColumnIndex(columnIndex));
            }

            template<typename T>
            virtual std::vector<T> GetColumn(const std::string &columnName) const {
                return _GetColumn(getColumnIndex(columnName));
            }

            // SET
            template<>
            virtual std::size_t SetColumn(const size_t columnIndex, const std::vector<std::string>& colData) {
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
            virtual std::size_t SetColumn(const size_t columnIndex, std::vector<std::string>&& colData) {
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
            virtual std::size_t SetColumn(const std::string &columnName, const std::vector<std::string>& colData) {
                return SetColumn(getColumnIndex(columnName), colData);
            }

            template<>
            virtual std::size_t SetColumn(const std::string &columnName, std::vector<std::string>&& colData) {
                return SetColumn(getColumnIndex(columnName), std::forward<std::vector<std::string>>(colData));
            }

            // REMOVE
            virtual std::size_t RemoveColumn(const size_t columnIndex) {
                std::size_t realColumnIndex = getColumnIndex(columnIndex);

                for (const auto& row : documentMesh) {
                    row.erase(realColumnIndex);
                }

                return documentMesh.size();
            }

            virtual std::size_t RemoveColumn(const std::string &columnName) {
                return RemoveColumn(getColumnIndex(columnName));
            }

            //////////////////////////////////////////////////////////
            ///////////////////////// ROWS ///////////////////////////
            //////////////////////////////////////////////////////////

            // GET
            virtual std::vector<std::string> GetRow(const size_t rowIndex) const {
                return _GetRow(getRowIndex(rowIndex));
            }

            virtual std::vector<std::string> GetRow(const std::string &rowName) const {
                return _GetRow(getRowIndex(rowName));
            }

            // SET
            virtual void SetRow(const size_t rowIndex, const std::vector<std::string> &row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                MeshRow meshRow(row.size());
                auto&& seq = enumerate(wrapped(row));
                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)), std::begin(meshRow));

                documentMesh[getRowIndex(rowIndex)] = std::move(meshRow);
            }

            virtual void SetRow(const size_t rowIndex, std::vector<std::string> &&row) {
                using rapidcsv::read::enumerate;
                using rapidcsv::read::wrapped;

                MeshRow meshRow(row.size());
                auto&& seq = enumerate(wrapped(std::forward<std::vector<std::string>>(std::move(row))));
                std::copy(std::make_move_iterator(std::begin(seq)),
                          std::make_move_iterator(std::end(seq)), std::begin(meshRow));

                documentMesh[getRowIndex(rowIndex)] = std::move(meshRow);
            }

            virtual void SetRow(const std::string& rowName, const std::vector<std::string> &row) {
                SetRow(getRowIndex(rowName), row);
            }

            virtual void SetRow(const std::string& rowName, std::vector<std::string> &&row) {
                SetRow(getRowIndex(rowName), std::forward<std::vector<std::string>>(row));
            }

            virtual std::vector<std::string> RemoveRow (const size_t rowIndex) {
                const 
                MeshRow& meshRow =
            }

            virtual std::vector<std::string> RemoveRow(const std::string &rowName) = 0;
        private:
            std::string toCsvRow(const MeshRow &row) const {
                if (!row.empty()) {
                    std::ostringstream oss;
                    auto rowIter = std::begin(row);
                    for (std::size_t i = 0; i < maxColumnCount; i++) {
                        if (rowIter != std::end(row)) {
                            if (rowIter->first == i) {
                                oss << rowIter->second;
                            }
                            rowIter++;
                        }

                        if (i < maxColumnCount - 1) {
                            oss << documentProperties.fieldSep();
                        }
                    }
                    return oss.str();
                }
                return {};
            }

            template <typename In, typename Pos>
            std::string to_csv_row(In begin, In end, Pos func) const {
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

            template<typename T>
            std::vector<T> _GetColumn(const size_t columnIndex) const {
                using rapidcsv::read::simpleReader;
                using rapidcsv::read::r_transform;
                using rapidcsv::read::r_copy_if;

                std::vector<T> column;
                auto begin = (documentProperties.hasHeader() ? std::next(std::begin(documentMesh))
                                                             : std::begin(documentMesh));
                auto end = std::end(documentMesh);

                auto reader = std::move(r_copy_if(r_transform(simpleReader(begin, end), [&columnIndex](const MeshRow &row) {
                    auto finder = row.find(columnIndex);
                    return finder != std::end(row) ? std::make_pair(true, finder->second)
                                                   : std::make_pair(false, "");
                }), &std::get<0>));

                std::transform(std::begin(reader),
                               std::end(reader),
                               std::back_inserter(column), [](std::pair<bool, std::string>&& data) {
                            return std::move(convert::convert_to_val<T>(std::get<1>(data)));
                        });

                return column;
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

            std::size_t getColumnIndex(const std::string &columnName) const {
                auto columnIter = columnNames.find(columnName);
                if (columnIter == columnNames.end()) {
                    throw std::out_of_range("column not found: " + columnName);
                }
                return columnIter->second;
            }

            std::size_t getColumnIndex(const std::size_t columnIndex) const {
                auto normalizedColumn = columnIndex + (documentProperties.hasRowLabel() ? 1 : 0);
                if (columnIndex >= this->columnCount() || normalizedColumn >= columnCount()) {
                    throw std::out_of_range(std::string("column out of range : ") + columnIndex);
                }
                return normalizedColumn;
            }

            std::size_t getRowIndex(const std::string &rowName) const {
                auto rowIter = rowNames.find(std::cref(rowName));
                if (rowIter == std::end(rowNames)) {
                    throw std::out_of_range("Row label not found");
                }
                return rowIter->second;
            }

            std::size_t getRowIndex(const std::size_t rowIndex) const {
                auto normalizedRow = rowIndex + (documentProperties.hasHeader() ? 1 : 0);
                if (rowIndex >= maxRowCount() || normalizedRow >= maxRowCount()) {
                    throw std::out_of_range(std::string("Row index out of range ") + rowIndex);
                }
                return normalizedRow;
            }

            // Label Methods ------------------------------------------------------
            void setColumnLabel(const std::string& oldColumnLabel, const std::string &newColumnLabel) {
                auto realColumnIndex = getColumnIndex(oldColumnLabel);
                columnNames.erase(oldColumnLabel);
                columnNames[newColumnLabel] = realColumnIndex;
            }

            void setRowLabel(const std::string& oldRowLabel, const std::string &newRowLabel) {
                bool found;
                std::tie(found, std::ignore) = GetRow(rowIndex)
                if (mProperties.hasRowLabel()) {

                }
                const std::size_t rowIdx = rowIndex + (mProperties.mColumnNameIdx + 1);
                mRowNames[rowLabel] = rowIdx;
                if (mProperties.mRowNameIdx >= 0) {
                    mData.at(rowIdx).at(static_cast<std::size_t>(mProperties.mRowNameIdx)) = rowLabel;
                }
            }

        private:
            std::vector<MeshRow> documentMesh;
            std::unordered_map<std::string, std::size_t> columnNames;
            std::unordered_map<std::reference_wrapper<std::string>, std::size_t> rowNames;
            std::size_t _rowCount = 0;
            std::size_t _columnCount = 0;
            std::size_t maxColumnCount = 0;
        };
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

    for (auto&& row : reader) {
        this->_rowCount++;
        this->_columnCount = std::max(this->columnCount, row.size());
        this->documentMesh.emplace_back(std::move(row));
    }

    // Set up column labels
    if (documentProperties.hasHeader()) {
        if (this->_rowCount > 0) {
            --this->_rowCount;
        }
        for (auto &field : documentMesh[0]) {
            columnNames[field.second] = field.first;
        }
    }

    // Set up row labels
    if (documentProperties.hasRowLabel()) {
        if (this->_columnCount > 0) {
            --this->_columnCount;
        }
        std::size_t i = 0;
        for (auto &dataRow : documentMesh) {
            rowNames[dataRow[0]] = i++;
        }
    }
}

void rapidcsv::CSVDocument::save(const std::string &path) const {
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

template<typename T>
std::vector<T> rapidcsv::CSVDocument::GetColumn(const size_t columnIndex) const {
    auto realColumnIndex = getColumnIndex(columnIndex);
    return _GetColumn(realColumnIndex);
}

template<typename T>
std::vector<T> GetColumn(const std::string &columnName) const {
    auto columnIndex = getColumnIndex(columnName);
    return _GetColumn(columnIndex);
}

template<typename T>
std::vector<T> GetColumn(const size_t columnIndex, const T& fillValue) const {
    auto realColumnIndex = getColumnIndex(columnIndex);
    return _GetColumn(columnIndex, fillValue);
}

template<typename T>
std::vector<T> GetColumn(const std::string &columnName, const T& fillValue) const {
    auto columnIndex = getColumnIndex(columnName);
    return _GetColumn(columnIndex, fillValue);
}

#endif //RAPIDCSV_CSV_DOCUMENT_HPP
