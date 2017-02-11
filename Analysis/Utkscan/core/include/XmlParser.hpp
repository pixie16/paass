///@file XmlParser.hpp
///@brief Class to parse nodes from utkscan's configuration file.
///@author S. V. Paulauskas
///@date February 09, 2017
#ifndef PAASS_XMLPARSER_HPP
#define PAASS_XMLPARSER_HPP

#include <set>
#include <sstream>

#include "pugixml.hpp"

#include "Globals.hpp"
#include "Messenger.hpp"

///A class that handles parsing the different XML nodes in the utkscan
/// configuration file.
class XmlParser {
public:
    ///Default Constructor
    XmlParser() {}

    ///Default Destructor
    ~XmlParser() {}

    ///Parses the Cfd node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    ///@throw invalid_argument if the Parameters node cannot be found.
    void ParseCfdNode(const pugi::xml_node &node, Globals *globals);

    ///Parses the Description node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@return The text that was contained in the node.
    std::string ParseDescriptionNode(const pugi::xml_node &node);

    ///Parses the Fitting node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    ///@throw invalid_argument if the Parameters node cannot be found.
    void ParseFittingNode(const pugi::xml_node &node, Globals *globals);

    ///Parses the Global node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    ///@throw invalid_argument if the EventWidth is missing
    ///@throw invalid_argument if the OutputPath is missing
    ///@throw invalid_argument if the Revision is missing.
    void ParseGlobalNode(const pugi::xml_node &node, Globals *globals);

    ///Parses the Reject node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@return The vector containing all of the rejection regions
    std::vector<std::pair<unsigned int, unsigned int> > ParseRejectNode(
            const pugi::xml_node &node);

    ///Parses the Configuration node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@throw invalid_argument if the Map node is missing
    ///@throw invalid_argument if the Global node is missing
    void ParseRootNode(const pugi::xml_node &node);

    ///Parses the Trace node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    ///@throw invalid_argument If the WaveformRange is missing
    void ParseTraceNode(const pugi::xml_node &node, Globals *globals);

    ///Parses the Vandle node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    void ParseVandleNode(const pugi::xml_node &node, Globals *globals);

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

    ///Constructs a message string for the throws
    ///@param[in] name : The name of the node where we had the error
    ///@return The message that we want the throw to contain.
    std::string CriticalNodeMessage(const std::string &name);
};

#endif //PAASS_XMLPARSER_HPP
