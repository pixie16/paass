#ifndef TREECORRELATOR_H
#define TREECORRELATOR_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <utility>
#include <stdexcept>
#include "pugixml.hpp"
#include "Places.hpp"
#include "PlaceBuilder.hpp"
#include "Exceptions.hpp"

using namespace std;

/** XML document walker and parser for TreeCorrelator xml config file*/
class Walker {
    public:
        void parsePlace(pugi::xml_node node, string parent);
        void traverseTree(pugi::xml_node node, string parent);
};

/** Singleton class holding map of all places.*/
class TreeCorrelator {
    public:
        /** Returns only instance of TreeCorrelator class.*/
        static TreeCorrelator* get();

        /** Create place or add existing place to the tree. Accepts
         * map <string, string> to pass place's parameters. The map should
         * consist of the following keys:
         * "parent" - name of a parent place or "root" if there is no parent
         * "name" - name of the place to be added or created, if the name
         *          follow a pattern "aaa_bbb_X-Y" where X, Y are integer
         *          numbers, it is understood a as range of places, e.g.
         *          ge_clover_high_0-3
         *          will be parsed as ge_clover_high_0, ge_clover_high_1,
         *          ge_clover_high_2, ge_clover_high_3
         *          with all the other parametrs common for these four places
         *          The primary use of this synatax is to include basic places
         *          (automatically created) in the tree.
         *  "type" - must be of one of existing Places type, currently these
         *           are ("PlaceDetector", "PlaceThreshold", "PlaceThresholdOR,
         *           "PlaceCounter", "PlaceOR", "PlaceAND").
         *
         *           If left empty, it will be assumed
         *           that this place exists already.
         *  "reset" - optional, "true" by default, sets place resetable ("true")
         *           or non-resetable ("false").
         *  "coincidence" - optional, "true" by default, sets relation with the
         *          parent place to coin. ("true") or anti-coin. ("false")
         *  "fifo"  - optional, "2" by default, sets the depth of place's fifo
         *          queue
         *  "range_min" - requaried for PlaceThreshold, sets the lower
         *              threshold
         *  "range_max" - requaried for PlaceThreshold, sets the high
         *              threshold
         */
        void createPlace(map<string, string>& params,
                         bool verbose = false);

        /** Add child to place parent with coincidence coin.*/
        void addChild(string parent, string child, bool coin = true,
                      bool verbose = false);

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

        static PlaceBuilder builder;
        /*Temprorary solution for choosing experiment*/
        void buildTree_LeRIBSS();
        void buildTree_Hybrid();

        /** Splits name string into the vector of string. Assumes that if
         * the last token (delimiter being "_") is in format "X-Y" where
         * X, Y are integers, the X and Y are range of base names to be retured
         * E.g. abc_1-2 will return ["abc_1", "abc_2"]. If no range token is
         * found, the name itself is returned as a only element of the vector*/
        vector<string> split_names(string name);
};

#endif
