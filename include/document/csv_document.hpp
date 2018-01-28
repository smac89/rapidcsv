#ifndef RAPIDCSV_CSV_DOCUMENT_HPP
#define RAPIDCSV_CSV_DOCUMENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <fstream>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <numeric>

#include "reader/simple_reader.hpp"
#include "document/properties.hpp"
#include "document/document.hpp"
#include "csv_reader.hpp"
#include "util/fp.hpp"

namespace rapidcsv {
    class CSVDocument : public Document {
        using Document::Document;
    public:
        using Elem = std::pair<std::size_t, std::string>;
        using MeshRow = std::vector<Elem>;

        std::size_t rowCount() const {
            return this->_rowCount;
        }

        std::size_t columnCount() const {
            return this->_columnCount;
        }

        void load(const std::string &pPath);
        void save(const std::string &pPath) const;

    private:
        std::string toCsvRow(const std::vector<std::string> &row) {
            if (!row.empty()) {
                std::ostringstream oss;
                std::copy(std::begin(row), std::end(row), std::ostream_iterator<std::string>(
                        oss, std::string(1, documentProperties.fieldSep()).c_str()));
                std::string joined = oss.str();
                return joined.erase(joined.rfind(documentProperties.fieldSep()));
            }
            return {};
        }

        template<typename T>
        std::vector<T> _GetColumn(const size_t columnIndex) const {
            using rapidcsv::read::Reader;
            using rapidcsv::read::SimpleReader;
            using Interim = std::pair<bool, std::string>;
            using TransFormType = Reader<Interim>;

            std::vector<T> column;
            auto begin = (documentProperties.hasHeader() ? std::next(std::begin(documentMesh))
                                                         : std::begin(documentMesh));
            auto end = std::end(documentMesh);

            auto reader = std::move(r_copy_if(r_transform(
                    SimpleReader(begin, end), [&columnIndex](const MeshRow &row) {
                        auto lb = std::lower_bound(std::begin(row), std::end(row), columnIndex,
                                                   [](const Elem& element, const std::size_t& value) {
                                                       return element.first < value;
                                                   });
                        return (lb != std::end(row) && lb->first == columnIndex) ? std::make_pair(true, lb->second)
                                                                                 : std::make_pair(false, "");
                    }), [](const Interim& content) { return std::get<0>(content); }));

            std::transform(std::begin(reader),
                           std::end(reader),
                           std::back_inserter(column), [](Interim&& data) {
                return std::move(convert::convert_to_val<T>(std::get<1>(data)));
            });

            return column;
        }

        std::size_t getColumnIndex(const std::string &columnName) const {
            auto columnIter = columnNames.find(columnName);
            if (columnIter == columnNames.end()) {
                throw std::out_of_range("column not found: " + columnName);
            }
            return columnIter->second;
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
    };
}

void rapidcsv::CSVDocument::load(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);

    auto reader =  r_transform(row_reader(std::istreambuf_iterator<char>{file},
                                          std::istreambuf_iterator<char>{}), [](std::vector<std::string>&& data) {
        MeshRow transformRow;
        std::transform(std::make_move_iterator(std::begin(data)),
                       std::end(data),
                       std::back_inserter(transformRow), [i = 0](std::string&& field))
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
        std::size_t i = 0;
        for (auto &columnName : documentMesh[0]) {
            columnNames[columnName] = i++;
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

#endif //RAPIDCSV_CSV_DOCUMENT_HPP
