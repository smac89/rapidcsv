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

#include "document/document.hpp"

namespace rapidcsv {
    using doc::Document;

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

//    void save(const Document& document);
//    void save(const Document& document, const std::string& path);
//
//    Document&& load(const Properties &properties);
//    Document&& load(const std::string& path);
}
