/** \file DetectorLibrary.cpp
 *  \brief Some useful function for managing the list of channel identifiers
 *  \author David Miller
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <string>

#include "DetectorLibrary.hpp"
#include "RawEvent.hpp"

#include "Globals.hpp"

using namespace std;

set<int> DetectorLibrary::emptyLocations;

DetectorLibrary* DetectorLibrary::instance = NULL;

/** Instance is created upon first call */
DetectorLibrary* DetectorLibrary::get() {
    if (!instance) {
        instance = new DetectorLibrary();
    }
    return instance;
}

DetectorLibrary::DetectorLibrary() : vector<Identifier>(), locations()
{
    GetKnownDetectors();
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
    mapkey_t key = MakeKey(x.GetType(), x.GetSubtype());

    locations[key].insert(x.GetLocation());
    vector<Identifier>::push_back(x);
}

/**
 * return the list of locations for a particular identifier
 */
const set<int>& DetectorLibrary::GetLocations(const Identifier &id) const
{
    return GetLocations(id.GetType(), id.GetSubtype());
}

/**
 * return the list of locations for a particular type and subtype
 */
const set<int>& DetectorLibrary::GetLocations(const string &type, const string &subtype) const
{
    mapkey_t key = MakeKey(type, subtype);

    if (locations.count(key) > 0) {
	return locations.find(key)->second; 
    } else {
	return emptyLocations;
    }
}

/**
 * return the next undefined location for a particular identifer
 */
int DetectorLibrary::GetNextLocation(const Identifier &id) const
{
  return GetNextLocation(id.GetType(), id.GetSubtype());
}

/**
 * return the next undefined location for a particular type and subtype
 */
int DetectorLibrary::GetNextLocation(const string &type, 
				     const string &subtype) const
{
    mapkey_t key = MakeKey(type, subtype);

    if (locations.count(key) > 0) {
	return *(locations.find(key)->second.rbegin()) + 1;
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
    return HasValue(GetIndex(mod,chan));
}

bool DetectorLibrary::HasValue(int index) const
{
  return ((signed)size() > index && at(index).GetType() != "");
}

void DetectorLibrary::Set(int index, const Identifier& value)
{
		
    /// Search the list of known detectors; if the detector type 
    ///    is not matched, print out an error message and terminate
    if (knownDetectors.find(value.GetType()) == knownDetectors.end()) {
	cout << endl;
	cout << "The detector called '" << value.GetType() <<"'"<< endl
	     << "read in from the file 'map2.txt'" << endl
	     << "is unknown to this program!.  This is a" << endl
	     << "fatal error.  Program execution halted!" << endl
	     << "If you believe this detector should exist," << endl
	     << "please edit the 'getKnownDetectors'" << endl
	     << "function inside the 'DetectorLibrary.cpp' file" << endl
	     << endl;
	cout << "The currently known detectors include:" << endl;
	copy(knownDetectors.begin(), knownDetectors.end(), ostream_iterator<string>(cout, " "));
	cout << endl;
	exit(EXIT_FAILURE);
    }

    unsigned int module  = ModuleFromIndex(index);
    if (module >= numModules ) {
	numModules = module + 1;
	resize(numModules * pixie::numberOfChannels);
	if (!value.HasTag("virtual")) {
	    numPhysicalModules = module + 1;
	}
    }
    
    string key;
    key = value.GetType() + ':' + value.GetSubtype();
    locations[key].insert(value.GetLocation());
    
    usedTypes.insert(value.GetType());
    usedSubtypes.insert(value.GetSubtype());

    at(index) = value;
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
	cout << setw(4) << ModuleFromIndex(i)
	     << setw(4) << ChannelFromIndex(i);
	at(i).Print();
    }
}

/**
 * Print the list of detectors used and initialize the global raw event
 */
void DetectorLibrary::PrintUsedDetectors(RawEvent& rawev) const
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

    rawev.Init(usedTypes);
}

/*!
  Retrieves a vector containing all detector types for which an analysis
  routine has been defined making it possible to declare this detector type
  in the map.txt file.  The currently known detector types are in detectorStrings
*/
const set<string>& DetectorLibrary::GetKnownDetectors(void)
{
    const unsigned int detTypes = 20;
    const string detectorStrings[detTypes] = {
	"dssd_front", "dssd_back", "idssd_front", "position", "timeclass",
	"ge", "si", "scint", "mcp", "mtc", "generic", "ssd", "vandleSmall",
	"vandleBig", "tvandle","pulser", "logic", "ion_chamber", "ignore", 
	"valid"};
  
    // only call this once
    if (!knownDetectors.empty())
	return knownDetectors;

    // this is a list of the detectors that are known to this program.
    cout << "DetectorLibrary: constructing the list of known detectors "
         << endl;

    //? get these from event processors
    for (unsigned int i=0; i < detTypes; i++)
	knownDetectors.insert(detectorStrings[i]);

    return knownDetectors;
}

/**
 * Retrieves the detector types used in the current analysis
 */
const set<string>& DetectorLibrary::GetUsedDetectors(void) const
{
    return usedTypes;
}

/**
 * Convert an index number into which module the detector resides in
 */
int DetectorLibrary::ModuleFromIndex(int index) const
{
    return int(index / pixie::numberOfChannels);
}

/**
 * Convert an index number into which channel the detector resides in
 */
int DetectorLibrary::ChannelFromIndex(int index) const
{
    return (index % pixie::numberOfChannels);
}

/**
 * Make a unique map key for a give detector type and subtype
 */
DetectorLibrary::mapkey_t DetectorLibrary::MakeKey(const string &type, const string &subtype) const
{
    return (type + ':' + subtype);
}
