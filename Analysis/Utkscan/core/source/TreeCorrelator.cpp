/** \file TreeCorrelator.cpp
 * \brief A class to handle complex correlations between various processors
 *
 * This guy was originally named "NewCorrelator.cpp" it was renamed.
 * \author K. A. Miernik
 * \date August 19, 2012
 */
#include "Exceptions.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "StringManipulationFunctions.hpp"
#include "TreeCorrelator.hpp"
#include "TreeCorrelatorXmlParser.hpp"

using namespace std;
using namespace StringManipulation;

TreeCorrelator *TreeCorrelator::instance = NULL;

PlaceBuilder TreeCorrelator::builder = PlaceBuilder();

TreeCorrelator *TreeCorrelator::get() {
    if (!instance) {
        instance = new TreeCorrelator();
    }
    return instance;
}

Place *TreeCorrelator::place(std::string name) {
    map<string, Place *>::iterator element = places_.find(name);
    if (element == places_.end()) {
        stringstream ss;
        ss << "TreeCorrelator: place " << name
           << " doesn't exist " << endl;
        throw TreeCorrelatorException(ss.str());
    }
    return element->second;
}

void TreeCorrelator::addChild(std::string parent, std::string child,
                              bool coin, bool verbose) {
    if (places_.count(parent) == 1 && places_.count(child) == 1) {
        place(parent)->addChild(place(child), coin);
        if (verbose) {
            Messenger m;
            stringstream ss;
            ss << "Setting " << child
               << " as a child of " << parent;
            m.detail(ss.str(), 1);
        }
    } else {
        stringstream ss;
        ss << "TreeCorrelator: could not set " << child
           << " as a child of " << parent << endl;
        throw TreeCorrelatorException(ss.str());
    }
}

void TreeCorrelator::createPlace(std::map <std::string, std::string> &params,
                                 bool verbose) {
    bool replace = false;
    if (params["replace"] != "")
        replace = StringToBool(params["replace"]);

    vector <string> names = split_names(params["name"]);
    for (vector<string>::iterator it = names.begin(); it != names.end(); ++it) {

        if (params["type"] != "") {
            if (replace) {
                if (places_.count((*it)) != 1) {
                    stringstream ss;
                    ss << "TreeCorrelator: cannot replace Place " << (*it)
                       << ", it doesn't exist";
                    throw TreeCorrelatorException(ss.str());
                }
                delete places_[(*it)];
                if (verbose) {
                    Messenger m;
                    stringstream ss;
                    ss << "Replacing place " << (*it);
                    m.detail(ss.str(), 1);
                }
            } else {
                if (places_.count((*it)) == 1) {
                    stringstream ss;
                    ss << "TreeCorrelator: place" << (*it) << " already exists";
                    throw TreeCorrelatorException(ss.str());
                }
                if (verbose) {
                    Messenger m;
                    stringstream ss;
                    ss << "Creating place " << (*it);
                    m.detail(ss.str(), 1);
                }
            }
            Place *current = builder.create(params, verbose);
            places_[(*it)] = current;
            if (StringToBool(params["init"]))
                current->activate(0.0);
        }

        if (params["parent"] != "root") {
            bool coincidence = StringToBool(params["coincidence"]);
            addChild(params["parent"], (*it), coincidence, verbose);
        }
    }
}

void TreeCorrelator::buildTree() {
    Messenger m;
    m.start("Growing TreeCorrelator");

    TreeCorrelatorXmlParser parser;
    parser.ParseNode(this);

    m.done();
}

vector <string> TreeCorrelator::split_names(std::string name) {
    vector <string> names;
    vector <string> name_tokens = TokenizeString(name, "_");

    if (name_tokens.size() > 1) {
        string base_name;
        for (vector<string>::iterator it = name_tokens.begin();
             it != name_tokens.end() - 1;
             ++it)
            base_name += (*it) + "_";

        if (name_tokens.back().find("-") != string::npos ||
            name_tokens.back().find(",") != string::npos) {
            vector <string> comma_token =
                    TokenizeString(name_tokens.back(), ",");
            for (vector<string>::iterator itc = comma_token.begin();
                 itc != comma_token.end();
                 ++itc) {
                vector <string> range_tokens = TokenizeString((*itc), "-");
                int range_min = stoi(range_tokens[0]);
                int range_max = range_min;
                if (range_tokens.size() > 1)
                    range_max = stoi(range_tokens[1]);

                for (int i = range_min; i <= range_max; ++i) {
                    stringstream ss;
                    ss << i;
                    names.push_back(base_name + ss.str());
                }
            }
        } else {
            names.push_back(base_name + name_tokens.back());
        }
    } else {
        names.push_back(name);
    }
    return names;
}

TreeCorrelator::~TreeCorrelator() {
    for (map<string, Place *>::iterator it = places_.begin();
         it != places_.end(); ++it) {
        delete it->second;
    }
    places_.clear();
    delete instance;
    instance = NULL;
}

