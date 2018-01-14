/*
 * rapidcsv.h
 *
 * URL:      https://github.com/d99kris/rapidcsv
 * Version:  1.1
 *
 * Copyright (C) 2017 Kristofer Berggren
 * All rights reserved.
 * 
 * rapidcsv is distributed under the BSD 3-Clause license, see LICENSE for details.
 *
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <unordered_set>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <tuple>
#include <functional>
#include <type_traits>
#include <exception>
#include <iterator>
#include <cstddef>
#include <utility>
#include <array>
#include <memory>
#include <fstream>

#include "csv_reader.hpp"
#include "csv_iterator.hpp"
#include "csv_constants.hpp"

namespace rapidcsv {

    namespace convert {
        template<typename T>
        std::string convert_to_string(const T &pVal) {
            std::ostringstream out;
            out << pVal;
            return out.str();
        }

        template<typename T>
        T convert_to_val(const std::string &pStr) {
            std::istringstream in(pStr);
            T pVal;
            in >> pVal;
            return pVal;
        }
    }

    template <typename P>
    class CSVProperty {
        friend class Properties;
        P value;
    public:
        CSVProperty(const P& _value): value(_value) {}
    };

    struct CSVRowSep: public CSVProperty<std::array<char, 2>> {
        using CSVProperty::CSVProperty;
        CSVRowSep(const char (&rowSep)[2]): CSVProperty({rowSep[0]}) {}
        CSVRowSep(const char (&rowSep)[3]): CSVProperty({rowSep[0], rowSep[1]}) {}
    };

    struct CSVHasRowLabel: public CSVProperty<bool> {
        using CSVProperty::CSVProperty;
    };

    struct CSVHasColLabel: public CSVProperty<bool> {
        using CSVProperty::CSVProperty;
    };

    struct CSVFieldSep: public CSVProperty<char> {
        using CSVProperty::CSVProperty;
    };

    struct CSVQuote: public CSVProperty<char> {
        using CSVProperty::CSVProperty;
    };

    class Properties {
        friend class PropertiesBuilder;
    public:
        std::string mPath;
        const int mColumnNameIdx;
        const int mRowNameIdx;
        const bool mHasCR;
        const bool mhasRowLabel;
        const bool mHasColLabel;

    public:
        Properties(): Properties("", {CRLF}, {'"'}, {','}, {false}, {false}) {}

        explicit Properties(const std::string &pPath, const CSVRowSep& rowSep, const CSVQuote& quote,
                            const CSVFieldSep& fieldSep, const CSVHasColLabel& hasColLabel,
                            const CSVHasRowLabel& hasRowLabel)
                : mPath(pPath), mColumnNameIdx(0), mRowNameIdx(0),
                  mHasCR(false), mhasRowLabel(false), mHasColLabel(false),
                  _quote(quote), _fieldSep(fieldSep), _hasColLabel(hasColLabel),
                  _hasRowLabel(hasRowLabel), _rowSep(rowSep) {
        }

        char quote() const {
            return _quote.value;
        }

        char fieldSep() const {
            return _fieldSep.value;
        }

        bool hasColLabel() const {
            return _hasColLabel.value;
        }

        bool hasRowLabel() const {
            return _hasRowLabel.value;
        }

        std::array<char, 2> rowSep() const {
            return _rowSep.value;
        }

    private:
        CSVQuote _quote;
        CSVFieldSep _fieldSep;
        CSVHasColLabel _hasColLabel;
        CSVHasRowLabel _hasRowLabel;
        CSVRowSep _rowSep;
    };

    class PropertiesBuilder {
    public:
        PropertiesBuilder(): PropertiesBuilder(Properties()) {}

        explicit PropertiesBuilder (const Properties& properties):
                _quote(properties._quote), _fieldSep(properties._fieldSep),
                _hasColLabel(properties._hasColLabel), _hasRowLabel(properties._hasRowLabel),
                _rowSep(properties._rowSep) {}

        explicit PropertiesBuilder (Properties&& properties):
                _quote(std::move(properties)._quote), _fieldSep(std::move(properties)._fieldSep),
                _hasColLabel(std::move(properties)._hasColLabel), _hasRowLabel(std::move(properties)._hasRowLabel),
                _rowSep(std::move(properties)._rowSep) {}

        PropertiesBuilder& rowSep(const std::array<char, 2> rowSep) {
            this->_rowSep = CSVRowSep(std::move(rowSep));
            return *this;
        }

        PropertiesBuilder& rowSep(const char (&rowSep)[3]) {
            this->_rowSep = CSVRowSep(rowSep);
            return *this;
        }

        PropertiesBuilder& rowSep(const char (&rowSep)[2]) {
            this->_rowSep = CSVRowSep(rowSep);
            return *this;
        }

        PropertiesBuilder& quote(const char quote) {
            this->_quote = CSVQuote(quote);
            return *this;
        }

        PropertiesBuilder& fieldSep(const char fieldSep) {
            this->_fieldSep = CSVFieldSep(fieldSep);
            return *this;
        }

        PropertiesBuilder& hasColLabel() {
            this->_hasColLabel = CSVHasColLabel(true);
            return *this;
        }

        PropertiesBuilder& hasRowLabel() {
            this->_hasRowLabel = CSVHasRowLabel(true);
            return *this;
        }

        PropertiesBuilder& filePath(const std::string& filePath) {
            this->path = filePath;
            return *this;
        }

        Properties build() const {
            return Properties(path, _rowSep, _quote, _fieldSep, _hasColLabel, _hasRowLabel);
        }

        operator Properties() const { return build(); }

    private:
        CSVQuote _quote;
        CSVFieldSep _fieldSep;
        CSVHasColLabel _hasColLabel;
        CSVHasRowLabel _hasRowLabel;
        CSVRowSep _rowSep;
        std::string path;
    };

    class Document;

    namespace convert {
        template <>
        inline std::string convert_to_string(const std::string& pVal) {
            return pVal;
        }

        template <>
        inline std::string convert_to_val(const std::string& pStr) {
            return pStr;
        }
    }
}

class rapidcsv::Document {
private:
    Properties mProperties;
    std::vector<std::vector<std::string>> mData;
    std::map<std::string, std::size_t> mColumnNames;
    std::map<std::string, std::size_t> mRowNames;
    std::size_t rowCount = 0;
    std::size_t columnCount = 0;

public:
    // Contructors --------------------------------------------------------
    explicit Document(const std::string& pPath): Document(PropertiesBuilder().filePath(pPath)) { }

    explicit Document(const Properties &pProperties): mProperties(pProperties) {
        if (!mProperties.mPath.empty()) {
            read_csv(mProperties.mPath);
        }
    }

    explicit Document(Properties &&pProperties): mProperties(std::move(pProperties)) {
        if (!mProperties.mPath.empty()) {
            read_csv(mProperties.mPath);
        }
    }

    explicit Document(const Document &pDocument) :
            mProperties(pDocument.mProperties),
            mData(pDocument.mData),
            mColumnNames(pDocument.mColumnNames),
            mRowNames(pDocument.mRowNames) {}

    explicit Document(Document &&document) :
            mProperties(std::move(document).mProperties),
            mData(std::move(document).mData),
            mColumnNames(std::move(document).mColumnNames),
            mRowNames(std::move(document).mRowNames) {}

    // Destructors --------------------------------------------------------
    virtual ~Document() {}

    // Document Methods ---------------------------------------------------
    void Load() { Load(mProperties.mPath); }
    void Load(const std::string& pPath) { read_csv(pPath); }

    void Save() const { Save(mProperties.mPath); }
    void Save(const std::string &pPath) const  { write_csv(pPath); }

    // Column Methods -----------------------------------------------------

//    template<typename T, std::size_t colIndex = 0>
//    std::vector<T> GetColumn(const size_t colIndex) const {
//        const std::size_t columnIdx = colIndex + (mProperties.hasRowLabel() ? 1 : 0);
//        if (mProperties.hasRowLabel()) {
//
//        }
//        std::vector<T> column;
//        for (auto itRow = mData.begin(); itRow != mData.end(); ++itRow) {
//            if (std::distance(mData.begin(), itRow) > mProperties.mColumnNameIdx) {
//                T val = convert::convert_to_val<T>(itRow->at(columnIdx));
//                column.push_back(val);
//            }
//        }
//        return column;
//    }

    template<typename T>
    std::vector<T> GetColumn(const size_t columnIndex) const {
        const std::size_t normalizedIndex = columnIndex + (mProperties.hasRowLabel() ? 1 : 0);

        std::vector<T> column;
        std::transform((mProperties.hasColLabel() ? std::next(std::begin(mData)) : std::begin(mData)),
                       std::end(mData),
                       std::back_inserter(column),
                       [&normalizedIndex](const std::vector<std::string>& row) {
                           return convert::convert_to_val<T>(row.at(normalizedIndex));
                       }
        );
        return column;
    }

    template<typename T>
    std::vector<T> GetColumn(const std::string &columnName) const {
        std::size_t columnIndex;
        bool found = false;
        std::tie(found, columnIndex) = GetColumnIdx(columnName);
        if (!found) {
            throw std::out_of_range("column not found: " + columnName);
        }
        return GetColumn<T>(columnIndex);
    }

    template<typename T>
    void SetColumn(const size_t columnIndex, const std::vector<T> &pColumn) {
        const size_t columnIdx = columnIndex + (mProperties.mRowNameIdx + 1);

        while (pColumn.size() + (mProperties.mColumnNameIdx + 1) > GetDataRowCount()) {
            std::vector<std::string> row;
            row.resize(GetDataColumnCount());
            mData.push_back(row);
        }

        if ((columnIdx + 1) > GetDataColumnCount()) {
            for (auto itRow = mData.begin(); itRow != mData.end(); ++itRow) {
                itRow->resize(columnIdx + 1 + (mProperties.mRowNameIdx + 1));
            }
        }

        for (auto itRow = pColumn.begin(); itRow != pColumn.end(); ++itRow) {
            std::string str = convert::convert_to_string(*itRow);
            mData.at(std::distance(pColumn.begin(), itRow) + mProperties.mColumnNameIdx + 1).at(columnIdx) = str;
        }
    }

    template<typename T>
    void SetColumn(const std::string &columnName, const std::vector<T> &column) {
        std::size_t columnIndex;
        bool found = false;
        std::tie(found, columnIndex) = GetColumnIdx(columnName);
        if (!found) {
            throw std::out_of_range("column not found: " + columnName);
        }
        SetColumn<T>(columnIndex, column);
    }

    void RemoveColumn(const size_t columnIndex) {
        const std::size_t columnIdx = columnIndex + (mProperties.mRowNameIdx + 1);
        for (auto itRow = mData.begin(); itRow != mData.end(); ++itRow) {
            itRow->erase(itRow->begin() + columnIdx);
        }
    }

    void RemoveColumn(const std::string &columnName) {
        std::size_t columnIndex;
        bool found = false;
        std::tie(found, columnIndex) = GetColumnIdx(columnName);
        if (!found) {
            throw std::out_of_range("column not found: " + columnName);
        }
        RemoveColumn(columnIndex);
    }

    size_t GetColumnCount() const {
        return (mData.size() > 0) ? (mData.at(0).size() - (mProperties.mRowNameIdx + 1)) : 0;
    }

    // Row Methods --------------------------------------------------------
    template<typename T>
    std::vector<T> GetRow(const size_t rowIndex) const {
        const std::size_t rowIdx = rowIndex + (mProperties.mColumnNameIdx + 1);
        std::vector<T> row;
        for (auto itCol = mData.at(rowIdx).begin(); itCol != mData.at(rowIdx).end(); ++itCol) {
            if (std::distance(mData.at(rowIdx).begin(), itCol) > mProperties.mRowNameIdx) {
                T val = convert::convert_to_val<T>(*itCol);
                row.push_back(val);
            }
        }
        return row;
    }

    template<typename T>
    std::vector<T> GetRow(const std::string &rowName) const {
        bool found = false;
        std::size_t rowIndex;
        std::tie(found, rowIndex) = GetRowIdx(rowName);
        if (!found) {
            throw std::out_of_range("row not found: " + rowName);
        }
        return GetRow<T>(rowIndex);
    }

    template<typename T>
    void SetRow(const size_t rowIndex, const std::vector<T> &pRow) {
        const size_t rowIdx = rowIndex + (mProperties.mColumnNameIdx + 1);

        while ((rowIdx + 1) > GetDataRowCount()) {
            std::vector<std::string> row;
            row.resize(GetDataColumnCount());
            mData.push_back(row);
        }

        if (pRow.size() > GetDataColumnCount()) {
            for (auto itRow = mData.begin(); itRow != mData.end(); ++itRow) {
                itRow->resize(pRow.size() + (mProperties.mRowNameIdx + 1));
            }
        }

        for (auto itCol = pRow.begin(); itCol != pRow.end(); ++itCol) {
            std::string str = convert::convert_to_string(*itCol);
            mData.at(rowIdx).at(std::distance(pRow.begin(), itCol) + mProperties.mRowNameIdx + 1) = str;
        }
    }

    template<typename T>
    void SetRow(const std::string &rowName, const std::vector<T> &pRow) {
        bool found = false;
        std::size_t rowIndex;
        std::tie(found, rowIndex) = GetRowIdx(rowName);
        if (!found) {
            throw std::out_of_range("row not found: " + rowName);
        }
        return SetRow<T>(rowIndex, pRow);
    }

    void RemoveRow(const size_t rowIndex) {
        const std::size_t rowIdx = rowIndex + (mProperties.mColumnNameIdx + 1);
        mData.erase(mData.begin() + rowIdx);
    }

    void RemoveRow(const std::string &rowName) {
        bool found = false;
        std::size_t rowIndex;
        std::tie(found, rowIndex) = GetRowIdx(rowName);
        if (!found) {
            throw std::out_of_range("row not found: " + rowName);
        }

        RemoveRow(rowIndex);
    }

    size_t GetRowCount() const {
        return mData.size() - (mProperties.mColumnNameIdx + 1);
    }

    // Cell Methods -------------------------------------------------------
    template<typename T>
    T GetCell(const size_t columnIndex, const size_t rowIndex) const {
        const std::size_t columnIdx = columnIndex + (mProperties.mRowNameIdx + 1);
        const std::size_t rowIdx = rowIndex + (mProperties.mColumnNameIdx + 1);

        T val = convert::convert_to_val<T>(mData.at(rowIdx).at(columnIdx));
        return val;
    }

    template<typename T>
    T GetCell(const std::string &columnName, const std::string &rowName) const {
        std::size_t columnIndex;
        bool found = false;
        std::tie(found, columnIndex) = GetColumnIdx(columnName);
        if (!found) {
            throw std::out_of_range("column not found: " + columnName);
        }

        std::size_t rowIndex;
        std::tie(found, rowIndex) = GetRowIdx(columnName);
        if (!found) {
            throw std::out_of_range("row not found: " + rowName);
        }

        return GetCell<T>(columnIndex, rowIndex);
    }

    template<typename T>
    void SetCell(const size_t columnIndex, const size_t rowIndex, const T &pCell) {
        const size_t columnIdx = columnIndex + (mProperties.mRowNameIdx + 1);
        const size_t rowIdx = rowIndex + (mProperties.mColumnNameIdx + 1);

        while ((rowIdx + 1) > GetDataRowCount()) {
            std::vector<std::string> row;
            row.resize(GetDataColumnCount());
            mData.push_back(row);
        }

        if ((columnIdx + 1) > GetDataColumnCount()) {
            for (auto itRow = mData.begin(); itRow != mData.end(); ++itRow) {
                itRow->resize(columnIdx + 1);
            }
        }

        std::string str = convert::convert_to_string(pCell);
        mData.at(rowIdx).at(columnIdx) = str;
    }

    template<typename T>
    void SetCell(const std::string &columnName, const std::string &rowName, const T &pCell) {
        std::size_t columnIndex;
        bool found = false;
        std::tie(found, columnIndex) = GetColumnIdx(columnName);
        if (!found) {
            throw std::out_of_range("column not found: " + columnName);
        }

        std::size_t rowIndex;
        std::tie(found, rowIndex) = GetRowIdx(rowName);
        if (!found) {
            throw std::out_of_range("row not found: " + rowName);
        }

        SetCell<T>(columnIndex, rowIndex, pCell);
    }

    // Label Methods ------------------------------------------------------
    void SetColumnLabel(size_t columnIndex, const std::string &columnLabel) {
        const std::size_t columnIdx = columnIndex + (mProperties.mRowNameIdx + 1);
        mColumnNames[columnLabel] = columnIdx;
        if (mProperties.mColumnNameIdx >= 0) {
            mData.at(mProperties.mColumnNameIdx).at(columnIdx) = columnLabel;
        }
    }

    void SetRowLabel(size_t rowIndex, const std::string &rowLabel) {
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

    void read_csv(const std::string &path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);

        auto reader = rowReader(std::istreambuf_iterator<char>{file},
                                std::istreambuf_iterator<char>{});

        for (auto& row : reader) {
            this->rowCount++;
            this->columnCount = std::max(this->columnCount, row.size());
            this->mData.push_back(row);
        }

        // Set up column labels
        if (mProperties.hasColLabel()) {
            if (this->rowCount > 0) {
                --this->rowCount;
            }
            std::size_t i = 0;
            for (auto &columnName : mData[0]) {
                mColumnNames[columnName] = i++;
            }
        }

        // Set up row labels
        if (mProperties.hasRowLabel()) {
            if (this->columnCount > 0) {
                --this->columnCount;
            }
            std::size_t i = 0;
            for (auto &dataRow : mData) {
                mRowNames[dataRow[0]] = i++;
            }
        }
    }

    void write_csv(const std::string& path) const {
        std::ofstream file(path, std::ios::out | std::ios::binary);

        std::transform(std::begin(mData), std::end(mData),
                       std::ostream_iterator<std::string>(file, mProperties.rowSep().data()),
                       std::bind(&Document::to_csv_row, this));
    }

    std::string to_csv_row(const std::vector<std::string>& row) {
        if (!row.empty()) {
            std::ostringstream oss;
            std::copy(std::begin(row), std::end(row), std::ostream_iterator<std::string>(
                    oss, std::string(1, mProperties.fieldSep()).c_str()));
            std::string joined = oss.str();
            return joined.erase(joined.rfind(mProperties.fieldSep()));
        }
        return {};
    }

    std::tuple<bool, std::size_t> GetColumnIdx(const std::string &columnName) const {
        if (mProperties.hasColLabel()) {
            auto columnIter = mColumnNames.find(columnName);
            if (columnIter != mColumnNames.end()) {
                return std::make_tuple(true, columnIter->second - (mProperties.hasRowLabel() ? 1 : 0));
            }
        }
        return std::make_tuple(false, -1);
    }

    std::tuple<bool, std::size_t> GetRowIdx(const std::string &rowName) const {
        if (mProperties.mRowNameIdx >= 0) {
            auto rowIter = mRowNames.find(rowName);
            if (rowIter != mRowNames.end()) {
                return std::make_tuple(true, rowIter->second - (mProperties.mColumnNameIdx + 1));
            }
        }
        return std::make_tuple(false, -1);
    }

    std::size_t GetDataRowCount() const {
        return this->rowCount;
    }

    std::size_t GetDataColumnCount() const {
        return this->columnCount;
    }
};
