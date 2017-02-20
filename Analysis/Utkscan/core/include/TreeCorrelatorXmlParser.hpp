///@file TreeCorrelatorXmlParser.hpp
///@brief Class that handles parsing the TreeCorrelator node
///@author S. V. Paulauskas
///@date February 20, 2017

#ifndef _PIXIESUITE_TREECORRELATORXMLPARSER_HPP
#define _PIXIESUITE_TREECORRELATORXMLPARSER_HPP

#include <string>
#include <vector>

#include "TreeCorrelator.hpp"
#include "XmlParser.hpp"

///Class to handle parsing the TreeCorrelator node
class TreeCorrelatorXmlParser : public XmlParser {
public:
    ///Default Constructor
    TreeCorrelatorXmlParser() {}

    ///Default Destructor
    ~TreeCorrelatorXmlParser() {}

    ///Parses the provided node from an opened xml file.
    ///@param[in] node : The node that we are going to parse
    ///@throw invalid_argument if the node cannot be found.
    void ParseNode(TreeCorrelator *tree);

private:
    ///An instance of the messenger class so that we can output pretty info
    Messenger messenger_;

    ///A stringstream that we can use repeatedly without having to redefine.
    std::stringstream sstream_;

    ///Parse a specific place
    ///@param [in] node : the xml node to parse
    ///@param [in] parent : the string of the parent node
    ///@param [in] verbose : verbosity
    void ParsePlace(pugi::xml_node node, std::string parent, bool verbose);

    ///Walks recursively through the tree
    ///@param [in] node : the xml node to parse
    ///@param [in] parent : the string of the parent node
    ///@param [in] verbose : verbosity
    void TraverseNode(pugi::xml_node node, std::string parent, bool verbose);
};

#endif //#ifdef PIXIESUITE_TREECORRELATORXMLPARSER_HPP
