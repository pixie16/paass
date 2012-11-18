#include <iostream>
#include <string>
#include <sstream>

#include "DetectorLibrary.hpp"
#include "TreeCorrelator.hpp"
#include "Globals.hpp"

using namespace std;

TreeCorrelator* TreeCorrelator::instance = NULL;

/** Instance is created upon first call */
TreeCorrelator* TreeCorrelator::get() {
    if (!instance) {
        instance = new TreeCorrelator();
        cout << "Created instance of TreeCorrelator" << endl;
    }
    return instance;
}

void TreeCorrelator::addPlace(string name, Place* place,
                              bool verbose /*= false*/) {
    if (places.count(name) == 0) {
        places[name] = place;
        if (verbose) {
            cout << "TreeCorrelator: created place " << name << endl;
        }
    } else {
        cerr << "Error: TreeCorrelator: Place " << name
                << " already exists." << endl;
        exit(EXIT_FAILURE);
    }
}

void TreeCorrelator::addBasicPlace(string name) {
    PlaceDetector* place = new PlaceDetector();
    addPlace(name, place, verbose::MAP_INIT);
}

void TreeCorrelator::addChild(string parent, string child, 
                             bool coin /* = true*/, bool verbose /*= false*/) {
    if (places.count(parent) == 1 || places.count(child) == 1) {
        places[parent]->addChild(places[child], coin);
        if (verbose) {
            cout << "TreeCorrelator: setting " << child << " as a child of " << parent << endl;
        }
    } else {
        cerr << "Error: TreeCorrelator: could not set " << child
                << "as a child of " << parent << endl;
        exit(EXIT_FAILURE);
    }
}


void TreeCorrelator::buildTree() {
    /** Setup for LeRIBBS */

    /* Use this constant for debugging */
    cout << "TreeCorrelator: building tree" << endl;

    /** Here we create abstract places.*/
    PlaceOR* clover0 = new PlaceOR();
    addPlace("Clover0", clover0, verbose::CORRELATOR_INIT);
    PlaceOR* clover1 = new PlaceOR();
    addPlace("Clover1", clover1, verbose::CORRELATOR_INIT);
    PlaceOR* clover2 = new PlaceOR();
    addPlace("Clover2", clover2, verbose::CORRELATOR_INIT);
    PlaceOR* clover3 = new PlaceOR();
    addPlace("Clover3", clover3, verbose::CORRELATOR_INIT);

    /** Note that beta_scint detectors are acticated in BetaScintProcessor
     *  with threshold on energy defined therein.
     *  This place is also sensitive to this threshold as parent of beta places.
     *
     *  Fifo of this place is increased to accomodate multiplicity of beta
     *  events for longer events width. Check appopriate plot if depth is
     *  large enough.
     */
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
    /** End setup for LeRIBBS */
}

TreeCorrelator::~TreeCorrelator() {
    for (map<string, Place*>::iterator it = places.begin(); 
         it != places.end(); ++it) {
            if (verbose::MAP_INIT)
                cout << "TreeCorrelator: deleting place " << (*it).first << endl;
            delete it->second;
    }
}

