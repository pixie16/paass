///@file MapNodeXmlParser.hpp
///@brief Class to parse nodes needed for the Global's class from utkscan's
/// configuration file.
///@author S. V. Paulauskas
///@date February 09, 2017
#ifndef PAASS_XMLPARSER_HPP
#define PAASS_XMLPARSER_HPP

#include <set>
#include <sstream>

#include "pugixml.hpp"

#include "DetectorLibrary.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "XmlParser.hpp"

///A class that handles parsing nodes necessary for the Globals class from
/// utkscan's configuration file.
class MapNodeXmlParser : public XmlParser {
public:
    ///Default Constructor
    MapNodeXmlParser() {}

    ///Default Destructor
    ~MapNodeXmlParser() {}

    ///Parses the provided node from an opened xml file.
    ///@param[in] node : The node that we are going to parse
    ///@throw invalid_argument if the node cannot be found.
    void ParseNode(DetectorLibrary *lib);

private:
    //An instance of the messenger class so that we can output pretty info
    Messenger messenger_;

    //A stringstream that we can use repeatedly without having to redefine.
    std::stringstream sstream_;

    ///Warn that we have an unknown parameter in the node.
    ///@param [in] node : an iterator pointing to the location of the unknown
    ///@param [in] knownChildren: A list of the nodes that are known.
    void WarnOfUnknownChildren(const pugi::xml_node &node,
                               const std::set<std::string> &knownChildren);
};

#endif //PAASS_XMLPARSER_HPP
