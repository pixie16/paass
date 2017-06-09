///@file TreeCorrelatorXmlParser.cpp
///@brief Class that handles parsing the TreeCorrelator node
///@author S. V. Paulauskas, K. Miernik
///@date February 20, 2017

#include "StringManipulationFunctions.hpp"
#include "TreeCorrelatorXmlParser.hpp"
#include "XmlInterface.hpp"

using namespace std;
using namespace StringManipulation;

void TreeCorrelatorXmlParser::ParseNode(TreeCorrelator *tree) {
    pugi::xml_node node = XmlInterface::get()->GetDocument()->child("Configuration").child("TreeCorrelator");
    bool verbose = node.attribute("verbose").as_bool(false);
    TraverseNode(node, string(node.attribute("name").value()), verbose);
}

void TreeCorrelatorXmlParser::ParsePlace(pugi::xml_node node, std::string parent, bool verbose) {
    map <string, string> params;
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
    ///@TODO This is a little clunky since we have the tree in the main
    /// method. This should be cleaned up at some point.
    TreeCorrelator::get()->createPlace(params, verbose);
}

void
TreeCorrelatorXmlParser::TraverseNode(pugi::xml_node node, std::string parent, bool verbose) {
    for (pugi::xml_node child = node.child("Place"); child; child = child.next_sibling("Place")) {
        ParsePlace(child, parent, verbose);
        TraverseNode(child, string(child.attribute("name").value()), verbose);
    }
}
