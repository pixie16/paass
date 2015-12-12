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

#include "pugixml.hpp"

#include "DetectorLibrary.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "RawEvent.hpp"
#include "TreeCorrelator.hpp"

using namespace std;

set<int> DetectorLibrary::emptyLocations;

DetectorLibrary* DetectorLibrary::instance = NULL;

DetectorLibrary* DetectorLibrary::get() {
    if (!instance) {
        instance = new DetectorLibrary();
    }
    return instance;
}

DetectorLibrary::DetectorLibrary() : vector<Identifier>(), locations(),
    numModules(0) {
    GetKnownDetectors();
    LoadXml();
    /* At this point basic Correlator places build automatically from
     * map file should be created so we can call buildTree function */
    try {
        TreeCorrelator::get()->buildTree();
    } catch (exception &e) {
        cout << "Exception caught at MapFile.cpp" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void DetectorLibrary::LoadXml() {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file("Config.xml");
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver: error parsing file Config.xml";
        ss << " : " << result.description();
        cout << ss.str() << endl;
    }

    Messenger m;
    m.start("Loading channels map");

    /** These attributes have reserved meaning, all other
     * attributes of [Channel] are treated as tags */
    set<string> reserved;
    reserved.insert("number");
    reserved.insert("type");
    reserved.insert("subtype");
    reserved.insert("location");
    reserved.insert("tags");

    pugi::xml_node map = doc.child("Configuration").child("Map");
    bool verbose = map.attribute("verbose_map").as_bool();
    pugi::xml_node tree = doc.child("Configuration").child("TreeCorrelator");
    bool verbose_tree = tree.attribute("verbose").as_bool(false);
    for (pugi::xml_node module = map.child("Module"); module;
         module = module.next_sibling("Module")) {
        int module_number = module.attribute("number").as_int(-1);
        if (module_number < 0) {
            stringstream ss;
            ss << "MapFile: Illegal module number "
                << "found " << module_number << " in configuration file.";
            throw GeneralException(ss.str());
        }
        for (pugi::xml_node channel = module.child("Channel"); channel;
             channel = channel.next_sibling("Channel")) {
            int ch_number = channel.attribute("number").as_int(-1);
            if (ch_number < 0 || ch_number >= (int)pixie::numberOfChannels ) {
                stringstream ss;
                ss << "MapFile : Identifier : Illegal channel number "
                   << "found " << ch_number << " in configuration file.";
                throw GeneralException(ss.str());
            }
            if ( HasValue(module_number, ch_number) ) {
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
                ch_location = GetNextLocation(ch_type, ch_subtype);
            }
            id.SetLocation(ch_location);

            string ch_tags = channel.attribute("tags").as_string("None");
            if(ch_tags != "None"){
                vector<string> tagList = strings::tokenize(ch_tags, ",");
                for(unsigned int i = 0; i < tagList.size(); i++)
                    id.AddTag(tagList[i], 1);
            }

            Set(module_number, ch_number, id);

            /** Create basic place for TreeCorrelator */
            std::map <string, string> params;
            params["name"] = id.GetPlaceName();
            params["parent"] = "root";
            params["type"] = "PlaceDetector";
            params["reset"] = "true";
            params["fifo"] = "2";
            params["init"] = "false";
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
    m.done();
}

DetectorLibrary::const_reference DetectorLibrary::at(DetectorLibrary::size_type idx) const {
    return vector<Identifier>::at(idx);
}

DetectorLibrary::const_reference DetectorLibrary::at(DetectorLibrary::size_type mod,
                                                     DetectorLibrary::size_type ch) const {
    return vector<Identifier>::at(GetIndex(mod,ch));
}

DetectorLibrary::reference DetectorLibrary::at(DetectorLibrary::size_type idx) {
    return vector<Identifier>::at(idx);
}

DetectorLibrary::reference DetectorLibrary::at(DetectorLibrary::size_type mod,
                                               DetectorLibrary::size_type ch) {
    return vector<Identifier>::at(GetIndex(mod,ch));
}


void DetectorLibrary::push_back(const Identifier &x) {
    mapkey_t key = MakeKey(x.GetType(), x.GetSubtype());

    locations[key].insert(x.GetLocation());
    vector<Identifier>::push_back(x);
}

const set<int>& DetectorLibrary::GetLocations(const Identifier &id) const {
    return GetLocations(id.GetType(), id.GetSubtype());
}

const set<int>& DetectorLibrary::GetLocations(const std::string &type,
                                              const std::string &subtype) const{
    mapkey_t key = MakeKey(type, subtype);

    if (locations.count(key) > 0) {
        return locations.find(key)->second;
    } else {
        return emptyLocations;
    }
}

int DetectorLibrary::GetNextLocation(const Identifier &id) const {
  return GetNextLocation(id.GetType(), id.GetSubtype());
}

int DetectorLibrary::GetNextLocation(const std::string &type,
				     const std::string &subtype) const {
    mapkey_t key = MakeKey(type, subtype);

    if (locations.count(key) > 0) {
        return *(locations.find(key)->second.rbegin()) + 1;
    } else {
        return 0;
    }
}

DetectorLibrary::size_type DetectorLibrary::GetIndex(int mod, int chan) const {
  return mod * pixie::numberOfChannels + chan;
}

bool DetectorLibrary::HasValue(int mod, int chan) const {
    return HasValue(GetIndex(mod,chan));
}

bool DetectorLibrary::HasValue(int index) const {
  return ((signed)size() > index && at(index).GetType() != "");
}

void DetectorLibrary::Set(int index, const Identifier& value) {
    /// Search the list of known detectors; if the detector type
    ///    is not matched, print out an error message and terminate
    if (knownDetectors.find(value.GetType()) == knownDetectors.end()) {
        stringstream ss;
        ss << "The detector called '" << value.GetType() << "'"
        << "read in from the file 'map2.txt' "
        << "is unknown to this program!.  This is a "
        << "fatal error.  Program execution halted! "
        << "If you believe this detector should exist, "
        << "please edit the 'getKnownDetectors' "
        << "function inside the 'DetectorLibrary.cpp' file."
        << endl;
        ss << "The currently known detectors include:" << endl;
        copy(knownDetectors.begin(), knownDetectors.end(),
            ostream_iterator<string>(ss, " "));
        throw GeneralException(ss.str());
    }

    unsigned int module = ModuleFromIndex(index);
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

void DetectorLibrary::Set(int mod, int ch, const Identifier &value) {
    Set(GetIndex(mod,ch), value);
}

void DetectorLibrary::PrintMap(void) const {
    cout << setw(4) << "MOD"
	 << setw(4) << "CH";
    Identifier::PrintHeaders();

    for (size_t i=0; i < size(); i++) {
        cout << setw(4) << ModuleFromIndex(i)
            << setw(4) << ChannelFromIndex(i);
        at(i).Print();
    }
}

void DetectorLibrary::PrintUsedDetectors(RawEvent& rawev) const {
    Messenger m;
    stringstream ss;
    ss << usedTypes.size() << " detector types are used in this analysis "
	   << "and are named:";
    m.detail(ss.str());
    ss.str("");
    copy(usedTypes.begin(), usedTypes.end(),
         ostream_iterator<string>(ss, " "));
    m.detail(ss.str(), 1);
    ss.str("");

    ss << usedSubtypes.size() <<" detector subtypes are used in this "
       << "analysis and are named:";
    m.detail(ss.str());
    ss.str("");
    copy(usedSubtypes.begin(), usedSubtypes.end(),
         ostream_iterator<string>(ss," "));
    m.detail(ss.str(), 1);

    rawev.Init(usedTypes);
}

const set<string>& DetectorLibrary::GetKnownDetectors(void) {
    const unsigned int detTypes = 24;
    const string detectorStrings[detTypes] = {
        "3hen", "beta", "dssd_front", "dssd_back", "ge", "generic",
        "idssd_front", "ignore", "ion_chamber", "liquid", "logic",
        "mcp", "mtc", "neutron_scint", "position", "pulser", "si", "ssd",
        "timeclass", "tvandle", "vandle", "beta_scint", "labr3", "pspmt"
    };

    if (!knownDetectors.empty())
        return knownDetectors;

    for (unsigned int i=0; i < detTypes; i++)
        knownDetectors.insert(detectorStrings[i]);

    return knownDetectors;
}

const set<string>& DetectorLibrary::GetUsedDetectors(void) const {
    return usedTypes;
}

int DetectorLibrary::ModuleFromIndex(int index) const {
    return int(index / pixie::numberOfChannels);
}

int DetectorLibrary::ChannelFromIndex(int index) const {
    return (index % pixie::numberOfChannels);
}

DetectorLibrary::mapkey_t DetectorLibrary::MakeKey(const std::string &type,
                                                   const std::string &subtype) const {
    return (type + ':' + subtype);
}

DetectorLibrary::~DetectorLibrary() {
    delete instance;
    instance = NULL;
}

