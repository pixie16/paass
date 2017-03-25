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

using namespace std;

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
    InitializeMemberVariables();
    configFile_ = file;

    try {
        GlobalsXmlParser parser;
        parser.ParseNode(this);
    }catch (invalid_argument &invalidArgument){
        throw;
    } catch (GeneralException &ex) {
        throw;
    }
}

void Globals::InitializeMemberVariables() {
    sysClockFreqInHz_ = sysconf(_SC_CLK_TCK);
    hasRawHistogramsDefined_ = true;
    outputFilename_ = outputPath_ = revision_ = "";
    eventLengthInTicks_ = discriminationStart_ = 0;
    qdcCompression_ = 1.0;
    adcClockInSeconds_ = clockInSeconds_ = eventLengthInSeconds_ =
    filterClockInSeconds_ = sigmaBaselineThresh_ =
    siPmSigmaBaselineThresh_ = vandleBigSpeedOfLight_ =
    vandleMediumSpeedOfLight_ = vandleSmallSpeedOfLight_ = 0;
}
