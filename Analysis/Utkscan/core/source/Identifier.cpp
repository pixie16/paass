/** \file Identifier.hpp
 * \brief Defines identifying information for channels
*/
#include <iomanip>
#include <iostream>

#include "Identifier.hpp"

using namespace std;

void Identifier::Zero() {
    dammID_ = 9999;
    location_ = 9999;
    type_ = "";
    subtype_ = "";
    tags_.clear();
}

void Identifier::PrintHeaders(void) {
    cout << setw(10) << "Type" << setw(10) << "Subtype" << setw(4) << "Loc" << setw(6) << "DammID" << "    TAGS" << endl;
}

void Identifier::Print(void) const {
    cout << setw(10) << type_ << setw(10) << subtype_ << setw(4) << location_ << setw(6) << dammID_ << "    ";
    for (set<string>::const_iterator it = tags_.begin(); it != tags_.end(); it++) {
        if (it != tags_.begin())
            cout << ", ";
        cout << *it;
    }
    cout << endl;
}
