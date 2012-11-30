#include "PathHolder.hpp"
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

Place* TreeCorrelator::place(string name) {
#ifdef DEBUG
    if (places_.count(name) == 0) {
        stringstream ss;
        ss << "TreeCorrelator: place " << name
           << " doesn't exist " << endl;
        throw TreeCorrelatorException(ss.str());
    }
#endif
    return places_[name];
}

void TreeCorrelator::addChild(string parent, string child, 
                             bool coin /* = true*/, bool verbose /*= false*/) {
    if (places_.count(parent) == 1 || places_.count(child) == 1) {
        place(parent)->addChild(place(child), coin);
        if (verbose) {
            cout << "TreeCorrelator: setting " << child 
                 << " as a child of " << parent << endl;
        }
    } else {
        stringstream ss;
        ss << "TreeCorrelator: could not set " << child
           << "as a child of " << parent << endl;
        throw TreeCorrelatorException(ss.str());
    }
}

vector<string> TreeCorrelator::split_names(string name) {
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

void TreeCorrelator::createPlace(map<string, string>& params,
                                 bool verbose /*= false*/) {
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
                    ss << "TreeCorrelator: cannot replace Place" << (*it) 
                       << ", it doesn't exist";
                    throw TreeCorrelatorException(ss.str());
                }
                delete places_[(*it)];
                if (verbose::CORRELATOR_INIT)
                    cout << "TreeCorrelator: replacing place " << (*it) << endl;
            } else {
                if (places_.count((*it)) == 1) {
                    stringstream ss;
                    ss << "TreeCorrelator: place" << (*it) << " already exists";
                    throw TreeCorrelatorException(ss.str());
                }
                if (verbose::CORRELATOR_INIT)
                    cout << "TreeCorrelator: creating place " << (*it) << endl;
            }
            Place* current = builder.create(params);
            places_[(*it)] = current;
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

    PathHolder* conf_path = new PathHolder();
    string xmlFileName = conf_path->GetFullPath("TreeCorrelator.xml");
    delete conf_path;

    pugi::xml_parse_result result = doc.load_file(xmlFileName.c_str());
    cout << "Loading configiration file " << xmlFileName << " : "
         << result.description() 
         << endl << "Configuration description: "
         << doc.child("TreeCorrelator").attribute("description").value()
         << endl;

    pugi::xml_node tree = doc.child("TreeCorrelator");
    Walker walker;
    walker.traverseTree(tree, string(tree.attribute("name").value()));
}

TreeCorrelator::~TreeCorrelator() {
    for (map<string, Place*>::iterator it = places_.begin(); 
         it != places_.end(); ++it) {
            if (verbose::MAP_INIT)
                cout << "TreeCorrelator: deleting place " << (*it).first << endl;
            delete it->second;
    }
}

