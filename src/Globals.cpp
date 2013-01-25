#include "pugixml.hpp"

#include "Exceptions.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "PathHolder.hpp"

Globals* Globals::instance = NULL;

Globals::Globals() {
    clockInSeconds_ = -1;
    adcClockInSeconds_ = -1;
    filterClockInSeconds_ = -1;
    eventInSeconds_ = -1;
    energyContraction_ = 1.0;
    revision_ = "None";

    pugi::xml_document doc;

    PathHolder* conf_path = new PathHolder();
    std::string xmlFileName = conf_path->GetFullPath("Config.xml");
    delete conf_path;
    pugi::xml_parse_result result = doc.load_file(xmlFileName.c_str());

    std::stringstream ss;
    if (!result) {
        ss << "Globals : error parsing file " << "Config.xml";
        ss << " : " << result.description();
        throw GeneralException(ss.str());
    }

    Messenger m;
    pugi::xml_node description =
        doc.child("Configuration").child("Description");
    std::string desc_text = description.text().get();
    m.detail("Experiment: " + desc_text);

    m.start("Loading global parameters");
    pugi::xml_node global = doc.child("Configuration").child("Global");
    for (pugi::xml_node_iterator it = global.begin();
                                 it != global.end(); ++it) {
        if (std::string(it->name()).compare("Revision") == 0) {
            revision_ = it->attribute("version").as_string();
            ss << "Revision: " << revision_;
            m.detail(ss.str());
            ss.str("");

            if (revision_ == "A" || revision_ == "D") {
                clockInSeconds_ = 10e-9; //< one pixie clock is 10 ns
                adcClockInSeconds_ = 10e-9; //< one ADC clock is 10 ns
                filterClockInSeconds_ = 10e-9; //< one filter clock is 10 ns
            } else if (revision_ == "F") {
                clockInSeconds_ = 8e-9; //< one pixie clock is 8 ns
                adcClockInSeconds_ = 4e-9; //< one ADC clock is 4 ns
                filterClockInSeconds_ = 8e-9; //< one filter clock is 8 ns
            } else {
                throw GeneralException("Globals: unknown revision version " + 
                                       revision_);
            }

        } else if (std::string(it->name()).compare("EventWidth") == 0) {

            std::string units = it->attribute("unit").as_string("None");
            double value = it->attribute("value").as_double(-1);
            if (units == "ns") {
                value *= 1e-9;
            } else if (units == "us"){
                value *= 1e-6;
            } else if (units == "ms"){
                value *= 1e-3;
            } else if (units == "s") {
            } else {
                throw GeneralException("Globals: unknown units " + 
                                       units);
            }
            eventInSeconds_ = value;
            ss << "Event width: " << eventInSeconds_ * 1e6 
               << " us" << ", i.e. " << eventWidth()
               << " pixie16 clock tics.";
            m.detail(ss.str());
            ss.str("");

        } else if (std::string(it->name()).compare("EnergyContraction") == 0) {

            energyContraction_ = it->attribute("value").as_double(1);

        } else {

            ss << "Unknown global parameter " << it->name();
            m.warning(ss.str());
            ss.str("");

        }
    }
    SanityCheck();
}

void Globals::SanityCheck() {
    Messenger m;
    std::stringstream ss;

    if (!(revision_ == "A" || revision_ == "D" || revision_ == "F")) {
        ss << "Globals: unknown revision version named "
           << revision_;
        throw GeneralException(ss.str());
    }

    if (clockInSeconds_ <= 0) {
        ss << "Globals: illegal value of clockInSeconds "
            << clockInSeconds_;
        throw GeneralException(ss.str());
    }

    if (adcClockInSeconds_ <= 0) {
        ss << "Globals: illegal value of adcClockInSeconds "
            << adcClockInSeconds_;
        throw GeneralException(ss.str());
    }

    if (filterClockInSeconds_ <= 0) {
        ss << "Globals: illegal value of filterClockInSeconds "
            << filterClockInSeconds_;
        throw GeneralException(ss.str());
    }

    if (eventInSeconds_ <= 0) {
        ss << "Globals: illegal value of eventInSeconds "
            << eventInSeconds_;
        throw GeneralException(ss.str());
    }

    if (energyContraction_ <= 0) {
        ss << "Globals: Surely you don't want to use Energy contraction = "
            << energyContraction_ << ". I'll better stop the program.";
        throw GeneralException(ss.str());
    } else {
        ss << "Energy contraction: " << energyContraction_;
        m.detail(ss.str());
    }

    m.done();
}

/** Instance is created upon first call */
Globals* Globals::get() {
    if (!instance) {
        instance = new Globals();
    }
    return instance;
}

