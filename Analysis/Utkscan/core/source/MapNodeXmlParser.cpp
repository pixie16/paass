///@file MapNodeXmlParser.cpp
///@brief Class to parse the Map node in the utkscan configuration file
///@author S. V. Paulauskas
///@date February 09, 2017
#include <iostream>
#include <stdexcept>
#include <string>

#include "HelperFunctions.hpp"
#include "MapNodeXmlParser.hpp"
#include "StringManipulationFunctions.hpp"
#include "TreeCorrelator.hpp"
#include "XmlInterface.hpp"

using namespace std;

void MapNodeXmlParser::ParseNode(DetectorLibrary *lib){
    pugi::xml_node map =
            XmlInterface::get()->GetDocument()->child("Configuration").child("Map");

    bool isVerbose = map.attribute("verbose").as_bool(false);
    bool isVerboseTree =
            XmlInterface::get()->GetDocument()->child("Configuration").
                    child("Tree").attribute("verbose").as_bool(false);
    TreeCorrelator *tree = TreeCorrelator::get();

    Messenger m;
    m.start("Loading channels map");
    stringstream ss;

    /** These attributes have reserved meaning, all other
     * attributes of [Channel] are treated as tags */
    set<string> reserved = {"number", "type", "subtype", "location", "tags",
                            "firmware", "frequency"};

    for (pugi::xml_node module = map.child("Module"); module;
         module = module.next_sibling("Module")) {
        int module_number = module.attribute("number").as_int(-1);

        if (module_number < 0) {
            ss << "MapNodeXmlParser::ParseNode : User requested illegal "
                    "module number (" << module_number
               << ") in configuration file.";
            throw GeneralException(ss.str());
        }

        if(isVerbose) {
            ss << "Module " << module_number << ":";
            m.detail(ss.str());
            ss.str("");
        }

        for (pugi::xml_node channel = module.child("Channel"); channel;
             channel = channel.next_sibling("Channel")) {
            unsigned int ch_number =
                    channel.attribute("number").as_int(-1);

            if (ch_number < 0 || ch_number >= Pixie16::maximumNumberOfChannels ) {
                ss << "MapNodeXmlParser::ParseNode : Illegal channel number ("
                   << ch_number << ") in configuration file.";
                throw GeneralException(ss.str());
            }

            if ( lib->HasValue(module_number, ch_number) ) {
                ss << "MapNodeXmlParser::ParseNode : Module " << module_number
                   << ", Channel " << ch_number << " is initialized more than once";
                throw GeneralException(ss.str());
            }

            Identifier id;

            string ch_type = channel.attribute("type").as_string("ignore");
            id.SetType(ch_type);

            string ch_subtype = channel.attribute("subtype").as_string("ignore");
            id.SetSubtype(ch_subtype);

            int ch_location = channel.attribute("location").as_int(-1);

            if (ch_location == -1)
                ch_location = lib->GetNextLocation(ch_type, ch_subtype);

            id.SetLocation(ch_location);

            if(isVerbose) {
                ss << "Channel " << ch_number  << " = " << ch_type << ":"
                   << ch_subtype << ", Location = "
                   << ch_location;
            }

            string ch_tags = channel.attribute("tags").as_string("None");
            if(ch_tags != "None"){
                ss << ", tags = " << ch_tags;
                vector<string> tagList =
                        StringManipulation::TokenizeString(ch_tags, ",");
                for(unsigned int i = 0; i < tagList.size(); i++)
                    id.AddTag(tagList[i], 1);
            }

            lib->Set(module_number, ch_number, id);

            /** Create basic place for TreeCorrelator */
            std::map <string, string> params;
            params["name"] = id.GetPlaceName();
            params["parent"] = "root";
            params["type"] = "PlaceDetector";
            params["reset"] = "true";
            params["fifo"] = "2";
            params["init"] = "false";
            tree->createPlace(params, isVerboseTree);

            if (isVerbose) {
                m.detail(ss.str(), 1);
                ss.str("");
            }
        }//end loop over channels
    }//end loop over modules
    m.done();
}

///This method simply warns the user of extraneous information that was
/// contained in the node.
void MapNodeXmlParser::WarnOfUnknownChildren(const pugi::xml_node &node,
                                      const set<string> &knownChildren) {
    for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
        if (knownChildren.find(it->name()) == knownChildren.end()) {
            sstream_ << "Unknown parameter in " << it->path()
                     << ".  This information is ignored by the program.";
            messenger_.detail(sstream_.str());
            sstream_.str("");
        }
}

