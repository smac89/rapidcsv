// test042.cpp - test GetColumnCount() and GetRowCount()

#include <rapidcsv.hpp>
#include "unittest.h"

int main() {
    int rv = 0;

    std::string csv =
            "-,A,B,C\n"
                    "1,3,9,81\n"
                    "2,4,16,256\n";

    std::string path = unittest::TempPath();
    unittest::WriteFile(path, csv);

    try {
        rapidcsv::Document doc1(path);
        unittest::ExpectEqual(size_t, doc1.GetColumnCount(), 3);
        unittest::ExpectEqual(size_t, doc1.GetRowCount(), 2);

        rapidcsv::Document doc2(rapidcsv::PropertiesBuilder().filePath(path));
        unittest::ExpectEqual(size_t, doc2.GetColumnCount(), 4);
        unittest::ExpectEqual(size_t, doc2.GetRowCount(), 3);
    }
    catch (const std::exception &ex) {
        std::cout << ex.what() << std::endl;
        rv = 1;
    }

    unittest::DeleteFile(path);

    return rv;
}
