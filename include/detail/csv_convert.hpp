#ifndef RAPIDCSV_CSV_CONVERT_HPP
#define RAPIDCSV_CSV_CONVERT_HPP

#include <string>
#include <sstream>

namespace rapidcsv {
    //////////////////////////////////////////////////////////
    ////////////////////// CONVERTERS ////////////////////////
    //////////////////////////////////////////////////////////

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

#endif //RAPIDCSV_CSV_CONVERT_HPP
