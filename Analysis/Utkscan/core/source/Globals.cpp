/** \file Globals.cpp
 * \brief constant parameters used in pixie16 analysis
 * \author K. A. Miernik and S. V. Paulauskas
 */
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <unistd.h>

#include "Exceptions.hpp"
#include "Globals.hpp"
#include "GlobalsXmlParser.hpp"
#include "XmlInterface.hpp"

///Initializes the instance_ of the class to a null pointer.
Globals *Globals::instance_ = NULL;

///Instance is created upon first call
Globals *Globals::get() {
    if (!instance_)
        instance_ = new Globals("Config.xml");
    return (instance_);
}

///Instance is created upon first call
Globals *Globals::get(const std::string &file) {
    if (!instance_)
        instance_ = new Globals(file);
    return (instance_);
}

///The destructor that will delete the instance_ of the class.
Globals::~Globals() {
    delete (instance_);
    instance_ = NULL;
}

Globals::Globals(const std::string &file) {
    const pugi::xml_node rootNode =
            XmlInterface::get(file)->GetDocument()->child("Configuration");

    GlobalsXmlParser xmlParser;

    if(!rootNode)
        throw std::invalid_argument("The root node \"/Configuration\" does "
                                            "not exist. No configuration can "
                                            "be loaded");
    else
        xmlParser.ParseRootNode(rootNode);

    InitializeMemberVariables();
    configFile_ = file;

    std::stringstream ss;
    Messenger m;

    m.detail("Experiment Summary : " + xmlParser.ParseDescriptionNode(
            rootNode.child("Description")));

    try {
        if(rootNode.child("Global")) {
            m.start("Loading Global Node");
            xmlParser.ParseGlobalNode(rootNode.child("Global"), this);
            m.done();
        } else
            throw GeneralException("Globals::Globals : We are missing "
                                                "the Globals node!");

        if(rootNode.child("Reject")) {
            m.start("Loading Reject Node");
            reject_ = xmlParser.ParseRejectNode(rootNode.child("Reject"));
            m.done();
        }

        if(rootNode.child("Vandle")) {
            m.start("Loading Vandle Node");
            xmlParser.ParseVandleNode(rootNode.child("Vandle"), this);
            m.done();
        }

        if(rootNode.child("Trace")) {
            m.start("Loading Trace Node");
            xmlParser.ParseTraceNode(rootNode.child("Trace"), this);
            m.done();
        }

        if(rootNode.child("Cfd")) {
            m.start("Loading Cfd Node");
            xmlParser.ParseCfdNode(rootNode.child("Cfd"), this);
            m.done();
        }

        if(rootNode.child("Fitting")) {
            m.start("Loading Fitting Node");
            xmlParser.ParseFittingNode(rootNode.child("Fitting"), this);
            m.done();
        }
    } catch (std::exception &e) {
        m.detail("Globals::Globals : Exception caught while parsing "
                         "configuration file.");
        throw;
    }
}

void Globals::InitializeMemberVariables() {
    sysClockFreqInHz_ = sysconf(_SC_CLK_TCK);
    hasRawHistogramsDefined_ = true;
    outputFilename_ = outputPath_ = revision_ = "";
    eventLengthInTicks_ = traceDelay_ = discriminationStart_ = 0;
    qdcCompression_ = 1.0;
    adcClockInSeconds_ = clockInSeconds_ = eventLengthInSeconds_ =
    filterClockInSeconds_ = sigmaBaselineThresh_ =
    siPmSigmaBaselineThresh_ = vandleBigSpeedOfLight_ =
    vandleMediumSpeedOfLight_ = vandleSmallSpeedOfLight_ = 0;
}