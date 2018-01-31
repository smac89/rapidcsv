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
    class CSVDocument : public Document {
        using Document::Document;
        using rapidcsv::read::SimpleReader;

    public:
        using MeshRow = std::map<std::size_t, std::string>;
        using Elem = MeshRow::value_type;

        std::size_t rowCount() const {
            return this->_rowCount;
        }

        std::size_t columnCount() const {
            return this->_columnCount;
        }

        void load(const std::string &pPath);
        void save(const std::string &pPath) const;

        template<typename T>
        std::vector<T> GetColumn(const size_t columnIndex) const;

        template<typename T>
        std::vector<T> GetColumn(const std::string &columnName) const;

        template<typename T>
        std::vector<T> GetColumn(const size_t columnIndex, const T& fillValue) const;

        template<typename T>
        std::vector<T> GetColumn(const std::string &columnName, const T& fillValue) const;

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

        template<typename T>
        std::vector<T> _GetColumn(const size_t columnIndex) const {
            using rapidcsv::read::simpleReader;
            using rapidcsv::read::r_transform;
            using rapidcsv::read::r_copy_if;

            std::vector<T> column;
            auto begin = (documentProperties.hasHeader() ? std::next(std::begin(documentMesh))
                                                         : std::begin(documentMesh));
            auto end = std::end(documentMesh);

            auto reader = std::move(r_copy_if(r_transform(
                    simpleReader(begin, end), [&columnIndex](const MeshRow &row) {
                        auto finder = row.find(columnIndex);
                        return finder != std::end(row) ? std::make_pair(true, finder->second)
                                                       : std::make_pair(false, "");
                    }), [](const std::pair<bool, std::string>& content) { return std::get<0>(content); }));

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

        std::size_t getColumnIndex(const std::string &columnName) const {
            auto columnIter = columnNames.find(columnName);
            if (columnIter == columnNames.end()) {
                throw std::out_of_range("column not found: " + columnName);
            }
            return getColumnIndex(columnIter->second);
        }

        std::size_t getColumnIndex(const std::size_t columnIndex) const {
            const std::size_t normalizedColumn = columnIndex + (documentProperties.hasRowLabel() ? 1 : 0);
            if (columnIndex >= this->columnCount() || normalizedColumn >= columnCount()) {
                throw std::out_of_range(std::string("column out of range : ") + columnIndex);
            }
            return normalizedColumn;
        }

    private:
        std::vector<MeshRow> documentMesh;
        std::unordered_map<std::string, std::size_t> columnNames;
        std::unordered_map<std::string, std::size_t> rowNames;
        std::size_t _rowCount = 0;
        std::size_t _columnCount = 0;
        std::size_t maxColumnCount = 0;
    };
}

void rapidcsv::CSVDocument::load(const std::string& path) {
    using rapidcsv::read::wrapReadable;
    using rapidcsv::read::enumerate;
    using ParamType = std::tuple<std::size_t, std::string>;
    std::ifstream file (path, std::ios::in | std::ios::binary);

    auto reader =  r_transform(row_reader(file), [](std::vector<std::string>&& row) {
        MeshRow transformRow;

        auto zip = enumerate(wrapReadable(row));

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
        this->documentMesh.push_back(row);
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

    std::ofstream file(path, std::ios::out | std::ios::binary);

    std::transform(std::begin(documentMesh), std::end(documentMesh),
                   std::ostream_iterator<std::string>(file, to_string(documentProperties.rowSep())),
                   std::bind(&CSVDocument::toCsvRow, this));
}

template<typename T>
std::vector<T> rapidcsv::CSVDocument::GetColumn(const size_t columnIndex) const {
    std::size_t realColumnIndex = getColumnIndex(columnIndex);
    return _GetColumn(realColumnIndex);
}

template<typename T>
std::vector<T> GetColumn(const std::string &columnName) const {
    std::size_t columnIndex = getColumnIndex(columnName);
    return _GetColumn(columnIndex);
}

template<typename T>
std::vector<T> GetColumn(const size_t columnIndex, const T& fillValue) const {
    std::size_t realColumnIndex = getColumnIndex(columnIndex);
    return _GetColumn(columnIndex, fillValue);
}

template<typename T>
std::vector<T> GetColumn(const std::string &columnName, const T& fillValue) const {
    std::size_t columnIndex = getColumnIndex(columnName);
    return _GetColumn(columnIndex, fillValue);
}

#endif //RAPIDCSV_CSV_DOCUMENT_HPP
