/** \file TreeCorrelator.cpp
 * \brief A class to handle complex correlations between various processors
 *
 * This guy was originally named "NewCorrelator.cpp" it was renamed.
 * \author K. A. Miernik
 * \date August 19, 2012
 */
#include "TreeCorrelator.hpp"
#include "Globals.hpp"
#include "Exceptions.hpp"
#include "Messenger.hpp"

using namespace std;

void Walker::parsePlace(pugi::xml_node node, std::string parent, bool verbose) {
    map<string, string> params;
    params["parent"] = parent;
    params["type"] = "";
    params["reset"] = "true";
    params["coincidence"] = "true";
    params["fifo"] = "2";
    params["init"] = "false";
    for (pugi::xml_attribute attr = node.first_attribute(); attr;
         attr = attr.next_attribute()) {
        params[attr.name()] = attr.value();
    }
    TreeCorrelator::get()->createPlace(params, verbose);
}

void Walker::traverseTree(pugi::xml_node node, std::string parent, bool verbose) {
    for (pugi::xml_node child = node.child("Place");
         child;
         child = child.next_sibling("Place")) {
        parsePlace(child, parent, verbose);
        traverseTree(child, string(child.attribute("name").value()), verbose);
    }
}

TreeCorrelator* TreeCorrelator::instance = NULL;

PlaceBuilder TreeCorrelator::builder = PlaceBuilder();

TreeCorrelator* TreeCorrelator::get() {
    if (!instance) {
        instance = new TreeCorrelator();
    }
    return instance;
}

Place* TreeCorrelator::place(std::string name) {
    map<string, Place*>::iterator element = places_.find(name);
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

vector<string> TreeCorrelator::split_names(std::string name) {
    vector<string> names;
    vector<string> name_tokens = strings::tokenize(name, "_");

    if (name_tokens.size() > 1) {
        string base_name;
        for (vector<string>::iterator it = name_tokens.begin();
                it != name_tokens.end() - 1;
                ++it)
            base_name += (*it) + "_";

        if (name_tokens.back().find("-") != string::npos ||
                name_tokens.back().find(",") != string::npos) {
            vector<string> comma_token = strings::tokenize(name_tokens.back(), ",");
            for (vector<string>::iterator itc = comma_token.begin();
                    itc != comma_token.end();
                    ++itc) {
                vector<string> range_tokens =
                                        strings::tokenize((*itc), "-");
                int range_min = strings::to_int(range_tokens[0]);
                int range_max = range_min;
                if (range_tokens.size() > 1)
                    range_max = strings::to_int(range_tokens[1]);

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

void TreeCorrelator::createPlace(std::map<std::string, std::string>& params,
                                 bool verbose) {
    bool replace = false;
    if (params["replace"] != "")
        replace = strings::to_bool(params["replace"]);

    vector<string> names = split_names(params["name"]);
    for (vector<string>::iterator it = names.begin();
         it != names.end();
         ++it) {

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
            Place* current = builder.create(params, verbose);
            places_[(*it)] = current;
            if (strings::to_bool(params["init"]))
                current->activate(0.0);
        }

        if (params["parent"] != "root") {
            bool coincidence = strings::to_bool(params["coincidence"]);
            addChild(params["parent"], (*it), coincidence, verbose);
        }

    }
}

void TreeCorrelator::buildTree() {
    pugi::xml_document doc;

    Messenger m;
    m.start("Creating TreeCorrelator");
    pugi::xml_parse_result result = doc.load_file("Config.xml");
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver: error parsing file Config.xml";
        ss << " : " << result.description();
        m.fail();
        throw IOException(ss.str());
    }

    pugi::xml_node tree = doc.child("Configuration").child("TreeCorrelator");
    bool verbose = tree.attribute("verbose").as_bool(false);

    Walker walker;
    walker.traverseTree(tree, string(tree.attribute("name").value()), verbose);

    m.done();
}

TreeCorrelator::~TreeCorrelator() {
    for (map<string, Place*>::iterator it = places_.begin();
         it != places_.end(); ++it) {
        delete it->second;
    }
    places_.clear();
    delete instance;
    instance = NULL;
}

