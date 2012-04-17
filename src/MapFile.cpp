/** \file MapFile.cpp
 * Implementation to read in a new format map file
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "DetectorLibrary.hpp"
#include "MapFile.hpp"
#include "RawEvent.h"

#include "param.h"

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
    const size_t maxConfigLineLength = 100;

    char line[maxConfigLineLength];

    ifstream in(filename.c_str());
    
    if (!in) {
	cout << "Could not find new map file " << filename << endl;
	// Perhaps call old version here
	return;
    }
    
    // build the channel list
    for (size_t i=0; i < pixie::numberOfChannels; i++) {
	completeChannelList.push_back(i);
    }

    vector<string> tokenList;
    vector< vector<string> > normalLines;
    vector< vector<string> > wildcardLines;

    while (!in.eof()) {
	tokenList.clear();
	in.getline(line, maxConfigLineLength);
	if (!in.good())
	    break;
	
	// Allowing for comments for any line not starting with a number or a star
	if (!isdigit(line[0]) && line[0] != '*')
	    continue;

	TokenizeString(string(line), tokenList);
	if (tokenList.size() < 3) {
	    cerr << "Too few tokens in map file line: " << endl;
	    cerr << line << endl;
	    continue;
	}

	// separate lines based on whether or not they contain wildcards
	//   wildcards are * or ranges (i.e. 1-4) or even/odd for channels given by 'e' / 'o'
	if ( HasWildcard(tokenList.at(0)) || HasWildcard(tokenList.at(1)) ) {
	    wildcardLines.push_back(tokenList);
	} else {
	    normalLines.push_back(tokenList);
	}
    }

    // now we parse each tokenized line, processing wildcard lines afterwards
    for (vector< vector<string> >::iterator it = normalLines.begin();
	 it != normalLines.end(); it++) {
	ProcessTokenList(*it);
    }
    for (vector< vector<string> >::iterator it = wildcardLines.begin();
	 it != wildcardLines.end(); it++) {
	ProcessTokenList(*it);
    }
}

/**
 *  Tokenize a line from the map file, breaking into words separated by whitespace
 */
void MapFile::TokenizeString(const string &in, vector<string> &out) const
{
    string delimiters(" \t");

    // skip delimiters at beginning.
    string::size_type lastPos = in.find_first_not_of(delimiters, 0);
    
    // find first "non-delimiter".
    string::size_type pos = in.find_first_of(delimiters, lastPos);
    
    while (pos != string::npos || lastPos != string::npos) {
	// found a token, add it to the vector
	out.push_back(in.substr(lastPos, pos - lastPos));
	
	// skip delimiters.  Note the "not_of"
	lastPos = in.find_first_not_of(delimiters, pos);
	
	// find next "non-delimiter"
	pos = in.find_first_of(delimiters, lastPos);
    }
}

/**
 * Process a tokenized line of the mapfile and fill the global identifier array
 */
void MapFile::ProcessTokenList(const vector<string> &tokenList) const
{
    extern DetectorLibrary modChan;

    vector<int> moduleList;
    vector<int> channelList;

    // first token corresponds to module list
    TokenToVector(tokenList.at(0), moduleList, completeModuleList);
    // second token corresponds to channel list
    TokenToVector(tokenList.at(1), channelList, completeChannelList);
    // third token corresponds to detector type
    Identifier id;

    string type(tokenList.at(2));
    string subtype;
    // fourth token can provide the subytpe
    if (tokenList.size() > 3) {
	subtype = tokenList.at(3);
    } else {
	subtype = tokenList.at(2);
    }
    id.SetType(type);
    id.SetSubtype(subtype);

    // fifth token gives the starting location number
    //   otherwise we just find the smallest location in the map for the type/subtype
    int startingLocation;
    if (tokenList.size() > 4) {
	stringstream(tokenList.at(4)) >> startingLocation;
    } else {
	startingLocation = modChan.GetNextLocation(type, subtype);
    }

    for (vector<int>::iterator modIt = moduleList.begin();
	 modIt != moduleList.end(); modIt++) {
	for (vector<int>::iterator chanIt = channelList.begin();
	     chanIt != channelList.end(); chanIt++) {
	    int index = modChan.GetNumber(*modIt, *chanIt);
	    id.SetLocation(startingLocation);
	    
	    modChan[index] = id;
	}
    }
}

/**
 * Test if a string has a wildcard 
 */
bool MapFile::HasWildcard(const string &str) const
{
    return (str.find_first_of("*-eo",0) != string::npos);
}

/**
 * Convert a token into a string 
 */
void MapFile::TokenToVector(string token, vector<int> &list,
			    const vector<int> &completeList ) const
{
    if (!HasWildcard(token)) {
	// this is a single number
	int num;

	stringstream(token) >> num;
	list.push_back(num);
	return;
    }
    // first check for all indicator '*'
    if (token.find_first_of('*') != string::npos) {
	// then we just copy the complete list
	list = completeList;
	return;
    }

    // first check for even / odd
    bool hasEven = false;
    bool hasOdd  = false;
    string::size_type pos;

    pos = token.find_first_of('e');
    if (pos != string::npos) {
	hasEven = true;
	token.erase(pos, 1);
    }

    pos = token.find_first_of('o');
    if (pos != string::npos) {
	hasOdd = true;
	token.erase(pos, 1);
    }

    int low = *min_element(completeList.begin(), completeList.end());
    int high = *max_element(completeList.begin(), completeList.end());
    
    // next look for a range indicator "-
    pos = token.find_first_of('-');
    if (pos != string::npos) {
	string leftToken(token, 0, pos);
	string rightToken(token, pos+1);
	
	stringstream(leftToken) >> low;
	stringstream(rightToken) >> high;
    }

    // finally parse the complete list and fill the list accordingly
    for (vector<int>::const_iterator it = completeList.begin();
	 it != completeList.end(); it++) {
	if ( *it < low || *it > high )
	    continue;
	if ( hasOdd && ( *it % 2 ) == 0 )
	    continue;
	if ( hasEven && ( *it % 2 ) != 0 ) 
	    continue;
	list.push_back(*it);
    }
}

MapFile theMapFile(MapFile::defaultFile);
// This strangely segfaults by calling the constructor repeatedly
// MapFile theMapFile;

