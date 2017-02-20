///@file MapNodeXmlParser.cpp
///@brief Class to parse the Map node in the utkscan configuration file
///@author S. V. Paulauskas
///@date February 09, 2017
#include <iostream>

#include <cstring>

#include "HelperFunctions.hpp"
#include "MapNodeXmlParser.hpp"
#include "StringManipulationFunctions.hpp"
#include "TreeCorrelator.hpp"
#include "XmlInterface.hpp"

using namespace std;

void MapNodeXmlParser::ParseNode(DetectorLibrary *lib) {
    pugi::xml_node map =
            XmlInterface::get()->GetDocument()->child("Configuration").child(
                    "Map");

    if (!map)
        throw invalid_argument("MapNodeXmlParser::ParseNode : The map node "
                                       "could not be read! This is fatal.");

    bool isVerbose = map.attribute("verbose").as_bool(false);
    bool isVerboseTree =
            XmlInterface::get()->GetDocument()->child("Configuration").
                    child("Tree").attribute("verbose").as_bool(false);
    TreeCorrelator *tree = TreeCorrelator::get();

    messenger_.start("Loading channels map");

    //These attributes have reserved meaning, all other
    //attributes of [Channel] are treated as tags
    set<string> reserved = {"number", "type", "subtype", "location", "tags",
                            "firmware", "frequency"};

    for (pugi::xml_node module = map.child("Module"); module;
         module = module.next_sibling("Module")) {

        int module_number = module.attribute("number").as_int(-1);

        if (module_number < 0) {
            sstream_ << "MapNodeXmlParser::ParseNode : User requested illegal "
                    "module number (" << module_number
                     << ") in configuration file.";
            throw GeneralException(sstream_.str());
        }

        if (isVerbose) {
            sstream_ << "Module " << module_number << ":";
            messenger_.detail(sstream_.str());
            sstream_.str("");
        }

        for (pugi::xml_node channel = module.child("Channel"); channel;
             channel = channel.next_sibling("Channel")) {
            unsigned int channelNumber =
                    channel.attribute("number").as_uint(
                            Pixie16::maximumNumberOfChannels);

            if (channelNumber >= Pixie16::maximumNumberOfChannels) {
                sstream_ << "MapNodeXmlParser::ParseNode : Illegal channel "
                         << "number (" << channelNumber
                         << ") in configuration file.";
                throw GeneralException(sstream_.str());
            }

            if (lib->HasValue(module_number, channelNumber)) {
                sstream_ << "MapNodeXmlParser::ParseNode : Module "
                         << module_number << ", Channel " << channelNumber
                         << " is initialized more than once";
                throw GeneralException(sstream_.str());
            }

            Identifier id;

            id.SetType(channel.attribute("type").as_string("ignore"));
            id.SetSubtype(channel.attribute("subtype").as_string("ignore"));

            if (channel.attribute("location").as_int(-1) == -1)
                id.SetLocation(lib->GetNextLocation(id.GetType(),
                                                    id.GetSubtype()));
            else
                id.SetLocation(channel.attribute("location").as_uint());

            if (isVerbose) {
                sstream_ << "Channel " << channelNumber << " = " << id.GetType()
                         << ":" << id.GetSubtype() << ", Location = "
                         << id.GetLocation();
            }

            string ch_tags = channel.attribute("tags").as_string("None");
            if (ch_tags != "None") {
                sstream_ << ", tags = " << ch_tags;
                vector<string> tagList =
                        StringManipulation::TokenizeString(ch_tags, ",");
                for (unsigned int i = 0; i < tagList.size(); i++)
                    id.AddTag(tagList[i], 1);
            }

            if (isVerbose) {
                messenger_.detail(sstream_.str(), 1);
                sstream_.str("");
            }

            if (channel.child("Calibration").text()) {
                ParseCalibrations(channel.child("Calibration"), id, isVerbose);
            } else {
                if (isVerbose)
                    messenger_.detail("This channel is not calibrated.", 2);
            }

            if (channel.child("Walk").text()) {
                ParseCalibrations(channel.child("Walk"), id, isVerbose);
            } else {
                if (isVerbose)
                    messenger_.detail("This channel is not walk corrected.", 2);
            }

            lib->Set(module_number, channelNumber, id);

            //Create basic place for TreeCorrelator
            std::map<string, string> params;
            params["name"] = id.GetPlaceName();
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

void MapNodeXmlParser::ParseCalibrations(const pugi::xml_node &node,
                                         const Identifier &id,
                                         const bool &isVerbose) {
    for (pugi::xml_node cal = node; cal; cal = cal.next_sibling(node.name())) {
        string model = cal.attribute("model").as_string("None");
        double min = cal.attribute("min").as_double(0);
        double max =
                cal.attribute("max").as_double(numeric_limits<double>::max());

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
            sstream_ << node.name() << " Model : " << model << " from [" << min
                     << "," << max << "] ";
            if (!parameters.empty()) {
                sstream_ << "with coefficients :";
                for (vector<double>::iterator itPars = parameters.begin();
                     itPars != parameters.end(); ++itPars)
                    sstream_ << " " << (*itPars);
            }
            messenger_.detail(sstream_.str(), 2);
            sstream_.str("");
        }

        if (strcmp(node.name(), "Calibration") == 0)
            calibrations_.AddChannel(id, model, min, max, parameters);
        if (strcmp(node.name(), "Walk") == 0)
            walkCorrector_.AddChannel(id, model, min, max, parameters);
    }
}