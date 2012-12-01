#include "ChanIdentifier.hpp"
#include <string>
#include <iomanip>
#include <iostream>

using namespace std;

/** 
 * Return the value of a tag 
 */
Identifier::TagValue Identifier::GetTag(const string &s) const
{
    map<string, TagValue>::const_iterator it = tag.find(s);
    
    if (it == tag.end()) {
        return TagValue();
    }
    return it->second;
}

/**
 * Identifier constructor
 *
 * The dammid and detector location variable are set to -1
 * and the detector type and sub type are both set to ""
 * when an identifier object is created.
 */
Identifier::Identifier(){
    Zero();
}

/**
 * Identifier zero
 *
 * The dammid and detector location variable are reset to -1
 * and the detector type and sub type are both reset to ""
 * when an identifier object is zeroed.
 */
void Identifier::Zero()
{
    dammID   = -1;
    location = -1;
    type     = "";
    subtype  = "";

    tag.clear();
}

/**
 * Print column headings to aid with print 
 */
void Identifier::PrintHeaders(void) 
{
    cout << setw(10) << "Type"
	 << setw(10) << "Subtype"
	 << setw(4)  << "Loc"
	 << setw(6)  << "DammID"
	 << "    TAGS" << endl;	 
}

/**
 * Print the info containing in the identifier with trailing newline
 */
void Identifier::Print(void) const
{
    cout << setw(10) << type
	 << setw(10) << subtype
	 << setw(4)  << location
	 << setw(6)  << dammID
	 << "    ";
    for (map<string, TagValue>::const_iterator it = tag.begin();
	 it != tag.end(); it++) {
	if (it != tag.begin())
	    cout << ", ";
	cout << it->first << "=" << it->second;
    }
    cout << endl;
}

