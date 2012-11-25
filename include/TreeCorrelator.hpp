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

        /** Return pointer to place or throw exception if doesn't exist. */
        Place* place(string name);

        /** Create place, alter or add existing place to the tree. */
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
        map<string, Place*> places_;
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
