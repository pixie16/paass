/** \file MapFile.cpp
 * Implementation to read in a new format map file
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator> // tmp
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

#include "DetectorLibrary.hpp"
#include "MapFile.hpp"
#include "RawEvent.hpp"
#include "TreeCorrelator.hpp"
#include "PathHolder.hpp"
#include "Exceptions.hpp"

#include "Globals.hpp"
#include "Messenger.hpp"

using namespace std;

/**
 * Contains the description of each channel in the analysis.  The
 * description is read in from map.txt and includes the detector type and 
 * subtype, the damm spectrum number, and physical location
 */

/**
 * Read in a map file 
 * This occurs in the constructor so to preempt scanor fortran routines;
 *   this allows us to define the operation of the program dependent
 *   on what detector types show up in the map
 */
MapFile::MapFile(const string &filename /*="map2.txt"*/)
{  
    PathHolder* conf_path = new PathHolder();
    string mapFileName = conf_path->GetFullPath(filename);
    delete conf_path;

    ifstream in(mapFileName.c_str());
     
    if (!in.good()) {
        throw IOException("Could not open map file " + mapFileName);
    }

    LoadXml();
    
    /*
    const size_t maxConfigLineLength = 100;
    char line[maxConfigLineLength];
     
    vector<string> tokenList;
    vector< vector<string> > normalLines;
    vector< vector<string> > wildcardLines;

    while (!in.eof()) {
        tokenList.clear();
        in.getline(line, maxConfigLineLength);
        if (in.fail())
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

    Messenger m;
    m.start("Loading map file: " + mapFileName);

    // now we parse each tokenized line, processing wildcard lines afterwards
    for (vector< vector<string> >::iterator it = normalLines.begin();
	 it != normalLines.end(); it++) {
        ProcessTokenList(*it);
    }
    for (vector< vector<string> >::iterator it = wildcardLines.begin();
	 it != wildcardLines.end(); it++) {
        ProcessTokenList(*it);
    }
    m.done();
    */

    /* At this point basic Correlator places build automatically from
     * map file should be created so we can call buildTree function */
    try {
        TreeCorrelator::get()->buildTree();
    } catch (exception &e) {
        cout << "Exception caught at MapFile.cpp" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    isRead = true;
}

void MapFile::LoadXml() {
    pugi::xml_document doc;

    PathHolder* conf_path = new PathHolder();
    string xmlFileName = conf_path->GetFullPath("Config.xml");
    delete conf_path;

    pugi::xml_parse_result result = doc.load_file(xmlFileName.c_str());
    if (!result) {
        stringstream ss;
        ss << "MapFile: error parsing file " << xmlFileName;
        ss << " : " << result.description();
        cout << ss.str() << endl;
    }

    Messenger m;
    m.detail("Loading channels map");

    DetectorLibrary* modChan = DetectorLibrary::get();

    /** These attributes have reserved meaning, all other
     * attributes of <Channel> are treated as tags */
    set<string> reserved;
    reserved.insert("number");
    reserved.insert("type");
    reserved.insert("subtype");
    reserved.insert("location");

    pugi::xml_node map = doc.child("Configuration").child("Map");
    bool verbose = map.attribute("verbose_map").as_bool();
    pugi::xml_node tree = doc.child("Configuration").child("TreeCorrelator");
    bool verbose_tree = tree.attribute("verbose").as_bool(false);
    for (pugi::xml_node module = map.child("Module"); module;
         module = module.next_sibling("Module")) {
        int module_number = module.attribute("number").as_int();
        if (module_number < 0) {
            stringstream ss;
            ss << "MapFile: Illegal module number "
                << "found " << module_number << " in cofiguration file.";
            throw GeneralException(ss.str());
        }
        for (pugi::xml_node channel = module.child("Channel"); channel;
             channel = channel.next_sibling("Channel")) {
            int ch_number = channel.attribute("number").as_int(-1);
            if (ch_number < 0) {
                stringstream ss;
                ss << "DetectorDriver::ReadWalk: Illegal channel number "
                   << "found " << ch_number << " in cofiguration file.";
                throw GeneralException(ss.str());
            }
            if ( modChan->HasValue(module_number, ch_number) ) {
                stringstream ss;
                ss << "MapFile: Identifier for module " << module_number 
                   << ", channel " << ch_number
                   << " is initialized more than once";
                throw GeneralException(ss.str());
            }
            Identifier id;

            string ch_type = channel.attribute("type").as_string("None");
            id.SetType(ch_type);

            string ch_subtype = channel.attribute("subtype").as_string("None");
            id.SetSubtype(ch_subtype);

            int ch_location = channel.attribute("location").as_int(-1);
            if (ch_location == -1) {
                //Take next available
                ch_location = modChan->GetNextLocation(ch_type, ch_subtype);
            }
            id.SetLocation(ch_location);

            for (pugi::xml_attribute_iterator ait = channel.attributes_begin();
                 ait != channel.attributes_end(); ++ait) {
                string name = ait->name();
                if (reserved.find(name) != reserved.end()) {
                    Identifier::TagValue value(ait->as_int());
                    id.AddTag(name, value);
                }
            }

            modChan->Set(module_number, ch_number, id);

            /** Create basic place for TreeCorrelator */
            std::map <string, string> params;
            params["name"] = id.GetPlaceName();
            params["parent"] = "root";
            params["type"] = "PlaceDetector";
            params["reset"] = "true";
            params["fifo"] = "2";
            TreeCorrelator::get()->createPlace(params, verbose_tree);

            if (verbose) {
                stringstream ss;
                ss << "Module " << module_number 
                   << ", channel " << ch_number  << ", type "
                   << ch_type << " "
                   << ch_subtype << ", location " 
                   << ch_location;
                Messenger m;
                m.detail(ss.str(), 1);
            }
        }
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
    DetectorLibrary* modChan = DetectorLibrary::get();
    
    vector<int> moduleList;
    vector<int> channelList;

    vector<string>::const_iterator tokenIt = tokenList.begin();

    // first token corresponds to module list
    TokenToVector(*tokenIt++, moduleList, modChan->GetModules() );
    // second token corresponds to channel list
    TokenToVector(*tokenIt++, channelList, pixie::numberOfChannels);
    // third token corresponds to detector type
    Identifier id;

    string type(*tokenIt++);
    string subtype;

    // fourth token can provide the subytpe
    //   if token is a whole number, assume it as a location and skip
    if (tokenIt != tokenList.end() && 
        tokenIt->find_first_not_of("1234567890") != string::npos) {
        // synonym for "Same as type"
        if ( *tokenIt == "--" ) {
            subtype = type;
        } else { 
            subtype = *tokenIt;
        }
	tokenIt++;
    } else {
        subtype = type;
    }

    id.SetType(type);
    id.SetSubtype(subtype);

    // fifth token gives the starting location number
    //   otherwise we just find the smallest location in the map for the type/subtype
    int startingLocation;
    if (tokenIt != tokenList.end() &&
	tokenIt->find_first_not_of("1234567890") == string::npos ) {
        stringstream(*tokenIt) >> startingLocation;
        tokenIt++;
    } else {
        startingLocation = modChan->GetNextLocation(type, subtype);
    }

    // process additional identifiers as key=integer or toggling boolean flag to true (1)
    for (;tokenIt != tokenList.end(); tokenIt++) {
	string::size_type equalPos = tokenIt->find_first_of('=');

	if (equalPos == string::npos) {
	    id.AddTag(*tokenIt, Identifier::TagValue(1) );
	} else {
	    string key   = tokenIt->substr(0,equalPos);
	    Identifier::TagValue value;

	    stringstream(tokenIt->substr(equalPos+1)) >> value;
	    id.AddTag(tokenIt->substr(0,equalPos), value);
	}       
    }

    for (vector<int>::iterator modIt = moduleList.begin();
	 modIt != moduleList.end(); modIt++) {
        for (vector<int>::iterator chanIt = channelList.begin();
            chanIt != channelList.end(); chanIt++) {
            // check if this channel has already been defined
            if ( modChan->HasValue(*modIt, *chanIt) ) {
            // if this is a wildcard line, just continue
            if (HasWildcard(tokenList.at(0)) ||
                HasWildcard(tokenList.at(1))) {
                continue;
            }
            cerr << "Identifier for " << type << " in module " << *modIt
                << " : channel " << *chanIt << " is initialized more than once in the map file"
                << endl;

            exit(EXIT_FAILURE);
            }

            id.SetLocation(startingLocation);
            modChan->Set(*modIt, *chanIt, id);

            /* 
             * Create basic places for correlator
             * names are build as
             *      type_subtype_location
             *      eg. ge_clover_high_5 
             * see also RawEvent.hpp, Identifier::GetPlaceName()
             */
            map<string, string> params;
            params["name"] = id.GetPlaceName();
            params["parent"] = "root";
            params["type"] = "PlaceDetector";
            params["reset"] = "true";
            params["fifo"] = "2";
            try {
                TreeCorrelator::get()->createPlace(params, verbose::MAP_INIT);
            } catch (exception &e) {
                cout << "Exception caught at MapFile.cpp" << endl;
                cout << "\t" << e.what() << endl;
                exit(EXIT_FAILURE);
            }

            startingLocation++;
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
void MapFile::TokenToVector(string token, vector<int> &list, int number) const
{
    if (!HasWildcard(token)) {
	// this is a single number
	int num;

	stringstream(token) >> num;
	list.push_back(num);
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

    // this also properly handles the "*" wildcard to do all modules/channels
    int low = 0;
    int high = number-1;
    
    // next look for a range indicator "-"
    pos = token.find_first_of('-');
    if (pos != string::npos) {
	string leftToken(token, 0, pos);
	string rightToken(token, pos+1);
	
	stringstream(leftToken) >> low;
	stringstream(rightToken) >> high;
	
	// extend the range if the number is not high enough
	number = max(high+1, number);
    }

    // finally parse the complete list and fill the list accordingly
    for (int i=low; i <= high; i++) {
	if ( hasOdd && ( i % 2 ) == 0 )
	    continue;
	if ( hasEven && ( i % 2 ) != 0 ) 
	    continue;
	list.push_back(i);
    }
}


