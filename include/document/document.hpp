#ifndef RAPIDCSV_DOCUMENT_HPP
#define RAPIDCSV_DOCUMENT_HPP

#include <cstddef> // std::size_t
#include <vector>
#include <string>
#include <unordered_map>
#include "document/properties.hpp"

namespace rapidcsv {

    namespace doc {
        class Document {
            friend Document rapidcsv::load(const std::string&);
            friend Document rapidcsv::load(const Properties&);
            friend void rapidcsv::save(const Document&);
            friend void rapidcsv::save(const doc::Document&, const std::string&);

        protected:
            Properties documentProperties;

        public:

            //////////////////////////////////////////////////////////
            /////////////////////// COLUMNS //////////////////////////
            //////////////////////////////////////////////////////////

            //GET
            template<typename T>
            virtual std::vector<T> GetColumn(const size_t columnIndex, const T& fillValue) const = 0;

            template<typename T>
            virtual std::vector<T> GetColumn(const std::string &columnName, const T& fillValue) const = 0;

            template<typename T>
            virtual std::vector<T> GetColumn(const size_t columnIndex) const = 0;

            template<typename T>
            virtual std::vector<T> GetColumn(const std::string &columnName) const = 0;

            // SET
            template<typename T>
            virtual std::size_t SetColumn(const size_t columnIndex, const std::vector<T>& colData) {
                std::vector<std::string> converted(colData.size());
                std::transform(std::begin(colData), std::end(colData),
                               std::make_move_iterator(std::begin(converted)),
                               &rapidcsv::convert::convert_to_string);
                return SetColumn(columnIndex, std::move(converted));
            }

            template<typename T>
            virtual std::size_t SetColumn(const std::string &columnName, const std::vector<T>& colData) {
                std::vector<std::string> converted(colData.size());
                std::transform(std::begin(colData), std::end(colData),
                               std::make_move_iterator(std::begin(converted)),
                               &rapidcsv::convert::convert_to_string);
                return SetColumn(columnName, std::move(converted));
            }

            template<>
            virtual std::size_t SetColumn(const size_t columnIndex, const std::vector<std::string>& colData);

            template<>
            virtual std::size_t SetColumn(const std::string &columnName, const std::vector<std::string>& colData);

            template<>
            virtual std::size_t SetColumn(const size_t columnIndex, std::vector<std::string>&& colData);

            template<>
            virtual std::size_t SetColumn(const std::string &columnName, std::vector<std::string>&& colData);

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
            virtual T GetCell(const size_t columnIndex, const size_t rowIndex) const {
                return rapidcsv::convert::convert_to_val(GetCell(columnIndex, rowIndex));
            }

            template<typename T>
            virtual T GetCell(const std::string &columnName, const std::string &rowName) const {
                return rapidcsv::convert::convert_to_val(GetCell(columnName, rowName));
            }

            template<>
            virtual std::string GetCell(const std::size_t columnIndex, const std::size_t rowIndex) const = 0;

            template<>
            virtual std::string GetCell(const std::string &columnName, const std::string &rowName) const = 0;

            // SET
            template<>
            virtual void SetCell(const std::size_t columnIndex, const std::size_t rowIndex, const std::string&) = 0;

            template<>
            virtual void SetCell(const std::string &columnName, const std::string &rowName, const std::string&) = 0;

            template<typename T>
            virtual void SetCell(const std::size_t columnIndex, const std::size_t rowIndex, const T& tVal) {
                SetCell(columnIndex, rowIndex, rapidcsv::convert::convert_to_string(tVal));
            }

            template<typename T>
            virtual void SetCell(const std::string &columnName, const std::string &rowName, const T& tVal) {
                SetCell(columnName, rowName, rapidcsv::convert::convert_to_string(tVal));
            }

            // REMOVE
            virtual std::string RemoveCell(const std::size_t columnIndex, const size_t rowIndex) = 0;
            virtual std::string RemoveCell(const std::string &columnName, const std::string &rowName) = 0;

            //////////////////////////////////////////////////////////
            //////////////////////// LABELS //////////////////////////
            //////////////////////////////////////////////////////////

            // SET
            virtual void SetColumnLabel(std::size_t columnIndex, const std::string &columnLabel) = 0;
            virtual void SetColumnLabel(const std::string &columnLabel, const std::string &newColumnLabel) = 0;

            // GET
            virtual std::string GetColumnLabel(std::size_t columnIndex) = 0;

            //////////////////////////////////////////////////////////
            //////////////////////// SIZING //////////////////////////
            //////////////////////////////////////////////////////////
            virtual std::size_t rowCount(const std::size_t columnIndex) const = 0;
            virtual std::size_t rowCount(const std::string& columnName) const = 0;

            virtual std::size_t maxRowCount() const = 0;
            virtual std::size_t columnCount() const = 0;

            virtual ~Document() {}

        protected:
            //////////////////////////////////////////////////////////
            ///////////////////////// INDEX //////////////////////////
            //////////////////////////////////////////////////////////

//            virtual std::size_t getColumnIndex(const std::string &columnName) const = 0;
//            virtual std::size_t getColumnIndex(const std::size_t columnIndex) const = 0;
//            virtual std::size_t getRowIndex(const std::string &rowName) const = 0;
//            virtual std::size_t getRowIndex(const std::size_t rowIndex) const = 0;
        };
    }

    void save(const doc::Document& document);
    doc::Document load(const Properties &properties);

    doc::Document load(const std::string& path) {
        return load(PropertiesBuilder().filePath(path));
    }

    void save(const doc::Document& document, const std::string& path) {
        PropertiesBuilder builder = document.documentProperties;
        document.documentProperties = builder.filePath(path);
        save(document);
    }
}

#endif //RAPIDCSV_DOCUMENT_HPP
