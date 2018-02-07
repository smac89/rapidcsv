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

    namespace detail {
        template<typename P>
        class CSVProperty {
            friend class rapidcsv::Properties;
        protected:
            P value;
        public:
            CSVProperty(const P &_value) : value(_value) {}
        };

        struct CSVRowSep : public CSVProperty<RowSepType> {
            using CSVProperty::CSVProperty;
        };

        struct CSVHasRowLabel : public CSVProperty<bool> {
            using CSVProperty::CSVProperty;
        };

        struct CSVHasHeader : public CSVProperty<bool> {
            using CSVProperty::CSVProperty;
        };

        struct CSVFieldSep : public CSVProperty<char> {
            using CSVProperty::CSVProperty;
        };

        struct CSVQuote : public CSVProperty<char> {
            using CSVProperty::CSVProperty;
        };

        struct CSVQuoteEscape : public CSVQuote {
            using CSVQuote::CSVQuote;
        };
    }

    class Properties {
        friend class PropertiesBuilder;

        using detail::CSVQuote;
        using detail::CSVRowSep;
        using detail::CSVFieldSep;
        using detail::CSVHasRowLabel;
        using detail::CSVHasHeader;

    public:
        Properties() : Properties("", RowSepType::LF, '"', ',', false, false) {}
        Properties &operator=(Properties &&) = default;
        Properties &operator=(const Properties &) = default;

        std::string filePath() const {
            return _filePath;
        }

        char quote() const {
            return _quote.value;
        }

        char fieldSep() const {
            return _fieldSep.value;
        }

        bool hasHeader() const {
            return _hasHeader.value;
        }

        bool hasRowLabel() const {
            return _hasRowLabel.value;
        }

        RowSepType rowSep() const {
            return _rowSep.value;
        }

    private:

        explicit Properties(std::string &&pPath, CSVRowSep &&rowSep, CSVQuote &&quote,
                            CSVFieldSep &&fieldSep, CSVHasHeader &&hasHeader, CSVHasRowLabel &&hasRowLabel) :
                _filePath(pPath), _quote(quote), _fieldSep(fieldSep),
                _hasHeader(hasHeader), _hasRowLabel(hasRowLabel), _rowSep(rowSep) {}

        const std::string _filePath;
        const detail::CSVQuote _quote;
        const detail::CSVFieldSep _fieldSep;
        const detail::CSVHasHeader _hasHeader;
        const detail::CSVHasRowLabel _hasRowLabel;
        const detail::CSVRowSep _rowSep;
    };

    class PropertiesBuilder {
        using detail::CSVQuote;
        using detail::CSVRowSep;
        using detail::CSVFieldSep;
        using detail::CSVHasRowLabel;
        using detail::CSVHasHeader;

        Properties properties;

    public:
        PropertiesBuilder() : PropertiesBuilder(Properties()) {}

        explicit PropertiesBuilder(Properties &&properties) : properties(std::move(properties)) {}

        explicit PropertiesBuilder(std::string filePath, RowSepType rowSep, char quote,
                                   char fieldSep, bool hasHeader, bool hasRowLabel) :
                properties(std::move(filePath), rowSep, quote, fieldSep, hasHeader, hasRowLabel) {}

        PropertiesBuilder& operator = (const Properties &properties) {
            this->properties = properties;
            return *this;
        }

        PropertiesBuilder &rowSep(RowSepType rowSep) {
            this->properties = Properties(std::move(properties).filePath(),
                                          std::move(rowSep),
                                          std::move(properties).quote(),
                                          std::move(properties).fieldSep(),
                                          std::move(properties).hasHeader(),
                                          std::move(properties).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &quote(char quote) {
            this->properties = Properties(std::move(properties).filePath(),
                                          std::move(properties).rowSep(),
                                          std::move(quote),
                                          std::move(properties).fieldSep(),
                                          std::move(properties).hasHeader(),
                                          std::move(properties).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &fieldSep(char fieldSep) {
            this->properties = Properties(std::move(properties).filePath(),
                                          std::move(properties).rowSep(),
                                          std::move(properties).quote(),
                                          std::move(fieldSep),
                                          std::move(properties).hasHeader(),
                                          std::move(properties).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &hasHeader() {
            this->properties = Properties(std::move(properties).filePath(),
                                          std::move(properties).rowSep(),
                                          std::move(properties).quote(),
                                          std::move(properties).fieldSep(),
                                          true,
                                          std::move(properties).hasRowLabel());
            return *this;
        }

        PropertiesBuilder &hasRowLabel() {
            this->properties = Properties(std::move(properties).filePath(),
                                          std::move(properties).rowSep(),
                                          std::move(properties).quote(),
                                          std::move(properties).fieldSep(),
                                          std::move(properties).hasHeader(),
                                          true);
            return *this;
        }

        PropertiesBuilder &filePath(std::string filePath) {
            this->properties = Properties(std::move(filePath),
                                          std::move(properties).rowSep(),
                                          std::move(properties).quote(),
                                          std::move(properties).fieldSep(),
                                          std::move(properties).hasHeader(),
                                          std::move(properties).hasRowLabel());
            return *this;
        }

        Properties build() const {
            return properties;
        }

        operator Properties() const { return build(); }

    private:
        explicit PropertiesBuilder(std::string &&pPath, RowSepType &&rowSep, char &&quote,
                                   char &&fieldSep, bool &&hasHeader, bool &&hasRowLabel) :
                properties(std::forward<std::string>(pPath), rowSep, quote, fieldSep, hasHeader, hasRowLabel) {}
    };

    namespace operators {
        constexpr const char* to_string(RowSepType rowSepType) {
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
