///@file DetectorLibrary.cpp
///@brief Some useful function for managing the list of channel channelConfigurations
///@author David Miller, K. Miernik, S. V. Paulauskas

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <string>

#include "Constants.hpp"
#include "DetectorLibrary.hpp"
#include "MapNodeXmlParser.hpp"
#include "Messenger.hpp"

using namespace std;

set<int> DetectorLibrary::emptyLocations;

DetectorLibrary *DetectorLibrary::instance = NULL;

DetectorLibrary *DetectorLibrary::get() {
    if (!instance)
        instance = new DetectorLibrary();
    return (instance);
}

DetectorLibrary::DetectorLibrary() : vector<ChannelConfiguration>(), locations(), numModules(0) {
    try {
        MapNodeXmlParser parser;
        parser.ParseNode(this);
    } catch (invalid_argument &ia) {
        throw;
    }
}

DetectorLibrary::const_reference DetectorLibrary::at(DetectorLibrary::size_type idx) const {
    return vector<ChannelConfiguration>::at(idx);
}

DetectorLibrary::const_reference DetectorLibrary::at(DetectorLibrary::size_type mod, DetectorLibrary::size_type ch) const {
    return vector<ChannelConfiguration>::at(GetIndex(mod, ch));
}

DetectorLibrary::reference DetectorLibrary::at(DetectorLibrary::size_type idx) {
    return vector<ChannelConfiguration>::at(idx);
}

DetectorLibrary::reference DetectorLibrary::at(DetectorLibrary::size_type mod, DetectorLibrary::size_type ch) {
    return vector<ChannelConfiguration>::at(GetIndex(mod, ch));
}

void DetectorLibrary::push_back(const ChannelConfiguration &x) {
    mapkey_t key = MakeKey(x.GetType(), x.GetSubtype());
    locations[key].insert(x.GetLocation());
    vector<ChannelConfiguration>::push_back(x);
}

const set<int> &DetectorLibrary::GetLocations(const ChannelConfiguration &id) const {
    return GetLocations(id.GetType(), id.GetSubtype());
}

const set<int> &DetectorLibrary::GetLocations(const std::string &type, const std::string &subtype) const {
    mapkey_t key = MakeKey(type, subtype);

    if (locations.count(key) > 0)
        return locations.find(key)->second;
    else
        return emptyLocations;
}

unsigned int DetectorLibrary::GetNextLocation(const ChannelConfiguration &id) const {
    return GetNextLocation(id.GetType(), id.GetSubtype());
}

unsigned int DetectorLibrary::GetNextLocation(const std::string &type, const std::string &subtype) const {
    mapkey_t key = MakeKey(type, subtype);

    if (locations.count(key) > 0) {
        return *(locations.find(key)->second.rbegin()) + 1;
    } else {
        return 0;
    }
}

unsigned int DetectorLibrary::GetIndex(int mod, int chan) const {
    return mod * Pixie16::maximumNumberOfChannels + chan;
}

bool DetectorLibrary::HasValue(int mod, int chan) const {
    return HasValue(GetIndex(mod, chan));
}

bool DetectorLibrary::HasValue(int index) const {
    return ((signed) size() > index && at(index).GetType() != "");
}

void DetectorLibrary::Set(int index, const ChannelConfiguration &value) {
    if (knownDetectors.find(value.GetType()) == knownDetectors.end())
        knownDetectors.insert(value.GetType());

    unsigned int module = ModuleFromIndex(index);
    if (module >= numModules) {
        numModules = module + 1;
        resize(numModules * Pixie16::maximumNumberOfChannels);
        if (!value.HasTag("virtual"))
            numPhysicalModules = module + 1;
    }

    string key;
    key = value.GetType() + ':' + value.GetSubtype();
    locations[key].insert(value.GetLocation());

    usedTypes.insert(value.GetType());
    usedSubtypes.insert(value.GetSubtype());

    at(index) = value;
}

void DetectorLibrary::Set(int mod, int ch, const ChannelConfiguration &value) {
    Set(GetIndex(mod, ch), value);
}

///@TODO this needs moved to UtkUnpacker
//void DetectorLibrary::PrintUsedDetectors(RawEvent &rawev) const {
//    Messenger m;
//    stringstream ss;
//    ss << usedTypes.size() << " detector types are used in this analysis " << "and are named:";
//    m.detail(ss.str());
//    ss.str("");
//    copy(usedTypes.begin(), usedTypes.end(), ostream_iterator<string>(ss, " "));
//    m.detail(ss.str(), 1);
//    ss.str("");
//
//    ss << usedSubtypes.size() << " detector subtypes are used in this " << "analysis and are named:";
//    m.detail(ss.str());
//    ss.str("");
//    copy(usedSubtypes.begin(), usedSubtypes.end(), ostream_iterator<string>(ss, " "));
//    m.detail(ss.str(), 1);
//
//    rawev.Init(usedTypes);
//}

const set <string> &DetectorLibrary::GetUsedDetectors(void) const {
    return usedTypes;
}

int DetectorLibrary::ModuleFromIndex(int index) const {
    return int(index / Pixie16::maximumNumberOfChannels);
}

int DetectorLibrary::ChannelFromIndex(int index) const {
    return (index % Pixie16::maximumNumberOfChannels);
}

DetectorLibrary::mapkey_t DetectorLibrary::MakeKey(const std::string &type, const std::string &subtype) const {
    return (type + ':' + subtype);
}

DetectorLibrary::~DetectorLibrary() {
    delete instance;
    instance = NULL;
}