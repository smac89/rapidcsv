#ifndef RAPIDCSV_CSV_EXCEPT_HPP
#define RAPIDCSV_CSV_EXCEPT_HPP

#include <exception>

namespace rapidcsv {
    namespace except {
        struct csv_quote_inside_non_quote_field_exception: public std::exception {
            virtual const char* what() const noexcept {
                return "Quotes are only allowed inside quoted fields";
            }
        };

        struct csv_unescaped_quote_exception: public std::exception {
            virtual const char* what() const noexcept {
                return "Quotes appearing inside a field, are to be escaped with another quote";
            }
        };

        struct csv_unterminated_quote_exception: public std::exception {
            virtual const char* what() const noexcept {
                return "Quoted field was not terminated by a closing quote";
            }
        };
    }

    namespace read {
        struct csv_nothing_to_read_exception: public std::exception {
            virtual const char* what() const noexcept {
                return "The reader has run out of bytes!";
            }
        };

        namespace iterator {
            struct past_the_end_iterator_exception: public std::runtime_error {
                past_the_end_iterator_exception():
                        std::runtime_error("Iterator has gone past the end") { }
            };

            struct empty_iterator_exception: public std::runtime_error {
                empty_iterator_exception():
                        std::runtime_error("Iterator is empty") { }
            };
        }
    }
}

#endif //RAPIDCSV_CSV_EXCEPT_HPP
