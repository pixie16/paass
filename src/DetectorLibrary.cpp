/** \file DetectorLibrary.cpp
 *  Some useful function for managing the list of channel identifiers
 */

#include <algorithm>
#include <map>
#include <string>

#include "DetectorLibrary.hpp"
#include "RawEvent.h"

#include "param.h"

using namespace std;

DetectorLibrary::DetectorLibrary() : vector<Identifier>(), highLocation()
{
    // do nothing
}

DetectorLibrary::~DetectorLibrary()
{
    // do nothing
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

int DetectorLibrary::GetNumber(int mod, int chan)
{
  if (mod + 1 > numModules) {
    numModules = mod + 1;
    resize(numModules * pixie::numberOfChannels);
  }
    
  return mod * pixie::numberOfChannels + chan;
}
