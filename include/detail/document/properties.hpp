#ifndef RAPIDCSV_PROPERTIES_HPP
#define RAPIDCSV_PROPERTIES_HPP

#include <array>
#include <utility>
#include <string>
#include <functional>

namespace rapidcsv {

    enum class RowSepType {
        CRLF, CR, LF
    };

    class Properties {
        friend class PropertiesBuilder;

    public:
        Properties() : Properties("", RowSepType::LF, '"', ',', false, false) {}
        Properties &operator=(Properties &&) = default;
        Properties &operator=(const Properties &) = default;
        Properties(Properties &&) = default;
        Properties(const Properties &) = default;

        std::string filePath() const {
            return _filePath;
        }

        char quote() const {
            return _quote;
        }

        char fieldSep() const {
            return _fieldSep;
        }

        bool hasHeader() const {
            return _hasHeader;
        }

        bool hasRowLabel() const {
            return _hasRowLabel;
        }

        RowSepType rowSep() const {
            return _rowSep;
        }

    private:

        explicit Properties(std::string &&pPath, RowSepType rowSep, char quote,
                            char fieldSep, bool hasHeader, bool hasRowLabel) :
                _filePath(pPath), _quote(quote), _fieldSep(fieldSep),
                _hasHeader(hasHeader), _hasRowLabel(hasRowLabel), _rowSep(rowSep) {}

        std::string _filePath;
        char _quote;
        char _fieldSep;
        bool _hasHeader;
        bool _hasRowLabel;
        RowSepType _rowSep;
    };

    class PropertiesBuilder {
        Properties prop;

    public:
        PropertiesBuilder() : PropertiesBuilder(Properties()) {}

        explicit PropertiesBuilder(Properties &&properties) : prop(std::move(properties)) {}

        explicit PropertiesBuilder(std::string filePath, RowSepType rowSep, char quote,
                                   char fieldSep, bool hasHeader, bool hasRowLabel) :
                prop(std::move(filePath), rowSep, quote, fieldSep, hasHeader, hasRowLabel) {}

        PropertiesBuilder& operator = (const Properties &properties) {
            this->prop = std::move(properties);
            return *this;
        }

        PropertiesBuilder &rowSep(RowSepType rowSep) {
            this->prop = Properties(std::move(prop).filePath(),
                                          std::move(rowSep),
                                          std::move(prop).quote(),
                                          std::move(prop).fieldSep(),
                                          std::move(prop).hasHeader(),
                                          std::move(prop).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &quote(char quote) {
            this->prop = Properties(std::move(prop).filePath(),
                                          std::move(prop).rowSep(),
                                          std::move(quote),
                                          std::move(prop).fieldSep(),
                                          std::move(prop).hasHeader(),
                                          std::move(prop).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &fieldSep(char fieldSep) {
            this->prop = Properties(std::move(prop).filePath(),
                                          std::move(prop).rowSep(),
                                          std::move(prop).quote(),
                                          std::move(fieldSep),
                                          std::move(prop).hasHeader(),
                                          std::move(prop).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &hasHeader() {
            this->prop = Properties(std::move(prop).filePath(),
                                          std::move(prop).rowSep(),
                                          std::move(prop).quote(),
                                          std::move(prop).fieldSep(),
                                          true,
                                          std::move(prop).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &hasRowLabel() {
            this->prop = Properties(std::move(prop).filePath(),
                                          std::move(prop).rowSep(),
                                          std::move(prop).quote(),
                                          std::move(prop).fieldSep(),
                                          std::move(prop).hasHeader(),
                                          true);
            return *this;
        }

        PropertiesBuilder &filePath(std::string filePath) {
            this->prop = Properties(std::move(filePath),
                                          std::move(prop).rowSep(),
                                          std::move(prop).quote(),
                                          std::move(prop).fieldSep(),
                                          std::move(prop).hasHeader(),
                                          std::move(prop).hasRowLabel());
            return *this;
        }

        Properties build() const {
            return prop;
        }

        operator Properties() const { return build(); }

    private:
        explicit PropertiesBuilder(std::string &&pPath, RowSepType &&rowSep, char &&quote,
                                   char &&fieldSep, bool &&hasHeader, bool &&hasRowLabel) :
                prop(std::forward<std::string>(pPath), rowSep, quote, fieldSep, hasHeader, hasRowLabel) {}
    };

    namespace operators {
        const char* to_string(RowSepType rowSepType) {
            switch (rowSepType) {
                case RowSepType::CRLF:
                    return "\r\n";
                case RowSepType::CR:
                    return "\r";
                default:
                    return "\n";
            }
        }
    }
}

#endif //RAPIDCSV_PROPERTIES_HPP
