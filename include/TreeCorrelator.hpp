#ifndef TREECORRELATOR_H
#define TREECORRELATOR_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include "Places.hpp"

using namespace std;

/** Singleton class holding map of all places.*/
class TreeCorrelator {
    public:
        /** Returns only instance (reference) of TCorrelator class.*/
        static TreeCorrelator& get() {
            // Safe for destruction, placed in the static memory
            static TreeCorrelator instance;
            return instance;
        }

        /** Add place to the map. If place exists program is halted. */
        void addPlace(string name, Place* place, bool verbose = false) {
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

        /** Add child to place parent with coincidence coin.*/
        void addChild(string parent, string child, bool coin = true, bool verbose = false) {
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

        /** This map holds all Places. */
        map<string, Place*> places;
    private:
        /** Make constructor, copy-constructor and operator =
         * private to complete singleton implementation.*/
        TreeCorrelator() {}
        /* Do not implement*/
        TreeCorrelator(TreeCorrelator const&);
        void operator=(TreeCorrelator const&);
};

#endif
