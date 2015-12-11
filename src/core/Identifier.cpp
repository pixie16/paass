/** \file Identifier.hpp
 * \brief Defines identifying information for channels
*/
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "Identifier.hpp"

using namespace std;

int Identifier::GetTag(const std::string &s) const {
    map<string, int>::const_iterator it = tag.find(s);

    if (it == tag.end()) {
        return(std::numeric_limits<int>::max());
    }
    return it->second;
}

void Identifier::Zero() {
    dammID   = -1;
    location = -1;
    type     = "";
    subtype  = "";

    tag.clear();
}

void Identifier::PrintHeaders(void) {
    cout << setw(10) << "Type"
	 << setw(10) << "Subtype"
	 << setw(4)  << "Loc"
	 << setw(6)  << "DammID"
	 << "    TAGS" << endl;
}

void Identifier::Print(void) const {
    cout << setw(10) << type
	 << setw(10) << subtype
	 << setw(4)  << location
	 << setw(6)  << dammID
	 << "    ";
    for (map<string, int>::const_iterator it = tag.begin();
	 it != tag.end(); it++) {
	if (it != tag.begin())
	    cout << ", ";
	cout << it->first << "=" << it->second;
    }
    cout << endl;
}
