///@file DetectorDriverXmlParser.hpp
///@brief Class to parse the DetectorDriver node
///@author S. V. Paulauskas
///@date February 20, 2017
#ifndef PAASS_DETECTORDRIVERXMLPARSER_HPP
#define PAASS_DETECTORDRIVERXMLPARSER_HPP

#include <set>
#include <sstream>

#include "pugixml.hpp"

#include "DetectorDriver.hpp"
#include "Messenger.hpp"
#include "XmlParser.hpp"

///A class that handles parsing nodes necessary for the Globals class from
/// utkscan's configuration file.
class DetectorDriverXmlParser : public XmlParser {
public:
    ///Default Constructor
    DetectorDriverXmlParser() {}

    ///Default Destructor
    ~DetectorDriverXmlParser() {}

    ///Parses the provided node from an opened xml file.
    ///@param[in] node : The node that we are going to parse
    ///@throw invalid_argument if the node cannot be found.
    void ParseNode(DetectorDriver *driver);

    ///Returns the config option to have Detector Driver root output
    std::pair<bool,std::string> GetRootOutOpt(){ return SysRootOut;}

    ///Returns the Max Root Tree File size (In GB)
    double GetRFileSize(){return rFileSize; }

private:
    ///An instance of the messenger class so that we can output pretty info
    Messenger messenger_;

    ///A stringstream that we can use repeatedly without having to redefine.
    std::stringstream sstream_;

    ///Parses the list of processors from the configuration file.
    ///@param[in] node : The first processor node that we have.
    ///@return A vector containing pointers to the newly created classes
    std::vector<EventProcessor *> ParseProcessors(const pugi::xml_node &node);

    ///Parses the list of analyzers from the configuration file.
    ///@param[in] node : The first processor node that we have.
    ///@return A vector containing pointers to the newly created classes
    std::vector<TraceAnalyzer *> ParseAnalyzers(const pugi::xml_node &node);

    ///Prints all of the attributes for a node to the screen.
    ///@param[in] node : The node that we'd like to print the attirbutes for.
    void PrintAttributeMessage(pugi::xml_node &node);

    ///Controls whether or not to have system-wide root output from the Detector Driver.
    std::pair<bool,std::string> SysRootOut;

    double rFileSize;//!<Root File's roll over size.
};

#endif //PAASS_DETECTORDRIVERXMLPARSER_HPP
