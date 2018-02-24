#ifndef RAPIDCSV_DOCUMENT_UTILS_HPP
#define RAPIDCSV_DOCUMENT_UTILS_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <unordered_map>
#include <iterator>
#include "detail/util/fp.hpp"

namespace rapidcsv {
    namespace doc {
        class DocumentBase {
            using RowType = std::unordered_map<std::size_t, std::string>;
            using Entry = RowType::value_type;

            static const std::string EMPTY = "";

        protected:
            inline std::size_t get_column_index(const std::size_t columnIndex) const {
                auto normalizedColumn = columnIndex + (documentProperties.hasRowLabel() ? 1 : 0);
                if (columnIndex >= column_count(0) || normalizedColumn >= column_count(0)) {
                    throw std::out_of_range(std::string("column out of range : ") + columnIndex);
                }
                return normalizedColumn;
            }

            inline std::size_t get_column_index(const std::string &columnName) const {
                auto columnIter = columnNames.find(columnName);
                if (columnIter == columnNames.end()) {
                    throw std::out_of_range("column not found: " + columnName);
                }
                return columnIter->second;
            }

            inline std::size_t get_row_index(const std::string &rowName) const {
                auto rowIter = rowNames.find(rowName);
                if (rowIter == std::end(rowNames)) {
                    throw std::out_of_range("Row label not found");
                }
                return rowIter->second;
            }

            inline std::size_t get_row_index(const std::size_t rowIndex) const {
                auto normalizedRow = rowIndex + (documentProperties.hasHeader() ? 1 : 0);
                if (rowIndex >= max_size() || normalizedRow >= max_size()) {
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

            std::vector<std::reference_wrapper<std::string>> const operator[] (const std::size_t& rowIndex) const {
                using rapidcsv::read::r_transform;
                using rapidcsv::read::sequence;

                RowType& row = documentMesh[rowIndex];
                auto &&reader = r_transform(sequence(static_cast<std::size_t>(0), max_column_count()), [](std::size_t const& index) {
                    auto found_it = row.find(index);
                    if (found_it != std::end(row)) {
                        return std::ref(found_it->second);
                    }
                    return std::ref(EMPTY);
                });

                std::vector<std::reference_wrapper<std::string>> row_content(max_column_count());
                std::copy(std::begin(reader), std::end(reader), std::back_inserter(row_content));

                return row_content;
            }

            rapidcsv::read::Reader<std::string> _get_row(const std::size_t rowIndex) const {
                using rapidcsv::read::sequence;
                using rapidcsv::read::r_copy_if;
                using Tup = std::tuple<std::size_t, std::size_t>;

                RowType & row = documentMesh[rowIndex];

                for (auto&& seq : sequence(r_copy_if(sequence(), [&row](const std::size_t index) {
                    return row.count(index) > 0;
                }))) {
                    data[std::get<0>(std::forward<Tup>(seq))] = row[std::get<1>(std::forward<Tup>(seq))];
                }

                return data;
            }

            // SET
            void set_column_label(std::size_t columnIndex, const std::string &columnLabel) {
                set_column_label(get_column_label(columnIndex), columnLabel);
            }

            void set_column_label(const std::string &columnLabel, const std::string &newColumnLabel) {
                // TODO
            }

            std::vector<RowType>& document() {
                return documentMesh;
            }

            Properties& properties() {
                return documentProperties;
            }

        public:
            // GET
            std::string get_column_label(std::size_t columnIndex) {
                auto normalizedColumnIndex = get_column_index(columnIndex);
                return documentMesh[0][normalizedColumnIndex];
            }

            std::string get_row_label(std::size_t rowIndex) const {
                // TODO
                return "";
            }

            std::size_t size() const {
                return documentMesh.size();
            }

            std::size_t max_size() const {
                // TODO
                return 0;
            }

            std::size_t column_count(const std::size_t row_index) const {
                return column_count(get_row_label(row_index));
            }

            std::size_t column_count(const std::string& row_name) const {
                // TODO
                return 0;
            }

            std::size_t max_column_count() const {
                // TODO
                return 0;
            }

        protected:
            Properties documentProperties;
            std::vector<RowType> documentMesh;
            std::unordered_map<std::string, std::size_t> columnNames;
            std::unordered_map<std::string, std::size_t> rowNames;
            std::size_t _rowCount = 0;
            std::size_t _columnCount = 0;
        };


    }
}

#endif // RAPIDCSV_DOCUMENT_UTILS_HPP
