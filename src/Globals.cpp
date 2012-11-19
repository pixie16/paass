/** \file Globals.cpp 
 * \brief A class to read the Globals.xml, Uses pugixml.
 * \author S.V. Paulauskas
 * \date 17 November 2012
 */
#include <iostream>
#include <sstream>

#include <cstdlib>

#include "Globals.hpp"

using namespace std;

Globals::Globals() {
    if(!doc.load_file("Globals.xml")) {
        cout << "I couldn't open the file Globals.xml. Find it!!" << endl;
        exit(1);
    }
}

Globals::Globals(const string &node) {
    if(!doc.load_file("Globals.xml")) {
        cout << "I couldn't open the file Globals.xml. Find it!!" << endl;
        exit(1);
    }

    node_ = node;
}

bool Globals::GetBool(const string &path) {
    if(GetInt(path) == 0)
        return(false);
    else
        return(true);
}

double Globals::GetDouble(const string &path) {
    return(atof(ReadXml(path).c_str()));
}

int Globals::GetInt(const string &path) {
    return(atoi(ReadXml(path).c_str()));
}

size_t Globals::GetSizeT(const string &path) {
    return((size_t)GetInt(path));
}

string Globals::GetString(const string &path) {
    return(ReadXml(path));
}

Globals::word_t Globals::GetWordT(const string &path) {
    return((Globals::word_t)GetInt(path));
}    

string Globals::ReadXml(const string &path) {
    stringstream ss; 
    if(node_ == "")
        ss << "/Globals/" << path;
    else
        ss << "/Globals/" << node_ << path;
    return(doc.first_element_by_path(ss.str().c_str()).child_value());
}

