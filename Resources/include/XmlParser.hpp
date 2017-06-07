/**@file XmlParser.hpp
 * @brief Class to help parse nodes XML based files.
 * @authors S. V. Paulauskas and T.T. King
 * @date February 09, 2017
 */
#ifndef _PAASS_XMLPARSER_HPP
#define _PAASS_XMLPARSER_HPP

#include <set>

#include "pugixml.hpp"

///A class that handles parsing XML nodes
class XmlParser {
public:
    ///Default Constructor
    XmlParser() {}

    ///Default Destructor
    virtual ~XmlParser() {}

    ///Parses the provided node from an opened xml file.
    ///@param[in] node : The node that we are going to parse
    ///@throw invalid_argument if the node cannot be found.
    virtual void ParseNode(const pugi::xml_node &node);

protected:
    ///Constructs a message string for the throws
    ///@param[in] name : The name of the node where we had the error
    ///@return The message that we want the throw to contain.
    std::string CriticalNodeMessage(const std::string &name);

    ///Constructs a custom critical error string for the throws
    ///@param[in] message : The error we want to throw
    ///@return The message that we want the throw to contain.
    std::string CriticalAttributeMessage(const std::string &message);

    ///Warn that we have an unknown parameter in the node.
    ///@param [in] node : an iterator pointing to the location of the unknown
    ///@param [in] knownChildren: A list of the nodes that are known.
    virtual void WarnOfUnknownChildren(const pugi::xml_node &node, const std::set<std::string> &knownChildren);
};

#endif //#ifdef _PAASS_XMLPARSER_HPP
