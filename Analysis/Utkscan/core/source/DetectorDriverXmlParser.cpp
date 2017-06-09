///@file DetectorDriverXmlParser.cpp
///@brief Class to parse the DetectorDriver node
///@author S. V. Paulauskas, S. Liddick, D. Miller, K. Miernik
///@date February 20, 2017
#include <iostream>

#include <cstring>

#include "HelperFunctions.hpp"
#include "DetectorDriverXmlParser.hpp"
#include "StringManipulationFunctions.hpp"
#include "TreeCorrelator.hpp"
#include "XmlInterface.hpp"

//These headers handle trace analysis
#include "CfdAnalyzer.hpp"
#include "FittingAnalyzer.hpp"
#include "TauAnalyzer.hpp"
#include "TraceAnalyzer.hpp"
#include "TraceExtractor.hpp"
#include "TraceFilterAnalyzer.hpp"
#include "WaaAnalyzer.hpp"
#include "WaveformAnalyzer.hpp"

//These headers handle processing of specific detector types
#include "BetaScintProcessor.hpp"
#include "CloverCalibProcessor.hpp"
#include "CloverProcessor.hpp"
#include "DoubleBetaProcessor.hpp"
#include "DssdProcessor.hpp"
#include "GeProcessor.hpp"
#include "Hen3Processor.hpp"
#include "ImplantSsdProcessor.hpp"
#include "IonChamberProcessor.hpp"
#include "LiquidScintProcessor.hpp"
#include "LitePositionProcessor.hpp"
#include "LogicProcessor.hpp"
#include "McpProcessor.hpp"
#include "NeutronScintProcessor.hpp"
#include "PositionProcessor.hpp"
#include "PspmtProcessor.hpp"
#include "TeenyVandleProcessor.hpp"
#include "TemplateProcessor.hpp"
#include "VandleProcessor.hpp"

//These headers are for handling experiment specific processing.
#include "E11027Processor.hpp"
#include "TemplateExpProcessor.hpp"
#include "VandleOrnl2012Processor.hpp"

#ifdef useroot //Some processors REQUIRE ROOT to function
#include "Anl1471Processor.hpp"
#include "IS600Processor.hpp"
#include "RootProcessor.hpp"
#include "TwoChanTimingProcessor.hpp"
#endif

using namespace std;

void DetectorDriverXmlParser::ParseNode(DetectorDriver *driver) {
    pugi::xml_node node =
            XmlInterface::get()->GetDocument()->
                    child("Configuration").child("DetectorDriver");

    if (!node)
        throw invalid_argument("DetectorDriverXmlParser::ParseNode : The detector driver node "
                                       "could not be read! This is fatal.");

    messenger_.start("Loading Analyzers");
    driver->SetTraceAnalyzers(ParseAnalyzers(node.child("Analyzer")));
    messenger_.done();

    messenger_.start("Loading Processors");
    driver->SetEventProcessors(ParseProcessors(node.child("Processor")));
    messenger_.done();
}

vector<EventProcessor *> DetectorDriverXmlParser::ParseProcessors(const pugi::xml_node &node) {
    std::vector < EventProcessor * > vecProcess;

    for (pugi::xml_node processor = node; processor;
         processor = processor.next_sibling(node.name())) {
        string name = processor.attribute("name").as_string();

        messenger_.detail("Loading " + name);
        if (name == "BetaScintProcessor") {
            vecProcess.push_back(new BetaScintProcessor(
                    processor.attribute("gamma_beta_limit").as_double(200.e-9),
                    processor.attribute("energy_contraction").as_double(1.0)));
        } else if (name == "CloverCalibProcessor") {
            vecProcess.push_back(new CloverCalibProcessor(
                    processor.attribute("gamma_threshold").as_double(1),
                    processor.attribute("low_ratio").as_double(1),
                    processor.attribute("high_ratio").as_double(3)));
        } else if (name == "CloverProcessor") {
            ///@TODO This needs to be cleaned. No method should have this
            /// many variables as arguments.
            vecProcess.push_back(new CloverProcessor(
                    processor.attribute("gamma_threshold").as_double(1.0),
                    processor.attribute("low_ratio").as_double(1.0),
                    processor.attribute("high_ratio").as_double(3.0),
                    processor.attribute("sub_event").as_double(100.e-9),
                    processor.attribute("gamma_beta_limit").as_double(200.e-9),
                    processor.attribute("gamma_gamma_limit").as_double(200.e-9),
                    processor.attribute("cycle_gate1_min").as_double(0.0),
                    processor.attribute("cycle_gate1_max").as_double(0.0),
                    processor.attribute("cycle_gate2_min").as_double(0.0),
                    processor.attribute("cycle_gate2_max").as_double(0.0)));
        } else if (name == "DoubleBetaProcessor") {
            vecProcess.push_back(new DoubleBetaProcessor());
        } else if (name == "DssdProcessor") {
            vecProcess.push_back(new DssdProcessor());
        } else if (name == "GeProcessor") {
            vecProcess.push_back(new GeProcessor());
        } else if (name == "Hen3Processor") {
            vecProcess.push_back(new Hen3Processor());
        } else if (name == "ImplantSsdProcessor") {
            vecProcess.push_back(new ImplantSsdProcessor());
        } else if (name == "IonChamberProcessor") {
            vecProcess.push_back(new IonChamberProcessor());
        } else if (name == "LiquidScintProcessor") {
            vecProcess.push_back(new LiquidScintProcessor());
        } else if (name == "LitePositionProcessor") {
            vecProcess.push_back(new LitePositionProcessor());
        } else if (name == "LogicProcessor") {
            vecProcess.push_back(new LogicProcessor());
        } else if (name == "McpProcessor") {
            vecProcess.push_back(new McpProcessor());
        } else if (name == "NeutronScintProcessor") {
            vecProcess.push_back(new NeutronScintProcessor());
        } else if (name == "PositionProcessor") {
            vecProcess.push_back(new PositionProcessor());
        } else if (name == "PspmtProcessor") {
            vecProcess.push_back(new PspmtProcessor());
        } else if (name == "TeenyVandleProcessor") {
            vecProcess.push_back(new TeenyVandleProcessor());
        } else if (name == "TemplateProcessor") {
            vecProcess.push_back(new TemplateProcessor());
        } else if (name == "VandleProcessor") {
            vecProcess.push_back(new VandleProcessor(
                    StringManipulation::TokenizeString(processor.attribute("types").as_string("medium"), ","),
                    processor.attribute("res").as_double(2.0), processor.attribute("offset").as_double(1000.0),
                    processor.attribute("NumStarts").as_uint(1), processor.attribute("compression").as_double(1.0)));
        } else if (name == "TemplateExpProcessor") {
            vecProcess.push_back(new TemplateExpProcessor());
        } else if (name == "E11027Processor") {
            vecProcess.push_back(new E11027Processor());
        } else if (name == "TemplateExpProcessor") {
            vecProcess.push_back(new TemplateExpProcessor());
        }
#ifdef useroot //Certain processors REQUIRE ROOT to actually work
        else if (name == "Anl1471Processor") {
            vecProcess.push_back(new Anl1471Processor());
        } else if (name == "IS600Processor") {
            vecProcess.push_back(new IS600Processor());
        } else if (name == "TwoChanTimingProcessor") {
            vecProcess.push_back(new TwoChanTimingProcessor());
        } else if (name == "VandleOrnl2012Processor") {
            vecProcess.push_back(new VandleOrnl2012Processor());
        } else if (name == "RootProcessor") { //Must be the last for silly reasons.
            vecProcess.push_back(new RootProcessor("tree.root", "tree"));
        }
#endif
        else {
            stringstream ss;
            ss << "DetectorDriverXmlParser: Unknown processor : " << name;
            throw GeneralException(ss.str());
        }

        PrintAttributeMessage(processor);
    }
    return vecProcess;
}

vector<TraceAnalyzer *> DetectorDriverXmlParser::ParseAnalyzers(const pugi::xml_node &node) {
    std::vector < TraceAnalyzer * > vecAnalyzer;

    for (pugi::xml_node analyzer = node; analyzer; analyzer = analyzer.next_sibling(node.name())) {
        string name = analyzer.attribute("name").value();
        messenger_.detail("Loading " + name);

        if (name == "CfdAnalyzer") {
            vecAnalyzer.push_back(new CfdAnalyzer(analyzer.attribute("type").as_string("poly")));
        } else if (name == "FittingAnalyzer") {
            vecAnalyzer.push_back(new FittingAnalyzer(analyzer.attribute("type").as_string("gsl")));
        } else if (name == "TauAnalyzer") {
            vecAnalyzer.push_back(new TauAnalyzer());
        } else if (name == "TraceExtractor") {
            vecAnalyzer.push_back(new TraceExtractor(analyzer.attribute("type").as_string(""),
                                                     analyzer.attribute("subtype").as_string(""),
                                                     analyzer.attribute("tag").as_string("trace")));
        } else if (name == "TraceFilterAnalyzer") {
            vecAnalyzer.push_back(new TraceFilterAnalyzer(analyzer.attribute("FindPileup").as_bool(false)));
        } else if (name == "WaaAnalyzer") {
            vecAnalyzer.push_back(new WaaAnalyzer());
        } else if (name == "WaveformAnalyzer") {
            std::vector <std::string> tokens =
                    StringManipulation::TokenizeString(analyzer.attribute("ignored").as_string(""), ",");
            vecAnalyzer.push_back(new WaveformAnalyzer(std::set<std::string>(tokens.begin(), tokens.end())));
        } else {
            stringstream ss;
            ss << "DetectorDriverXmlParser: Unknown analyzer : " << name;
            throw GeneralException(ss.str());
        }
        PrintAttributeMessage(analyzer);
    }
    return vecAnalyzer;
}

void DetectorDriverXmlParser::PrintAttributeMessage(pugi::xml_node &node) {
    stringstream ss;
    for (pugi::xml_attribute_iterator ait = node.attributes_begin();
         ait != node.attributes_end(); ++ait) {
        ss.str("");
        ss << ait->name();
        if (ss.str().compare("name") != 0) {
            ss << " = " << ait->value();
            messenger_.detail(ss.str(), 1);
        }
    }
}