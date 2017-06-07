/**@file XmlParser.cpp
 * @brief An abstracted class that provides basic functionality of parsing an
 * XML node.
 * @authors S. V. Paulauskas and T.T. King
 * @date February 09, 2017
 */
#include <iostream>
#include <stdexcept>

#include "XmlParser.hpp"

using namespace std;

///We implement a basic node parser here. It will find all of the children of
/// the provide node and output the attributes of those children. This is
/// meant to be overloaded or replaced in the derived classes.
void XmlParser::ParseNode(const pugi::xml_node &node) {
    if (!node)
        throw invalid_argument("XmlParser::ParseNode - The provided node was invalid.");
    cout << "Parsing the " << node.name() << " node." << endl;
    for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it) {
        cout << "Found child named " << it->name() << " with attributes : ";
        for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
            cout << " " << ait->name() << " = " << ait->value();
        }
        cout << endl;
    }
}

///This method simply warns the user of extraneous information that was contained in the node.
void XmlParser::WarnOfUnknownChildren(const pugi::xml_node &node, const set<string> &knownChildren) {
    for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
        if (knownChildren.find(it->name()) == knownChildren.end())
            cout << "Unknown parameter in " << it->path() << ".  This information is ignored by the program." << endl;
}

///We simply return an error message that will be used to give the user
/// information about what went wrong.
string XmlParser::CriticalNodeMessage(const std::string &name) {
    return "XmlParser::ParseGlobal - We couldn't find the \"" + name + "\" node. This node is critical to operation.";
}

///We simply return an error message that will be used to give the user
/// information about what went wrong.
string XmlParser::CriticalAttributeMessage(const std::string &message) {
    return "XmlParser::ParseGlobal - \"" + message;
}
