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
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_set>
#include <sstream>
#include <string>
#include <vector>
#include <type_traits>
#include <iterator>
#include <cstddef>
#include <utility>

static constexpr auto bufLength = 64 * 1024;

namespace rapidcsv {

    namespace detail {
        template <class ...T>
        struct are_same;

        template <class A, class B, class ...T>
        struct are_same<A, B, T...> {
            enum: bool { value = std::is_same<A, B>::value && are_same<B, T...>::value };
        };

        template <class A>
        struct are_same<A> : std::true_type {};
    }

    namespace convert {
        template <typename T>
        std::string convert_to_string(const T& pVal) {
            std::ostringstream out;
            out << pVal;
            return out.str();
        }

        template <typename T>
        T convert_to_val(const std::string& pStr) {
            std::istringstream in(pStr);
            T pVal;
            in >> pVal;
            return pVal;
        }
    }

    enum class CSVProperty : bool {
        hasCR = false,
        firstRowHeader = false,
        firstColHeader = false
    };

    struct Properties {
        template <typename... Prop, typename = std::enable_if<detail::are_same<CSVProperty, Prop...>::value>>
        explicit Properties(const std::string &pPath = "",
                            const int pColumnNameIdx = 0,
                            const int pRowNameIdx = 0, Prop... csvProperties)
                : mPath(pPath), mColumnNameIdx(pColumnNameIdx), mRowNameIdx(pRowNameIdx) {
//            std::unordered_set<CSVProperty> csv_properties {std::begin(csv_properties), std::end(csv_properties)};
        }

        std::string mPath;
        int mColumnNameIdx;
        int mRowNameIdx;
        bool mHasCR = false;
        bool mHeader;
    };

    class Document {
    private:
        Properties mProperties;
        std::vector<std::vector<std::string>> mData;
        std::map<std::string, std::size_t> mColumnNames;
        std::map<std::string, std::size_t> mRowNames;

    public:
        // Contructors --------------------------------------------------------
        explicit Document(const std::string pPath) {
            mProperties.mPath = pPath;
            ReadCsv(mProperties.mPath);
        }

        explicit Document(const Properties &pProperties)
                : mProperties(pProperties) {
            if (!mProperties.mPath.empty()) {
                ReadCsv(mProperties.mPath);
            }
        }

        explicit Document(const Document &pDocument) :
                mProperties(pDocument.mProperties),
                mData(pDocument.mData),
                mColumnNames(pDocument.mColumnNames),
                mRowNames(pDocument.mRowNames) {}

        explicit Document(const Document &&document) :
                mProperties(std::move(document.mProperties)),
                mData(std::move(document.mData)),
                mColumnNames(std::move(document.mColumnNames)),
                mRowNames(std::move(document.mRowNames)) {}

        // Destructors --------------------------------------------------------
        virtual ~Document() {}

        // Document Methods ---------------------------------------------------
        void Load(const std::string pPath) {
            mProperties.mPath = pPath;
            ReadCsv(mProperties.mPath);
        }

        void Save(const std::string &pPath = std::string()) {
            if (!pPath.empty()) {
                mProperties.mPath = pPath;
            }
            write_csv();
        }

        // Column Methods -----------------------------------------------------
        template<typename T>
        std::vector<T> GetColumn(const size_t pColumnIdx) const {
            const std::size_t columnIdx = pColumnIdx + (mProperties.mRowNameIdx + 1);
            std::vector<T> column;
            for (auto itRow = mData.begin(); itRow != mData.end(); ++itRow) {
                if (std::distance(mData.begin(), itRow) > mProperties.mColumnNameIdx) {
                    T val = convert::convert_to_val<T>(itRow->at(columnIdx));
                    column.push_back(val);
                }
            }
            return column;
        }

        template<typename T>
        std::vector<T> GetColumn(const std::string &pColumnName) const {
            const int columnIdx = GetColumnIdx(pColumnName);
            if (columnIdx < 0) {
                throw std::out_of_range("column not found: " + pColumnName);
            }
            return GetColumn<T>(static_cast<std::size_t>(columnIdx));
        }

        template<typename T>
        void SetColumn(const size_t pColumnIdx, const std::vector<T> &pColumn) {
            const size_t columnIdx = pColumnIdx + (mProperties.mRowNameIdx + 1);

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
        void SetColumn(const std::string &pColumnName, const std::vector<T> &pColumn) {
            const int columnIdx = GetColumnIdx(pColumnName);
            if (columnIdx < 0) {
                throw std::out_of_range("column not found: " + pColumnName);
            }
            SetColumn<T>(static_cast<std::size_t>(columnIdx), pColumn);
        }

        void RemoveColumn(const size_t pColumnIdx) {
            const std::size_t columnIdx = pColumnIdx + (mProperties.mRowNameIdx + 1);
            for (auto itRow = mData.begin(); itRow != mData.end(); ++itRow) {
                itRow->erase(itRow->begin() + columnIdx);
            }
        }

        void RemoveColumn(const std::string &pColumnName) {
            int columnIdx = GetColumnIdx(pColumnName);
            if (columnIdx < 0) {
                throw std::out_of_range("column not found: " + pColumnName);
            }

            RemoveColumn(static_cast<std::size_t>(columnIdx));
        }

        size_t GetColumnCount() const {
            return (mData.size() > 0) ? (mData.at(0).size() - (mProperties.mRowNameIdx + 1)) : 0;
        }

        // Row Methods --------------------------------------------------------
        template<typename T>
        std::vector<T> GetRow(const size_t pRowIdx) const {
            const std::size_t rowIdx = pRowIdx + (mProperties.mColumnNameIdx + 1);
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
        std::vector<T> GetRow(const std::string &pRowName) const {
            int rowIdx = GetRowIdx(pRowName);
            if (rowIdx < 0) {
                throw std::out_of_range("row not found: " + pRowName);
            }
            return GetRow<T>(static_cast<std::size_t>(rowIdx));
        }

        template<typename T>
        void SetRow(const size_t pRowIdx, const std::vector<T> &pRow) {
            const size_t rowIdx = pRowIdx + (mProperties.mColumnNameIdx + 1);

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
        void SetRow(const std::string &pRowName, const std::vector<T> &pRow) {
            int rowIdx = GetRowIdx(pRowName);
            if (rowIdx < 0) {
                throw std::out_of_range("row not found: " + pRowName);
            }
            return SetRow<T>(static_cast<std::size_t>(rowIdx), pRow);
        }

        void RemoveRow(const size_t pRowIdx) {
            const std::size_t rowIdx = pRowIdx + (mProperties.mColumnNameIdx + 1);
            mData.erase(mData.begin() + rowIdx);
        }

        void RemoveRow(const std::string &pRowName) {
            int rowIdx = GetRowIdx(pRowName);
            if (rowIdx < 0) {
                throw std::out_of_range("row not found: " + pRowName);
            }

            RemoveRow(static_cast<std::size_t>(rowIdx));
        }

        size_t GetRowCount() const {
            return mData.size() - (mProperties.mColumnNameIdx + 1);
        }

        // Cell Methods -------------------------------------------------------
        template<typename T>
        T GetCell(const size_t pColumnIdx, const size_t pRowIdx) const {
            const std::size_t columnIdx = pColumnIdx + (mProperties.mRowNameIdx + 1);
            const std::size_t rowIdx = pRowIdx + (mProperties.mColumnNameIdx + 1);

            T val = convert::convert_to_val<T>(mData.at(rowIdx).at(columnIdx));
            return val;
        }

        template<typename T>
        T GetCell(const std::string &pColumnName, const std::string &pRowName) const {
            const int columnIdx = GetColumnIdx(pColumnName);
            if (columnIdx < 0) {
                throw std::out_of_range("column not found: " + pColumnName);
            }

            const int rowIdx = GetRowIdx(pRowName);
            if (rowIdx < 0) {
                throw std::out_of_range("row not found: " + pRowName);
            }

            return GetCell<T>(static_cast<std::size_t>(columnIdx), static_cast<std::size_t>(rowIdx));
        }

        template<typename T>
        void SetCell(const size_t pColumnIdx, const size_t pRowIdx, const T &pCell) {
            const size_t columnIdx = pColumnIdx + (mProperties.mRowNameIdx + 1);
            const size_t rowIdx = pRowIdx + (mProperties.mColumnNameIdx + 1);

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
        void SetCell(const std::string &pColumnName, const std::string &pRowName, const T &pCell) {
            const int columnIdx = GetColumnIdx(pColumnName);
            if (columnIdx < 0) {
                throw std::out_of_range("column not found: " + pColumnName);
            }

            const int rowIdx = GetRowIdx(pRowName);
            if (rowIdx < 0) {
                throw std::out_of_range("row not found: " + pRowName);
            }

            SetCell<T>(static_cast<std::size_t>(columnIdx), static_cast<std::size_t>(rowIdx), pCell);
        }

        // Label Methods ------------------------------------------------------
        void SetColumnLabel(size_t pColumnIdx, const std::string &pColumnName) {
            const std::size_t columnIdx = pColumnIdx + (mProperties.mRowNameIdx + 1);
            mColumnNames[pColumnName] = columnIdx;
            if (mProperties.mColumnNameIdx >= 0) {
                mData.at(mProperties.mColumnNameIdx).at(columnIdx) = pColumnName;
            }
        }

        void SetRowLabel(size_t pRowIdx, const std::string &pRowName) {
            const std::size_t rowIdx = pRowIdx + (mProperties.mColumnNameIdx + 1);
            mRowNames[pRowName] = rowIdx;
            if (mProperties.mRowNameIdx >= 0) {
                mData.at(rowIdx).at(static_cast<std::size_t>(mProperties.mRowNameIdx)) = pRowName;
            }
        }

    private:
        void ReadCsv(const std::string &path) {
            std::ifstream file(path, std::ios::in | std::ios::binary);

            std::vector<std::string> row;
            std::string cell;
            bool quoted = false;
            int cr = 0;
            int lf = 0;

            for (std::string buffer(bufLength, '\0'); file.read(&buffer[0], buffer.length()).gcount() > 0; ) {
                if (file.gcount() < buffer.length()) {
                    buffer.resize(static_cast<std::size_t>(file.gcount()));
                }
                for (const char &byte: buffer) {
                    switch (byte) {
                        case '"':
                            if (cell.empty() || cell[0] == '"') {
                                quoted = !quoted;
                            }
                            cell = cell + byte;
                            break;

                        case ',':
                            if (!quoted) {
                                row.push_back(cell);
                                cell.clear();
                            } else {
                                cell = cell + byte;
                            }
                            break;

                        case '\r':
                            ++cr;
                            break;

                        case '\n':
                            ++lf;
                            row.push_back(cell);
                            cell.clear();
                            mData.push_back(row);
                            row.clear();
                            quoted = false; // disallow line breaks in quoted string, by auto-unquote at linebreak
                            break;

                        default:
                            cell = cell + byte;
                    }
                }
            }

            // Handle last line without linebreak
            if (!cell.empty() || !row.empty()) {
                row.push_back(cell);
                cell.clear();
                mData.push_back(row);
                row.clear();
            }

            // Assume CR/LF if at least half the linebreaks have CR
            mProperties.mHasCR = (cr > (lf / 2));

            // Set up column labels
            if (mProperties.mColumnNameIdx >= 0 && mData.size() > 0) {
                std::size_t i = 0;
                for (auto &columnName : mData[mProperties.mColumnNameIdx]) {
                    mColumnNames[columnName] = i++;
                }
            }

            // Set up row labels
            if (mProperties.mRowNameIdx >= 0 && mData.size() > mProperties.mColumnNameIdx + 1) {
                std::size_t i = 0;
                for (auto &dataRow : mData) {
                    mRowNames[dataRow[mProperties.mRowNameIdx]] = i++;
                }
            }
        }

        std::string read_csv(const std::string& path) {
            std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);

            std::streampos length = file.tellg();
            file.seekg(0, std::ios::beg);

            std::string buffer(length, '\0');
            file.read(&buffer[0], length);

            return buffer;
        }

        static std::string to_csv_row(const std::vector<std::string>& row) {
            if (!row.empty()) {
                std::ostringstream oss;
                std::copy(std::begin(row), std::end(row), std::ostream_iterator<std::string>(oss, ","));
                std::string joined = oss.str();
                return joined.erase(joined.rfind(','));
            }
            return {};
        }

        void write_csv() const {
            std::ofstream file(mProperties.mPath, std::ios::out | std::ios::binary);

            std::transform(std::begin(mData), std::end(mData),
                           std::ostream_iterator<std::string>(file, mProperties.mHasCR ? "\r\n" : "\n"),
                           Document::to_csv_row);
        }

        int GetColumnIdx(const std::string &pColumnName) const {
            if (mProperties.mColumnNameIdx >= 0) {
                if (mColumnNames.find(pColumnName) != mColumnNames.end()) {
                    return mColumnNames.at(pColumnName) - (mProperties.mRowNameIdx + 1);
                }
            }
            return -1;
        }

        int GetRowIdx(const std::string &pRowName) const {
            if (mProperties.mRowNameIdx >= 0) {
                if (mRowNames.find(pRowName) != mRowNames.end()) {
                    return mRowNames.at(pRowName) - (mProperties.mColumnNameIdx + 1);
                }
            }
            return -1;
        }

        std::size_t GetDataRowCount() const {
            return mData.size();
        }

        std::size_t GetDataColumnCount() const {
            return (mData.size() > 0) ? mData.at(0).size() : 0;
        }
    };

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