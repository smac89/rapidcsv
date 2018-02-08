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

#include <sstream>
#include <string>
#include <cstddef>

#include "csv_document.hpp"

namespace rapidcsv {
    using namespace doc;

    //////////////////////////////////////////////////////////
    /////////////////////// CONSTANTS ////////////////////////
    //////////////////////////////////////////////////////////

    static constexpr char CR = '\r';
    static constexpr char LF = '\n';
    static constexpr const char CRLF[3] = "\r\n";
    static constexpr auto bufLength = 64 * 1024;

    //////////////////////////////////////////////////////////
    //////////////////////// DOCUMENT ////////////////////////
    //////////////////////////////////////////////////////////

    void save(const Document& document);
    void save(const Document& document, const std::string& path);

    Document load(const Properties &properties);
    Document load(const std::string& path);

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
