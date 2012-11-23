#include <iostream>
#include <string>
#include <sstream>

#include "DetectorLibrary.hpp"
#include "TreeCorrelator.hpp"
#include "Globals.hpp"
#include "Exceptions.hpp"

using namespace std;

void Walker::parsePlace(pugi::xml_node node, string parent) {
    map<string, string> params;
    params["parent"] = parent;
    params["type"] = "";
    params["reset"] = "true";
    params["coincidence"] = "true";
    params["fifo"] = "2";
    for (pugi::xml_attribute attr = node.first_attribute();
         attr;
         attr = attr.next_attribute()) {
        params[attr.name()] = attr.value();
    }
    TreeCorrelator::get()->createPlace(params);
}

void Walker::traverseTree(pugi::xml_node node, string parent) {
    for (pugi::xml_node child = node.child("Place");
         child;
         child = child.next_sibling("Place")) {
        parsePlace(child, parent);
        traverseTree(child, string(child.attribute("name").value()));
    }
}


TreeCorrelator* TreeCorrelator::instance = NULL;

PlaceBuilder TreeCorrelator::builder = PlaceBuilder();

/** Instance is created upon first call */
TreeCorrelator* TreeCorrelator::get() {
    if (!instance) {
        instance = new TreeCorrelator();
        cout << "Creating instance of TreeCorrelator" << endl;
    }
    return instance;
}

void TreeCorrelator::addChild(string parent, string child, 
                             bool coin /* = true*/, bool verbose /*= false*/) {
    if (places.count(parent) == 1 || places.count(child) == 1) {
        places[parent]->addChild(places[child], coin);
        if (verbose) {
            cout << "TreeCorrelator: setting " << child 
                 << " as a child of " << parent << endl;
        }
    } else {
        stringstream ss;
        ss << "Error: TreeCorrelator: could not set " << child
           << "as a child of " << parent << endl;
        throw GeneralException(ss.str());
    }
}

vector<string> TreeCorrelator::split_names(string name) {
    vector<string> names;
    vector<string> name_tokens = strings::tokenize(name, "_");

    if (name_tokens.size() > 1 &&
        name_tokens.back().find("-") != string::npos) {
        vector<string> range_tokens = 
                                strings::tokenize(name_tokens.back(), "-");
        int range_min = strings::to_int(range_tokens[0]);
        int range_max = strings::to_int(range_tokens[1]);
        string base_name;
        for (vector<string>::iterator it = name_tokens.begin();
             it != name_tokens.end() - 1;
             ++it)
            base_name += (*it) + "_";
        for (int i = range_min; i <= range_max; ++i) {
            stringstream ss;
            ss << i;
            names.push_back(base_name + ss.str());
        }
    } else {
        names.push_back(name);
    }
    return names;
}

void TreeCorrelator::createPlace(map<string, string>& params,
                                 bool verbose /*= false*/) {
    vector<string> names = split_names(params["name"]);

    for (vector<string>::iterator it = names.begin();
         it != names.end();
         ++it) {

        if (params["type"] != "") {
            if (places.count((*it)) == 1) {
                stringstream ss;
                ss << "Place" << (*it) << " already exists";
                throw GeneralException(ss.str());
            }
            Place* current = builder.create(params);
            places[(*it)] = current;
            if (verbose::CORRELATOR_INIT)
                cout << "TreeCorrelator: creating place " << (*it) << endl;
        }

        if (params["parent"] != "root") {
            bool coincidence = strings::to_bool(params["coincidence"]);
            addChild(params["parent"], (*it), coincidence,
                     verbose::CORRELATOR_INIT);
        }

    }
}

void TreeCorrelator::buildTree() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("TreeCorrelator.xml");
    cout << "Loading configiration file TreeCorrelator.xml: "
         << result.description() 
         << endl << "Configuration description: "
         << doc.child("TreeCorrelator").attribute("description").value()
         << endl;

    pugi::xml_node tree = doc.child("TreeCorrelator");
    Walker walker;
    walker.traverseTree(tree, string(tree.attribute("name").value()));
}

/** OBSOLETE */
void TreeCorrelator::buildTree_LeRIBSS() {
    /** Setup for LeRIBBS 4 Clovers*/

    /* Use this constant for debugging */
    cout << "TreeCorrelator: building tree" << endl;

    /*
    PlaceOR* clover0 = new PlaceOR();
    addPlace("Clover0", clover0, verbose::CORRELATOR_INIT);
    PlaceOR* clover1 = new PlaceOR();
    addPlace("Clover1", clover1, verbose::CORRELATOR_INIT);
    PlaceOR* clover2 = new PlaceOR();
    addPlace("Clover2", clover2, verbose::CORRELATOR_INIT);
    PlaceOR* clover3 = new PlaceOR();
    addPlace("Clover3", clover3, verbose::CORRELATOR_INIT);

    PlaceOR* beta = new PlaceOR(true, 10);
    addPlace("Beta", beta, verbose::CORRELATOR_INIT);
    
    PlaceOR* gamma = new PlaceOR();
    addPlace("Gamma", gamma, verbose::CORRELATOR_INIT);
    addChild("Gamma", "Clover0", true, verbose::CORRELATOR_INIT);
    addChild("Gamma", "Clover1", true, verbose::CORRELATOR_INIT);
    addChild("Gamma", "Clover2", true, verbose::CORRELATOR_INIT);
    addChild("Gamma", "Clover3", true, verbose::CORRELATOR_INIT);

    PlaceAND* gammabeta = new PlaceAND();
    addPlace("GammaBeta", gammabeta, verbose::CORRELATOR_INIT);
    places["GammaBeta"] = new PlaceAND();
    addChild("GammaBeta", "Gamma", true, verbose::CORRELATOR_INIT);
    addChild("GammaBeta", "Beta", true, verbose::CORRELATOR_INIT);

    PlaceAND* gammawobeta = new PlaceAND();
    addPlace("GammaWOBeta", gammawobeta, verbose::CORRELATOR_INIT);
    addChild("GammaWOBeta", "Gamma", true, verbose::CORRELATOR_INIT);
    addChild("GammaWOBeta", "Beta", false, verbose::CORRELATOR_INIT);

    // Active if tape is moving
    PlaceDetector* tapemove = new PlaceDetector(false);
    addPlace("TapeMove", tapemove, verbose::CORRELATOR_INIT);
    // Active if beam is on
    PlaceDetector* beam = new PlaceDetector(false);
    addPlace("Beam", beam, verbose::CORRELATOR_INIT);
    // Activated with beam start, deactivated with TapeMove
    PlaceDetector* cycle  = new PlaceDetector(false);
    addPlace("Cycle", cycle, verbose::CORRELATOR_INIT);

    DetectorLibrary* modChan = DetectorLibrary::get();

    // Basic places are created in MapFile.cpp
    // Here we group them as children of just created abstract places
    unsigned int sz = modChan->size();
    for (unsigned i = 0; i < sz; ++i) {
        string type = (*modChan)[i].GetType();
        string subtype = (*modChan)[i].GetSubtype();
        int location = (*modChan)[i].GetLocation();
        string name = (*modChan)[i].GetPlaceName();

        if (type == "ge" && subtype == "clover_high") {
            int clover_number = int(location / 4);
            stringstream clover;
            clover << "Clover" << clover_number;
            addChild(clover.str(), name, true, verbose::CORRELATOR_INIT);
        } else if (type == "beta_scint" && subtype == "beta") {
            addChild("Beta", name, true, verbose::CORRELATOR_INIT);
        }
    }
    */
}

/** OBSOLETE */
void TreeCorrelator::buildTree_Hybrid() {
    /** Setup for LeRIBBS 3Hen Hybrid */

    /* Use this constant for debugging */
    cout << "DetectorDriver::InitializeCorrelator()" << endl;

    /*
    PlaceOR* clover0 = new PlaceOR();
    addPlace("Clover0", clover0, verbose::CORRELATOR_INIT);
    PlaceOR* clover1 = new PlaceOR();
    addPlace("Clover1", clover1, verbose::CORRELATOR_INIT);

    PlaceOR* beta = new PlaceOR(true, 10);
    addPlace("Beta", beta, verbose::CORRELATOR_INIT);
    
    // All Hen3 events
    PlaceCounter* hen3 = new PlaceCounter();
    addPlace("Hen3", hen3, verbose::CORRELATOR_INIT);

    // Real neutrons (children are thresholded)
    PlaceCounter* neutrons = new PlaceCounter();
    addPlace("Neutrons", neutrons, verbose::CORRELATOR_INIT);

    PlaceOR* gamma = new PlaceOR();
    addPlace("Gamma", gamma, verbose::CORRELATOR_INIT);
    addChild("Gamma", "Clover0", true, verbose::CORRELATOR_INIT);
    addChild("Gamma", "Clover1", true, verbose::CORRELATOR_INIT);

    PlaceAND* gammabeta = new PlaceAND();
    addPlace("GammaBeta", gammabeta, verbose::CORRELATOR_INIT);
    places["GammaBeta"] = new PlaceAND();
    addChild("GammaBeta", "Gamma", true, verbose::CORRELATOR_INIT);
    addChild("GammaBeta", "Beta", true, verbose::CORRELATOR_INIT);

    PlaceAND* gammawobeta = new PlaceAND();
    addPlace("GammaWOBeta", gammawobeta, verbose::CORRELATOR_INIT);
    addChild("GammaWOBeta", "Gamma", true, verbose::CORRELATOR_INIT);
    addChild("GammaWOBeta", "Beta", false, verbose::CORRELATOR_INIT);

    // Active if tape is moving
    PlaceDetector* tapemove = new PlaceDetector(false);
    addPlace("TapeMove", tapemove, verbose::CORRELATOR_INIT);
    // Active if beam is on
    PlaceDetector* beam = new PlaceDetector(false);
    addPlace("Beam", beam, verbose::CORRELATOR_INIT);
    // Activated with beam start, deactivated with TapeMove
    PlaceDetector* cycle  = new PlaceDetector(false);
    addPlace("Cycle", cycle, verbose::CORRELATOR_INIT);

    DetectorLibrary* modChan = DetectorLibrary::get();

    // Basic places are created in MapFile.cpp
    // Here we group them as children of just created abstract places
    unsigned int sz = modChan->size();
    for (unsigned i = 0; i < sz; ++i) {
        string type = (*modChan)[i].GetType();
        string subtype = (*modChan)[i].GetSubtype();
        int location = (*modChan)[i].GetLocation();
        string name = (*modChan)[i].GetPlaceName();

        if (type == "ge" && subtype == "clover_high") {
            int clover_number = int(location / 4);
            stringstream clover;
            clover << "Clover" << clover_number;
            addChild(clover.str(), name, true, verbose::CORRELATOR_INIT);
        } else if (type == "beta_scint" && subtype == "beta") {
            addChild("Beta", name, true, verbose::CORRELATOR_INIT);
        } else if (type == "3hen" && subtype == "big") {
            stringstream neutron;
            neutron << "Neutron" << location;
            PlaceThreshold* real_neutron  = new PlaceThreshold(detectors::neutronLowLimit,
                                                               detectors::neutronHighLimit);
            addPlace(neutron.str(), real_neutron, verbose::CORRELATOR_INIT);

            addChild("Hen3", name, true, verbose::CORRELATOR_INIT);
            addChild("Neutrons", neutron.str(), true, verbose::CORRELATOR_INIT);
        }
    }
    */
}

TreeCorrelator::~TreeCorrelator() {
    for (map<string, Place*>::iterator it = places.begin(); 
         it != places.end(); ++it) {
            if (verbose::MAP_INIT)
                cout << "TreeCorrelator: deleting place " << (*it).first << endl;
            delete it->second;
    }
}

