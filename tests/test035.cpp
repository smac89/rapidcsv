// test035.cpp - custom conversion

#include <iomanip>
#include <math.h>
#include <rapidcsv.hpp>
#include "unittest.h"

// Data requested as ints to be converted to fixed-point two decimal numbers
namespace rapidcsv::convert {
    template<>
    inline int convert_to_val(const std::string &pStr) {
        return static_cast<int>(roundf(100.0f * std::stof(pStr)));
    }

    template<>
    std::string convert_to_string(const int &pVal) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << (static_cast<float>(pVal) / 100.0f);
        return out.str();
    }
}

int main() {
    int rv = 0;

    std::string csv =
            "1,10,100,1000\n"
                    "0.1,0.01,0.001,0.006\n";

    std::string path = unittest::TempPath();
    unittest::WriteFile(path, csv);

    try {
        rapidcsv::Document doc(rapidcsv::PropertiesBuilder().filePath(path));

        unittest::ExpectEqual(int, doc.GetCell<int>(0, 0), 100);
        unittest::ExpectEqual(int, doc.GetCell<int>(1, 0), 1000);
        unittest::ExpectEqual(int, doc.GetCell<int>(2, 0), 10000);
        unittest::ExpectEqual(int, doc.GetCell<int>(3, 0), 100000);

        unittest::ExpectEqual(int, doc.GetCell<int>(0, 1), 10);
        unittest::ExpectEqual(int, doc.GetCell<int>(1, 1), 1);
        unittest::ExpectEqual(int, doc.GetCell<int>(2, 1), 0);
        unittest::ExpectEqual(int, doc.GetCell<int>(3, 1), 1);

        doc.SetCell<int>(0, 0, 12345);
        unittest::ExpectEqual(std::string, doc.GetCell<std::string>(0, 0), "123.45");
    }
    catch (const std::exception &ex) {
        std::cout << ex.what() << std::endl;
        rv = 1;
    }

    unittest::DeleteFile(path);

    return rv;
}

