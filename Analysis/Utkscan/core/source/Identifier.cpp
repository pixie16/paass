/** \file Identifier.hpp
 * \brief Defines identifying information for channels
*/
#include <iomanip>
#include <iostream>
#include <limits>

#include "Identifier.hpp"

using namespace std;

int Identifier::GetTag(const std::string &s) const {
    map<string, int>::const_iterator it = tag_.find(s);
    if (it == tag_.end())
        return (std::numeric_limits<int>::max());
    return it->second;
}

void Identifier::Zero() {
    dammID_ = 9999;
    location_ = 9999;
    type_ = "";
    subtype_ = "";

    tag_.clear();
}

void Identifier::PrintHeaders(void) {
    cout << setw(10) << "Type"
         << setw(10) << "Subtype"
         << setw(4) << "Loc"
         << setw(6) << "DammID"
         << "    TAGS" << endl;
}

void Identifier::Print(void) const {
    cout << setw(10) << type_
         << setw(10) << subtype_
         << setw(4) << location_
         << setw(6) << dammID_
         << "    ";
    for (map<string, int>::const_iterator it = tag_.begin();
         it != tag_.end(); it++) {
        if (it != tag_.begin())
            cout << ", ";
        cout << it->first << "=" << it->second;
    }
    cout << endl;
}
