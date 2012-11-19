/** \file Globals.hpp 
 * \brief A class to read the Globals.xml, Uses pugixml.
 * \author S.V. Paulauskas
 * \date 17 November 2012
 */
#ifndef __GLOBALS_HPP_
#define __GLOBALS_HPP_

#include <string>

#include <cstdlib>
#include <stdint.h>

#include "pugixml.hpp"

class Globals {
public:
    Globals();
    Globals(const std::string &node);
    ~Globals(){};

    /* Some of these would not work properly in the XML file -SVP */
    typedef uint32_t word_t; //< a pixie word
    typedef uint16_t halfword_t; //< a half pixie word
    typedef uint32_t bufword_t; //< word in a pixie buffer
    
    bool GetBool(const std::string &path);
    double GetDouble(const std::string &path);
    int GetInt(const std::string&path);
    size_t GetSizeT(const std::string &path);
    std::string GetString(const std::string &path);
    word_t GetWordT(const std::string &path);
private:
    std::string ReadXml(const std::string &path);
    
    pugi::xml_document doc;
    std::string node_;
};
#endif
