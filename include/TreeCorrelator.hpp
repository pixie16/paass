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

        /*Temprorary solution for choosing experiment*/
        void buildTree_LeRIBSS();
        void buildTree_Hybrid();

        double string_to_double (string s) { 
            istringstream iss(s);
            double value;
            if (!(iss >> value)) {
                stringstream ss;
                ss << "Could not convert string '" << s << "' to double" << endl;
                throw GeneralException(ss.str());
            }
            return value;
        }

        int string_to_int (string s) {
            istringstream iss(s);
            int value;
            if (!(iss >> value)) {
                stringstream ss;
                ss << "Could not convert string '" << s << "' to int" << endl;
                throw GeneralException(ss.str());
            }
            return value;
        }

        bool string_to_bool (string s) {
            if (s == "true" || s == "True" || s == "1")
                return true;
            else if (s == "false" || s == "False" || s == "0")
                return false;
            else {
                stringstream ss;
                ss << "Could not convert string '" << s << "' to bool" << endl;
                throw GeneralException(ss.str());
            }
        }

        vector<string> tokenize(string str, string delimiter);
};

#endif
