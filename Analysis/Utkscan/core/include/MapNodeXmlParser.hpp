///@file MapNodeXmlParser.hpp
///@brief Class to parse the Map node.
///@author S. V. Paulauskas
///@date February 09, 2017
#ifndef PAASS_MAPNODEXMLPARSER_HPP
#define PAASS_MAPNODEXMLPARSER_HPP

#include <set>
#include <sstream>

#include "pugixml.hpp"

#include "Calibrator.hpp"
#include "ChannelConfiguration.hpp"
#include "DetectorLibrary.hpp"
#include "Messenger.hpp"
#include "WalkCorrector.hpp"
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
    ///An instance of the messenger class so that we can output pretty info
    Messenger messenger_;

    ///A stringstream that we can use repeatedly without having to redefine.
    std::stringstream sstream_;

    ///A calibrator object that holds the calibrations that we parsed out of
    /// the file.
    Calibrator calibrations_;

    ///A WalkCorrector object that contains the parsed walk corrections.
    WalkCorrector walkCorrector_;

    ///Method to parse the Calibration node from the XML file.
    ///@param[in] node : The calibration node that we want to parse
    ///@param[in] config : The channel configuration that we will use to hold all this info.
    ///@param[in] isVerbose : True if we want verbose messaging
    void ParseCalibrations(const pugi::xml_node &node, const ChannelConfiguration &config, const bool &isVerbose);

    ///Parses the Cfd node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    ///@throw invalid_argument if the Parameters node cannot be found.
    void ParseCfdNode(const pugi::xml_node &node, ChannelConfiguration &config, const bool &isVerbose);

    ///Parses the Fitting node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    ///@throw invalid_argument if the Parameters node cannot be found.
    void ParseFittingNode(const pugi::xml_node &node, ChannelConfiguration &config, const bool &isVerbose);

    ///Parses the Trace node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the
    /// values that we need.
    ///@throw invalid_argument If the WaveformRange is missing
    void ParseTraceNode(const pugi::xml_node &node, ChannelConfiguration &config, const int &mod_freq,const double &mod_TD
            ,const bool &isVerbose);

    ///Parses the Trace node from the xml configuration file.
    ///@param[in] node : The node that we are going to parse
    ///@param[in] globals : A pointer to the globals class so we can set the values that we need.
    ///@throw invalid_argument If the WaveformRange is missing
    void ParseFilterNode(const pugi::xml_node &node, ChannelConfiguration &config, const bool &isVerbose);
};

#endif //PAASS_MAPNODEXMLPARSER_HPP
