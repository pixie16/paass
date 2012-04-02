/** \file MapFile.cpp
 * Implementation to read in a new format map file
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "MapFile.hpp"
#include "RawEvent.h"

using namespace std;

const string MapFile::defaultFile("map2.txt");

MapFile::MapFile()
{
    MapFile(defaultFile);
}

/**
 * Read in a map file 
 * This occurs in the constructor so to preempt scanor fortran routines;
 *   this allows us to define the operation of the program dependent
 *   on what detector types show up in the map
 */
MapFile::MapFile(const string &filename)
{
    ifstream in(filename.c_str());
    extern vector<Identifier> modChan;

    if (!in) {
	cout << "Could not find new map file " << filename << endl;
	// Perhaps call old version here
	return;
    }
    
    cout << "We do something" << endl;
}

MapFile theMapFile(MapFile::defaultFile);
// This strangely segfaults by calling the constructor repeatedly
// MapFile theMapFile;

