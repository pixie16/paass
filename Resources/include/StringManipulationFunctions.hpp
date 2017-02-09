///@file StringManipulationFunctions.hpp
///@brief A collection of functions to aid in string manipulations.
///@author S. V. Paulauskas
///@date February 09, 2017
#ifndef PAASS_STRINGMANIPULATIONFUNCTIONS_HPP
#define PAASS_STRINGMANIPULATIONFUNCTIONS_HPP
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace StringManipulation {
    ///Converts string to bool (True, true, 1 and False, false, 0) are
    /// accepted; throws an exception if not succesful. Notice tolower
    /// will work only with ascii, not with utf-8, but shouldn't be a
    /// problem for true and false words.
    ///@param [in] s : String to convert to bool
    ///@return A bool from the input string
    inline bool StringToBool(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "true" || s == "1")
            return true;
        else if (s == "false" || s == "0")
            return false;
        else {
            std::stringstream ss;
            ss << "strings::to_bool: Could not convert string '"
               << s << "' to bool" << std::endl;
            throw std::invalid_argument(ss.str());
        }
    }

    ///Tokenizes the string, splitting it on a given delimiter.
    ///delimiters are removed from returned vector of tokens.
    ///@param [in] str : The string to break up
    ///@param [in] delimiter : character to break up on
    ///@return The vector of tokens
    inline std::vector <std::string> TokenizeString(
            const std::string &str, const std::string &delimiter) {
        std::vector <std::string> tokenized;
        size_t current, next = -1;
        do {
            current = next + 1;
            next = str.find_first_of(delimiter, current);
            tokenized.push_back(str.substr( current, next - current ));
        } while (next != std::string::npos);
        return tokenized;
    }
}

#endif //PAASS_STRINGMANIPULATIONFUNCTIONS_HPP
