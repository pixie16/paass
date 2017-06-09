///@file MapNodeXmlParser.cpp
///@brief Class to parse the Map node in the utkscan configuration file
///@author S. V. Paulauskas, D. Miller, K. Miernik
///@date February 09, 2017
#include <iostream>

#include <cstring>

#include "DefaultConfigurationValues.hpp"
#include "HelperFunctions.hpp"
#include "MapNodeXmlParser.hpp"
#include "StringManipulationFunctions.hpp"
#include "TreeCorrelator.hpp"
#include "XmlInterface.hpp"

using namespace std;

void MapNodeXmlParser::ParseNode(DetectorLibrary *lib) {
    pugi::xml_node map = XmlInterface::get()->GetDocument()->child("Configuration").child("Map");

    if (!map)
        throw invalid_argument("MapNodeXmlParser::ParseNode : The Map node could not be read! This is fatal.");

    bool isVerbose = map.attribute("verbose").as_bool(false);
    bool isVerboseTree =
            XmlInterface::get()->GetDocument()->child("Configuration").child("Tree").attribute("verbose").as_bool(
                    false);
    TreeCorrelator *tree = TreeCorrelator::get();

    messenger_.start("Loading channels map");

    //These attributes have reserved meaning, all other attributes of [Channel] are treated as tags
    set<string> reserved = {"number", "type", "subtype", "location", "tags", "firmware", "frequency"};

    for (pugi::xml_node module = map.child("Module"); module; module = module.next_sibling("Module")) {

        int module_number = module.attribute("number").as_int(-1);

        if (module_number < 0) {
            sstream_ << "MapNodeXmlParser::ParseNode : User requested illegal module number (" << module_number
                     << ") in configuration file.";
            throw GeneralException(sstream_.str());
        }

        if (isVerbose) {
            sstream_ << "Module " << module_number << ":";
            messenger_.detail(sstream_.str());
            sstream_.str("");
        }

        for (pugi::xml_node channel = module.child("Channel"); channel; channel = channel.next_sibling("Channel")) {
            unsigned int channelNumber = channel.attribute("number").as_uint(Pixie16::maximumNumberOfChannels);

            if (channelNumber >= Pixie16::maximumNumberOfChannels) {
                sstream_ << "MapNodeXmlParser::ParseNode : Illegal channel " << "number (" << channelNumber
                         << ") in configuration file.";
                throw GeneralException(sstream_.str());
            }

            if (lib->HasValue(module_number, channelNumber)) {
                sstream_ << "MapNodeXmlParser::ParseNode : Module " << module_number << ", Channel " << channelNumber
                         << " is initialized more than once";
                throw GeneralException(sstream_.str());
            }

            ChannelConfiguration chanCfg;

            chanCfg.SetType(channel.attribute("type").as_string("ignore"));
            chanCfg.SetSubtype(channel.attribute("subtype").as_string("ignore"));

            if (channel.attribute("location").as_int(-1) == -1)
                chanCfg.SetLocation(lib->GetNextLocation(chanCfg.GetType(), chanCfg.GetSubtype()));
            else
                chanCfg.SetLocation(channel.attribute("location").as_uint());

            if (isVerbose)
                sstream_ << "Channel " << channelNumber << " = " << chanCfg.GetType() << ":" << chanCfg.GetSubtype()
                         << ", Location = " << chanCfg.GetLocation();

            string ch_tags = channel.attribute("tags").as_string("None");
            if (ch_tags != "None") {
                sstream_ << ", tags = " << ch_tags;
                vector<string> tagList = StringManipulation::TokenizeString(ch_tags, ",");
                for (unsigned int i = 0; i < tagList.size(); i++)
                    chanCfg.AddTag(tagList[i]);
            }

            if (isVerbose) {
                messenger_.detail(sstream_.str(), 1);
                sstream_.str("");
            }

            if (channel.child("Calibration").text())
                ParseCalibrations(channel.child("Calibration"), chanCfg, isVerbose);
            else if (isVerbose)
                messenger_.detail("This channel has no calibration associated with it.", 2);

            if (channel.child("Cfd"))
                ParseCfdNode(channel.child("Cfd"), chanCfg, isVerbose);
            else if (isVerbose)
                messenger_.detail("Using default CFD settings for this channel.", 2);

            if (channel.child("Filter"))
                ParseFilterNode(channel.child("Filter"), chanCfg, isVerbose);
            else if (isVerbose)
                messenger_.detail("Using default filter settings for this channel.", 2);

            if (channel.child("Fit"))
                ParseFittingNode(channel.child("Fit"), chanCfg, isVerbose);
            else if (isVerbose)
                messenger_.detail("Using default fitter settings for this channel.", 2);

            if (channel.child("Trace"))
                ParseTraceNode(channel.child("Trace"), chanCfg, isVerbose);
            else if (isVerbose)
                messenger_.detail("Using default trace settings for this channel.", 2);

            if (channel.child("Walk").text())
                ParseCalibrations(channel.child("Walk"), chanCfg, isVerbose);
            else if (isVerbose)
                messenger_.detail("This channel is not walk corrected.", 2);

            lib->Set(module_number, channelNumber, chanCfg);

            //Create basic place for TreeCorrelator
            std::map<string, string> params;
            params["name"] = chanCfg.GetPlaceName();
            params["parent"] = "root";
            params["type"] = "PlaceDetector";
            params["reset"] = "true";
            params["fifo"] = "2";
            params["init"] = "false";
            tree->createPlace(params, isVerboseTree);
        }//end loop over channels
    }//end loop over modules

    lib->SetCalibrations(calibrations_);
    lib->SetWalkCorrection(walkCorrector_);
    messenger_.done();
}

void MapNodeXmlParser::ParseCalibrations(const pugi::xml_node &node, const ChannelConfiguration &cfg,
                                         const bool &isVerbose) {
    for (pugi::xml_node cal = node; cal; cal = cal.next_sibling(node.name())) {
        string model = cal.attribute("model").as_string("None");
        double min = cal.attribute("min").as_double(0);
        double max = cal.attribute("max").as_double(numeric_limits<double>::max());

        stringstream pars(cal.text().as_string());
        vector<double> parameters;
        while (true) {
            double p;
            pars >> p;
            if (pars)
                parameters.push_back(p);
            else
                break;
        }

        if (isVerbose) {
            sstream_ << node.name() << " Model : " << model << " from [" << min << "," << max << "] ";
            if (!parameters.empty()) {
                sstream_ << "with coefficients :";
                for (vector<double>::iterator itPars = parameters.begin(); itPars != parameters.end(); ++itPars)
                    sstream_ << " " << (*itPars);
            }
            messenger_.detail(sstream_.str(), 2);
            sstream_.str("");
        }

        if (strcmp(node.name(), "Calibration") == 0)
            calibrations_.AddChannel(cfg, model, min, max, parameters);
        if (strcmp(node.name(), "Walk") == 0)
            walkCorrector_.AddChannel(cfg, model, min, max, parameters);
    }
}

///We parse the Cfd node here. This node contains the information necessary to the proper function of the various Cfd
/// timing codes. The only currently recognized node here is the Parameters node. If the Cfd node exists then the
/// Parameter node must also exist.
void MapNodeXmlParser::ParseCfdNode(const pugi::xml_node &node, ChannelConfiguration &config, const bool &isVerbose) {
        config.SetCfdParameters(make_tuple(
                node.child("Cfd").attribute("f").as_double(DefaultConfig::cfdF),
                node.child("Cfd").attribute("d").as_double(DefaultConfig::cfdD),
                node.child("Cfd").attribute("l").as_double(DefaultConfig::cfdL)));
}

///This method parses the fitting node. There are only two free parameters at the moment. The main part of this node
/// is the fitting parameters. These parameters are critical to the function of the software. If the fitting node is
/// present then the Parameters node must also be.
void MapNodeXmlParser::ParseFittingNode(const pugi::xml_node &node, ChannelConfiguration &config,
                                        const bool &isVerbose) {
    config.SetFittingParameters(make_pair(node.attribute("beta").as_double(DefaultConfig::fitBeta),
                                          node.attribute("gamma").as_double(DefaultConfig::fitGamma)));
}

///This node parses the Trace node. This node contains all of the information necessary for the users to do trace
/// analysis. The only critical node here is the WaveformRange node. If the Trace node exists then this node must
/// also exist.
void MapNodeXmlParser::ParseTraceNode(const pugi::xml_node &node, ChannelConfiguration &config, const bool &isVerbose) {

    config.SetDiscriminationStartInSamples(node.attribute("DiscriminationStart").as_uint(DefaultConfig::discrimStart));
    config.SetBaselineThreshold(node.attribute("baselineThreshold").as_double(DefaultConfig::baselineThreshold));
    config.SetWaveformBoundsInSamples(make_pair(node.attribute("RangeLow").as_int(DefaultConfig::waveformLow),
                                                node.attribute("RangeHigh").as_int(DefaultConfig::waveformHigh)));
    if(!node.attribute("delay").empty())
        config.SetTraceDelayInSamples(node.attribute("delay").as_uint());
    else
        throw invalid_argument("The Trace node must have the \"delay\" attribute!!");
}

///This node parses the Trace node. This node contains all of the information
/// necessary for the users to do trace analysis. The only critical node here
/// is the WaveformRange node. If the Trace node exists then this node must
/// also exist.
void MapNodeXmlParser::ParseFilterNode(const pugi::xml_node &node, ChannelConfiguration &config,
                                       const bool &isVerbose) {
    config.SetTriggerFilterParameters(TrapFilterParameters(
            node.child("Trigger").attribute("l").as_double(DefaultConfig::filterL),
            node.child("Trigger").attribute("g").as_double(DefaultConfig::filterG),
            node.child("Trigger").attribute("t").as_double(DefaultConfig::filterT)));
    config.SetEnergyFilterParameters(TrapFilterParameters(
            node.child("Energy").attribute("l").as_double(DefaultConfig::filterL),
            node.child("Energy").attribute("g").as_double(DefaultConfig::filterG),
            node.child("Energy").attribute("t").as_double(DefaultConfig::filterT)));
}