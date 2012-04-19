/** \file DetectorLibrary.cpp
 *  Some useful function for managing the list of channel identifiers
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <string>

#include "DetectorLibrary.hpp"
#include "RawEvent.h"

#include "param.h"

using namespace std;

DetectorLibrary::DetectorLibrary() : vector<Identifier>(), highLocation()
{
    cout << "Constructing detector library" << endl;
    // do nothing
}

DetectorLibrary::~DetectorLibrary()
{
    // do nothing
}

DetectorLibrary::const_reference DetectorLibrary::at(DetectorLibrary::size_type idx) const
{
    return vector<Identifier>::at(idx);
}

DetectorLibrary::const_reference DetectorLibrary::at(DetectorLibrary::size_type mod, DetectorLibrary::size_type ch) const
{
    return vector<Identifier>::at(GetIndex(mod,ch));
}

DetectorLibrary::reference DetectorLibrary::at(DetectorLibrary::size_type idx)
{
    return vector<Identifier>::at(idx);
}

DetectorLibrary::reference DetectorLibrary::at(DetectorLibrary::size_type mod, DetectorLibrary::size_type ch)
{
    return vector<Identifier>::at(GetIndex(mod,ch));
}


void DetectorLibrary::push_back(const Identifier &x)
{
    string key;

    key = x.GetType() + ':' + x.GetSubtype();
    highLocation[key] = max(highLocation[key], x.GetLocation());

    vector<Identifier>::push_back(x);
}

/**
 * return the next undefined location for a particular type and subtype
 */
int DetectorLibrary::GetNextLocation(const string &type, 
				     const string &subtype) const
{
    string key;

    key = type + ':' + subtype;

    if (highLocation.count(key) > 0) {
	return highLocation.find(key)->second + 1;
    } else {
	return 0;
    }
}

DetectorLibrary::size_type DetectorLibrary::GetIndex(int mod, int chan) const
{
  return mod * pixie::numberOfChannels + chan;
}

bool DetectorLibrary::HasValue(int mod, int chan) const
{
    return (size() > GetIndex(mod,chan)) && (at(mod,chan).GetType() != ""); 
}

void DetectorLibrary::Set(int index, const Identifier& value)
{
    int module  = index / pixie::numberOfChannels;

    if (module >= numModules ) {
	numModules = module + 1;
	resize(numModules * pixie::numberOfChannels);
	if (!value.HasTag("virtual")) {
	    numPhysicalModules = module + 1;
	}
    }

    string key;

    key = value.GetType() + ':' + value.GetSubtype();
    highLocation[key] = max(highLocation[key], value.GetLocation());

    usedTypes.insert(value.GetType());
    usedSubtypes.insert(value.GetSubtype());

    at(index) = value;

    // temporary, but good diagnostic for now
    cout << setw(4) << index << "  ";
    value.Print();
    //
}

void DetectorLibrary::Set(int mod, int ch, const Identifier &value)
{
    Set(GetIndex(mod,ch), value);
}

/**
 *  Dump the map
 */
void DetectorLibrary::PrintMap(void) const
{
    cout << setw(4) << "MOD" 
	 << setw(4) << "CH";
    Identifier::PrintHeaders();

    for (size_t i=0; i < size(); i++) {
	cout << setw(4) << (int)(i / pixie::numberOfChannels)
	     << setw(4) << (int)(i % pixie::numberOfChannels);
	at(i).Print();
    }
}

/**
 * Print the list of detectors used and initialize the global raw event
 */
void DetectorLibrary::PrintUsedDetectors(void) const
{
    // Print the number of detectors and detector subtypes used in the analysis
    cout << usedTypes.size() <<" detector types are used in this analysis " 
	 << "and are named:" << endl << "  "; 
    copy(usedTypes.begin(), usedTypes.end(), ostream_iterator<string>(cout, " "));
    cout << endl;
    
    cout << usedSubtypes.size() <<" detector subtypes are used in this "
	 << "analysis and are named:" << endl << "  ";
    copy(usedSubtypes.begin(), usedSubtypes.end(), ostream_iterator<string>(cout," "));
    cout << endl;   

    extern RawEvent rawev;

    rawev.Init(usedTypes, usedSubtypes);
}
