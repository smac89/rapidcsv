#ifndef RAPIDCSV_CSV_CONSTANTS_HPP
#define RAPIDCSV_CSV_CONSTANTS_HPP

namespace rapidcsv {
    static constexpr char CR = '\r';
    static constexpr char LF = '\n';
    static constexpr const char CRLF[3] = "\r\n";

    static constexpr auto bufLength = 64 * 1024;
}

#endif //RAPIDCSV_CSV_CONSTANTS_HPP
