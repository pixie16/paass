/** \file ParseXml.cpp 
 * \brief A class to read xml files using pugixml.
 * \author S.V. Paulauskas
 * \date 17 November 2012
 */
#include <iostream>
#include <sstream>

#include <cstdlib>

#include "ParseXml.hpp"

using namespace std;

ParseXml::ParseXml(const string &file) {
    if(!doc.load_file(file.c_str())) {
        cout << "I couldn't open the file " << file 
             << ". Find it!!" << endl;
        exit(1);
    }
}

ParseXml::ParseXml(const string &file, const string &node) {
    if(!doc.load_file(file.c_str())) {
        cout << "I couldn't open the file " << file 
             << ". Find it!!" << endl;
        exit(1);
    }
    node_ = node;
}

bool ParseXml::GetBool(const string &path) {
    if(GetInt(path) == 0)
        return(false);
    else
        return(true);
}

double ParseXml::GetDouble(const string &path) {
    return(atof(ReadXml(path).c_str()));
}

double ParseXml::GetPixieClock(const string &type) {
    stringstream ss;
#ifdef REVF
    ss << "pixie/revf/";
#else
    ss << "pixie/revd/";
#endif
    if(type != "clock")
        ss << type << "ClockInSeconds";
    else
        ss << type << "InSeconds";
    return(GetDouble(ss.str()));
}

int ParseXml::GetInt(const string &path) {
    return(atoi(ReadXml(path).c_str()));
}

size_t ParseXml::GetSizeT(const string &path) {
    return((size_t)GetInt(path));
}

string ParseXml::GetString(const string &path) {
    return(ReadXml(path));
}

word_t ParseXml::GetWordT(const string &path) {
    return((word_t)GetInt(path));
}    

string ParseXml::ReadXml(const string &path) {
    stringstream ss;
    if(node_ == "")
        ss << path;
    else
        ss << node_ << path;
    return(doc.first_element_by_path(ss.str().c_str()).child_value());
}

