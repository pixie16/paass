/** \file ParseXml.hpp 
 * \brief A class to read xml files using pugixml.
 * \author S.V. Paulauskas
 * \date 17 November 2012
 */
#ifndef __PARSEXML_HPP_
#define __PARSEXML_HPP_

#include <string>

#include <cstdlib>
#include <stdint.h>

#include "pugixml.hpp"

/* Some of these would not work properly in the XML file -SVP */
typedef uint32_t word_t; //< a pixie word
typedef uint16_t halfword_t; //< a half pixie word
typedef uint32_t bufword_t; //< word in a pixie buffer

class ParseXml {
public:
    ParseXml() {};
    ParseXml(const std::string &file);
    ParseXml(const std::string &file, const std::string &node);
    ~ParseXml(){};

    bool GetBool(const std::string &path);
    double GetDouble(const std::string &path);
    double GetPixieClock(const std::string &type);
    int GetInt(const std::string&path);
    size_t GetSizeT(const std::string &path);
    std::string GetString(const std::string &path);
    word_t GetWordT(const std::string &path);

    void SetNode(const std::string &node){node_ = node;};
private:
    std::string ReadXml(const std::string &path);
    
    pugi::xml_document doc;
    std::string node_;
};
#endif
