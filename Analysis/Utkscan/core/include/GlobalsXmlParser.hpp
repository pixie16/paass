///@file GlobalsXmlParser.hpp
///@brief Class to parse nodes needed for the Global's class from utkscan's configuration file.
///@author S. V. Paulauskas
///@date February 09, 2017
#ifndef PAASS_XMLPARSER_HPP
#define PAASS_XMLPARSER_HPP

#include <set>
#include <sstream>

#include "Globals.hpp"
#include "Messenger.hpp"
#include "XmlParser.hpp"

///A class that handles parsing nodes necessary for the Globals class from
/// utkscan's configuration file.
class GlobalsXmlParser : public XmlParser {
public:
    ///Default Constructor
    GlobalsXmlParser() {}

    ///Default Destructor
    ~GlobalsXmlParser() {}

    ///Handles the main processing of the Globals node.
    ///@param[in] globals : Pointer to the globals class.
    void ParseNode(Globals *globals);

    ///Parses the Description node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@return The text that was contained in the node.
    std::string ParseDescriptionNode(const pugi::xml_node &node);

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
    std::vector<std::pair<unsigned int, unsigned int>> ParseRejectNode(const pugi::xml_node &node);

    ///Parses the Configuration node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@throw invalid_argument if the Map node is missing
    ///@throw invalid_argument if the Global node is missing
    void ParseRootNode(const pugi::xml_node &node);

    ///Parses the Vandle node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    void ParseVandleNode(const pugi::xml_node &node, Globals *globals);

    ///Warn that we have an unknown parameter in the node.
    ///@param [in] node : an iterator pointing to the location of the unknown
    ///@param [in] knownChildren: A list of the nodes that are known.
    void WarnOfUnknownChildren(const pugi::xml_node &node, const std::set<std::string> &knownChildren);

private:
    //An instance of the messenger class so that we can output pretty info
    Messenger messenger_;

    //A stringstream that we can use repeatedly without having to redefine.
    std::stringstream sstream_;

    // list of possible Rev:F frequencies. Inner Struct is <freq,<adc factor,filter factor >>
    // We will add the "e-9" on the set end for readability here
    std::vector<std::pair<int, std::pair<double, double>>> revFfreq = {{100, {10, 10}}, {250, {4, 8}}, {500, {2, 10}}};
};

#endif //PAASS_XMLPARSER_HPP
