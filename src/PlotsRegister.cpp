/** \file PlotsRegister.cpp
 *
 *  Declaration of singleton register
 */

#include "PlotsRegister.hpp"

#include <iostream>

#include <cstdlib>

using std::cerr;
using std::cout;
using std::endl;

PlotsRegister PlotsRegister::plotsReg(true);

/**
 * return the singleton instance
 */
PlotsRegister* PlotsRegister::R() 
{
    return &plotsReg;
}

bool PlotsRegister::CheckRange (int offset, int range) const
{
    bool exists = false;
    int min = offset;
    int max = offset + range - 1;
    
    unsigned sz = reg.size();
    for (unsigned id = 0; id < sz; ++id) {
	if (min > reg[id].max && max < reg[id].min)
	    continue;
	if ( (min > reg[id].min && min < reg[id].max) || 
	     (max > reg[id].min && max < reg[id].max) ) {
	    exists = true;
	    break;
	}
    }
    return exists;
}

bool PlotsRegister::Add (int offset, int range)
{
    // Special case: empty Plots list
    if (offset == 0 && range == 0)
	return true;
    
    int min = offset;
    int max = offset + range - 1;
    
    if (max < min) {
	cerr << "Attempt to register incorrect histogram ids range: " << min << " to" << max << endl;
	exit(1);
    }
    
    if (min < 1 || max > 7999) {
	cerr << "Attempt to register histogram ids: " << min << " to " << max << endl;
	cerr << "Valid range is 1 to 7999" << endl;
	exit(1);
    }
    
    if (CheckRange(min, max)) {
	cerr << "Attempt to register histogram ids: " << min << " to " << max << endl;
	// cerr << "This range is already registered: " << reg[id].first << " to " << reg[id].second << endl;
	exit(EXIT_FAILURE);
    }
    
    reg.push_back( MinMax(min, max) );
    return true;        
}
