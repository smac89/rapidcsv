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
#include <exception>
#include <iterator>
#include <cstddef>
#include <utility>

static constexpr auto bufLength = 64 * 1024;

namespace rapidcsv {

    static constexpr auto CR = '\r';
    static constexpr auto LF = '\n';
    static constexpr auto CRLF = R"(\r\n)";

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

    namespace except {
        struct csv_quote_inside_non_quote_field_exception: public std::exception {
            virtual const char* what() const noexcept {
                return "Quotes are not allowed inside non-quoted fields";
            }
        };

        struct csv_unescaped_quote_exception: public std::exception {
            virtual const char* what() const noexcept {
                return "Quotes appearing inside a field, are to be escaped with another quote";
            }
        };

        struct csv_line_break_inside_non_quote_exception: public std::exception {
            virtual const char* what() const noexcept {
                return "Fields containing line breaks must be enclosed with quotes";
            }
        };
    }

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

    namespace parse {

        template <typename StreamIter>
        class CSVParser {
            using namespace except;
            friend class CSVParserIterator;

        protected:
            std::string current;
            CSVParserIterator iterator;
            StreamIter& _begin, &_end;

        private:
            bool _start_quoted_field;
            bool _end_quoted_field;
            bool _nested_quote;
            bool _is_quoted_field;

        public:
            CSVParser(StreamIter &begin, StreamIter &end): current('\0'), iterator(this), _begin(begin), _end(end) {
                _start_quoted_field = false;
                _end_quoted_field = true;
                _nested_quote = false;
                _is_quoted_field = false;
                parseNext();
            }

            CSVParserIterator* begin() {
                return &iterator;
            }

            CSVParserIterator* end() {
                return &iterator_empty;
            }
        private:
            bool parseNext() {
                if (_begin == _end) {
                    return false;
                }

                for (current = "";_begin != _end; ) {
                    char byte = *_begin++;
                    switch (byte) {
                        case '"':
                            if (current.empty()) {
                                _is_quoted_field = true;
                                _start_quoted_field = true;
                            } else if (!_is_quoted_field) {
                                throw csv_quote_inside_non_quote_field_exception();
                            } else if (!_end_quoted_field) {
                                _end_quoted_field = true;
                            } else {
                                _nested_quote = true;
                                _end_quoted_field = false;
                                continue;
                            }
                            current += byte;
                            break;

                        case ',':
                            if (!_is_quoted_field || _end_quoted_field) {
                                goto found_field_end;
                            }
                            current += byte;
                            break;

                        case CR:
                            if (_is_quoted_field && _end_quoted_field) {
                                throw csv_line_break_inside_non_quote_exception();
                            }
                            current += byte;
                            break;

                        case LF:
                            if (_is_quoted_field && _end_quoted_field) {
                                ++lf;
                                row.push_back(cell);
                                mData.push_back(row);
                                cell.erase();
                                row.clear();
                            } else {
                                cell += byte;
                            }
                            break;

                        default:
                            if (!quoted && cell.back() == '"') {
                                throw csv_unescaped_quote_exception();
                            }
                            cell += byte;
                    }
                }

                found_field_end: ;


                return true;
            }
        };

        class CSVParserIterator {
        public:
            CSVParserIterator(CSVParser* parser): _parser(parser) {}

            CSVParserIterator* operator++ () {
                if (!_parser->parseNext()) {
                    return &iterator_empty;
                }
                return this;
            }

            const std::string operator *() const { return _parser->current; }

        private:
            CSVParser* _parser;
        };

        class CSVParserIteratorEmpty: public CSVParserIterator {
        public:
            CSVParserIteratorEmpty(): CSVParserIterator(nullptr) {}
        } iterator_empty;
    }

    enum class CSVProperty {
        hasCR,
        hasRowLabel,
        hasColLabel
    };

    class Properties {
    public:
        std::string mPath;
        const int mColumnNameIdx;
        const int mRowNameIdx;
        const bool mHasCR;
        const bool mhasRowLabel;
        const bool mHasColLabel;

    public:
        template <typename... Prop>
        explicit Properties(const std::string &pPath = "",
                            const int pColumnNameIdx = 0,
                            const int pRowNameIdx = 0, Prop&&... csvProperties)
                : Properties(pPath, pColumnNameIdx, pRowNameIdx, { std::forward<Prop>(csvProperties)... }) {}

//        template <typename... Prop>
//        explicit Properties(const std::string &pPath = "", Prop&&... csvProperties)
//                : Properties(pPath, 0, 0, { std::forward<Prop>(csvProperties)... }) {}

    private:
        Properties(const std::string &pPath, const int pColumnNameIdx, const int pRowNameIdx,
                   const std::unordered_set<CSVProperty>& csv_properties)
                : mPath(pPath), mColumnNameIdx(pColumnNameIdx), mRowNameIdx(pRowNameIdx),
                  mHasCR(csv_properties.count(CSVProperty::hasCR) == 1),
                  mhasRowLabel(csv_properties.count(CSVProperty::hasRowLabel) == 1),
                  mHasColLabel(csv_properties.count(CSVProperty::hasColLabel) == 1) {}
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
            parse::CSVParser parser(std::istream_iterator<char>(std::cin), std::istream_iterator<char>{});
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
    void Load(const std::string& pPath) {
        mProperties.mPath = pPath;
        ReadCsv(mProperties.mPath);
    }

    void Save(const std::string &pPath = "") {
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
        using namespace except;

        std::ifstream file(path, std::ios::in | std::ios::binary);

        std::vector<std::string> row;
        std::string cell;
        bool quoted = false;
        int cr = 0;
        int lf = 0;

        for (const char& byte : std::move(read_csv(path))) {
            switch (byte) {
                case '"':
                    if (!quoted) {
                        if (cell.empty() || cell.front() == '"') {
                            quoted = true;
                        } else {
                            throw csv_quote_inside_non_quote_field_exception();
                        }
                    } else if (cell.front() != '"') {

                    } else if (cell.back() == '"') {
                        quoted = false;
                        break;
                    } else {
                        throw csv_unescaped_quote_exception();
                    }
                    cell += byte;
                    break;

                case ',':
                    if (!quoted) {
                        row.push_back(cell);
                        cell.erase();
                    } else {
                        cell += byte;
                    }
                    break;

                case CR:
                    if (!quoted) {
                        ++cr;
                    } else {
                        cell += byte;
                    }
                    break;

                case LF:
                    if (!quoted) {
                        ++lf;
                        row.push_back(cell);
                        mData.push_back(row);
                        cell.erase();
                        row.clear();
                    } else {
                        cell += byte;
                    }
                    break;

                default:
                    if (!quoted && cell.back() == '"') {
                        throw csv_unescaped_quote_exception();
                    }
                    cell += byte;
            }
        }

        for (std::string buffer(bufLength, '\0'); file.read(&buffer[0], bufLength -1).gcount() > 0; ) {
            std::string::size_type content_size = static_cast<std::string::size_type>(file.gcount());
            if (content_size < (bufLength - 1) || file.eof()) {
                buffer.resize(content_size + 1);
                if (buffer.find(LF, content_size - 1) == std::string::npos) {
                    buffer[content_size] = LF; // append line break to last line
                } else {
                    buffer.resize(content_size);
                }
            }
            if (cell.empty()) {
                quoted = true;
            } else if (!quoted && cell.back() == '"') {
                quoted = true;
                break;
            } else if (cell.front() == '"') {
                quoted = false;
            } else {

            }
            for (const char &byte: buffer) {
                switch (byte) {
                    case '"':
                        if (!quoted) {
                            if (cell.empty() || cell.front() == '"') {
                                quoted = true;
                            } else {
                                throw csv_quote_inside_non_quote_field_exception();
                            }
                        } else if (cell.front() != '"') {

                        } else if (cell.back() == '"') {
                            quoted = false;
                            break;
                        } else {
                            throw csv_unescaped_quote_exception();
                        }
                        cell += byte;
                        break;

                    case ',':
                        if (!quoted) {
                            row.push_back(cell);
                            cell.erase();
                        } else {
                            cell += byte;
                        }
                        break;

                    case CR:
                        if (!quoted) {
                            ++cr;
                        } else {
                            cell += byte;
                        }
                        break;

                    case LF:
                        if (!quoted) {
                            ++lf;
                            row.push_back(cell);
                            mData.push_back(row);
                            cell.erase();
                            row.clear();
                        } else {
                            cell += byte;
                        }
                        break;

                    default:
                        if (!quoted && cell.back() == '"') {
                            throw csv_unescaped_quote_exception();
                        }
                        cell += byte;
                }
            }
        }

        // Handle last line without linebreak
//            if (!cell.empty() || !row.empty()) {
//                row.push_back(cell);
//                cell.clear();
//                mData.push_back(row);
//                row.clear();
//            }

        // Assume CR/LF if at least half the linebreaks have CR
//            mProperties.mHasCR = (cr > (lf / 2));

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
