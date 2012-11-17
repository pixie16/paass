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
        /** Returns only instance of TreeCorrelator class.*/
        static TreeCorrelator* get();

        /** Add place to the map. If place exists program is halted. */
        void addPlace(string name, Place* place, bool verbose = false);

        /** Add child to place parent with coincidence coin.*/
        void addChild(string parent, string child, bool coin = true,
                      bool verbose = false);

        /** Creates most basic place read from map. Type of created place
         * is Detector. It is important to use this to create basic places,
         * as this way the owner of all places is TreeCorrelator.*/
        void addBasicPlace(string name);

        /** 
         * This function initializes the correlator tree. Should be called
         * after all basic places from map2.txt were initialized.
        */
        void buildTree();

        ~TreeCorrelator();

        /** This map holds all Places. */
        map<string, Place*> places;
    private:
        /** Make constructor, copy-constructor and operator =
         * private to complete singleton implementation.*/
        TreeCorrelator() {}
        /* Do not implement*/
        TreeCorrelator(TreeCorrelator const&);
        void operator=(TreeCorrelator const&);
        static TreeCorrelator* instance;
};

#endif
