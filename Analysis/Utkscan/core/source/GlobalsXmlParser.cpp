///@file GlobalsXmlParser.cpp
///@brief Class to parse nodes needed for the Global's class from utkscan's configuration file
///@author S. V. Paulauskas, K. Miernik
///@date February 09, 2017
#include <iostream>
#include <stdexcept>
#include <string>

#include "HelperFunctions.hpp"
#include "GlobalsXmlParser.hpp"
#include "TrapFilterParameters.hpp"
#include "XmlInterface.hpp"

using namespace std;

void GlobalsXmlParser::ParseNode(Globals *globals) {
    const pugi::xml_node root = XmlInterface::get()->GetDocument()->child("Configuration");

    if (!root)
        throw invalid_argument("The root node \"/Configuration\" does not exist. No configuration can be loaded.");
    else
        ParseRootNode(root);

    stringstream ss;
    Messenger m;

    m.start("Loading Globals");

    m.detail("Experiment Summary : " + ParseDescriptionNode(root.child("Description")));

    try {
        if (root.child("Global")) {
            m.start("Loading Global Node");
            ParseGlobalNode(root.child("Global"), globals);
            m.done();
        } else
            throw GeneralException("Globals::Globals : We are missing the Globals node!");

        if (root.child("Reject")) {
            m.start("Loading Reject Node");
            globals->SetRejectionRegions(ParseRejectNode(root.child("Reject")));
            m.done();
        }

        if (root.child("Vandle")) {
            m.start("Loading Vandle Node");
            ParseVandleNode(root.child("Vandle"), globals);
            m.done();
        }
    } catch (exception &e) {
        m.detail("Globals::Globals : Exception caught while parsing configuration file.");
        throw;
    }
    m.done();
}

///This method parses the Description node. The description node contains
/// information related to the experiment. This node can contain any text
/// that the user thinks could be useful to know when the program starts up.
string GlobalsXmlParser::ParseDescriptionNode(const pugi::xml_node &node) {
    return node.text().get();
}

///This method parses the Global node. This node contains some of the basic
/// information about the analysis. All of the nodes with the exception of
/// the HasRaw node are critical nodes. They must always be present for the
/// analysis to work properly.
void GlobalsXmlParser::ParseGlobalNode(const pugi::xml_node &node, Globals *globals) {
    if (!node.child("Revision").empty()) {
        string revision = node.child("Revision").attribute("version").as_string();

        if (revision == "A" || revision == "D") {
            globals->SetAdcClockInSeconds(10e-9);
            globals->SetClockInSeconds(10e-9);
            globals->SetFilterClockInSeconds(10e-9);
        } else if (revision == "F") {
            globals->SetAdcClockInSeconds(4e-9);
            globals->SetClockInSeconds(8e-9);
            globals->SetFilterClockInSeconds(8e-9);
        } else {
            throw invalid_argument("GlobalsXmlParser::ParseGlobal - The revision \"" + revision +
                                           "\", is not known to us. Known revisions are A, D, F");
        }
        messenger_.detail("Revision : " + revision);
    } else
        throw invalid_argument(CriticalNodeMessage("Revision"));

    if (!node.child("EventWidth").empty()) {
        double eventLengthInSeconds = Conversions::ConvertSecondsWithPrefix(
                node.child("EventWidth").attribute("value").as_double(0),
                node.child("EventWidth").attribute("unit").as_string("None"));
        globals->SetEventLengthInSeconds(eventLengthInSeconds);
        globals->SetEventLengthInTicks((unsigned int) (eventLengthInSeconds / globals->GetClockInSeconds()));
        sstream_ << "Event width: " << eventLengthInSeconds * 1e6 << " us" << ", i.e. "
                 << eventLengthInSeconds / globals->GetClockInSeconds() << " pixie16 clock ticks.";
        messenger_.detail(sstream_.str());
        sstream_.str("");
    } else
        throw invalid_argument(CriticalNodeMessage("EventWidth"));

    if (!node.child("HasRaw").empty())
        globals->SetHasRawHistogramsDefined(node.child("HasRaw").attribute("value").as_bool(true));
    else
        globals->SetHasRawHistogramsDefined(true);

    set <string> knownNodes = {"Revision", "EventWidth", "HasRaw"};
    WarnOfUnknownChildren(node, knownNodes);
}

///This method parses the Reject node. The rejection regions are regions of
/// the data files that the user would like to ignore. These rejection
/// regions must be entered with units of seconds.
vector<pair<unsigned int, unsigned int>> GlobalsXmlParser::ParseRejectNode(const pugi::xml_node &node) {
    vector<pair<unsigned int, unsigned int>> regions;
    for (pugi::xml_node time = node.child("Time"); time; time = time.next_sibling("Time")) {
        int start = time.attribute("start").as_int(0);
        int end = time.attribute("end").as_int(0);

        stringstream ss;
        if ((start == 0 && end == 0) || start > end) {
            ss << "Globals: incomplete or wrong rejection region " << "declaration: " << start << ", " << end;
            throw invalid_argument(ss.str());
        }

        ss << "Rejection region: " << start << " to " << end << " s";
        messenger_.detail(ss.str());
        regions.push_back(make_pair(start, end));
    }
    sstream_ << "Total Number of Rejection Regions : " << regions.size();
    messenger_.detail(sstream_.str());
    sstream_.str("");
    return regions;
}

///This method parses the Root node. For utkscan the root node is always
/// named Configuration. We throw invalid_arguments if we do not find a few
/// of the critial nodes.
void GlobalsXmlParser::ParseRootNode(const pugi::xml_node &node) {
    set <string> knownChildren = {"Author", "Description", "Global", "DetectorDriver", "Map", "Vandle",
                                  "TreeCorrelator", "TimeCalibration", "Reject", "Notebook"};
    if (node.child("Map").empty())
        throw invalid_argument(CriticalNodeMessage("Map"));
    if (node.child("Global").empty())
        throw invalid_argument(CriticalNodeMessage("Global"));

    if(!node.child("Trace").empty())
        throw invalid_argument("GlobalsXmlParser::ParseRootNode - The Trace node is now channel specific.");
    if(!node.child("Fitting").empty())
        throw invalid_argument("GlobalsXmlParser::ParseRootNode - The Fitting node is now channel specific and called"
                                       " \"Fit.\"");
    if(!node.child("Timing").empty())
        throw invalid_argument("GlobalsXmlParser::ParseRootNode - The Trace node is deprecated.");
    if(!node.child("Physical").empty())
        throw invalid_argument("GlobalsXmlParser::ParseRootNode - The Physical node is deprecated");
    if(!node.child("Cfd").empty())
        throw invalid_argument("GlobalsXmlParser::ParseRootNode - The Cfd node is now channel specific.");

    WarnOfUnknownChildren(node, knownChildren);
}

///Parses the Vandle node. This node contains some information that is
/// specific to the analysis and function of VANDLE detectors.
void GlobalsXmlParser::ParseVandleNode(const pugi::xml_node &node, Globals *globals) {
    if (!node.child("SpeedOfLightBig").empty())
        globals->SetVandleBigSpeedOfLight(node.child("SpeedOfLightBig").attribute("value").as_double());
    else
        globals->SetVandleBigSpeedOfLight(15.22998);

    sstream_ << "Speed of Light in Big Bars : " << globals->GetVandleBigSpeedOfLightInCmPerNs() << " cm/ns";
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("SpeedOfLightMedium").empty())
        globals->SetVandleMediumSpeedOfLight(node.child("SpeedOfLightMedium").attribute("value").as_double());
    else
        globals->SetVandleMediumSpeedOfLight(15.5);

    sstream_ << "Speed of Light in Medium Bars : " << globals->GetVandleMediumSpeedOfLightInCmPerNs() << " cm/ns";
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("SpeedOfLightSmall").empty())
        globals->SetVandleSmallSpeedOfLight(node.child("SpeedOfLightSmall").attribute("value").as_double());
    else
        globals->SetVandleSmallSpeedOfLight(12.65822);

    sstream_ << "Speed of Light in Small Bars : " << globals->GetVandleSmallSpeedOfLightInCmPerNs() << " cm/ns";
    messenger_.detail(sstream_.str());
    sstream_.str("");

    set <string> knownNodes = {"SpeedOfLightBig", "SpeedOfLightMedium", "SpeedOfLightSmall"};
    WarnOfUnknownChildren(node, knownNodes);
}

///This method simply warns the user of extraneous information that was contained in the node.
void GlobalsXmlParser::WarnOfUnknownChildren(const pugi::xml_node &node, const set <string> &knownChildren) {
    for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
        if (knownChildren.find(it->name()) == knownChildren.end()) {
            sstream_ << "Unknown parameter in " << it->path() << ".  This information is ignored by the program.";
            messenger_.detail(sstream_.str());
            sstream_.str("");
        }
}