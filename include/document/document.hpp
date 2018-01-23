#ifndef RAPIDCSV_DOCUMENT_HPP
#define RAPIDCSV_DOCUMENT_HPP

#include <cstddef> // std::size_t
#include <vector>
#include <string>
#include <unordered_map>
#include "document/properties.hpp"

namespace rapidcsv {

    class Document {
    public:

        explicit Document(const std::string& filePath): Document(PropertiesBuilder().filePath(filePath)) { }
        explicit Document(Properties &&properties):
                documentProperties(properties), heatMap(32, std::vector<bool>(64, false)) {}
        Document(const Document &pDocument) = default;
        Document(Document &&document) = default;

        virtual ~Document() {}

        virtual void load() {
            load(documentProperties.filePath());
        }

        virtual void save() {
            save(documentProperties.filePath());
        }

        virtual void load(const std::string& pPath) = 0;
        virtual void save(const std::string &pPath) const  = 0;

        virtual std::size_t rowCount() const = 0;
        virtual std::size_t columnCount() const = 0;

    protected:
        Properties documentProperties;
        std::vector<std::vector<bool>> heatMap;
    };
}

#endif //RAPIDCSV_DOCUMENT_HPP
