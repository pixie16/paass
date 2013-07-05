#include "pugixml.hpp"

#include "Exceptions.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"

Globals* Globals::instance = NULL;

Globals::Globals() {
    clockInSeconds_ = -1;
    adcClockInSeconds_ = -1;
    filterClockInSeconds_ = -1;
    eventInSeconds_ = -1;
    energyContraction_ = 1.0;
    hasReject_ = false;
    revision_ = "None";

    try {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file("Config.xml");

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

                if (revision_ == "A") {
                    clockInSeconds_ = 10e-9; //< one pixie clock is 10 ns
                    adcClockInSeconds_ = 10e-9; //< one ADC clock is 10 ns
                    filterClockInSeconds_ = 10e-9; //< one filter clock is 10 ns
                    maxWords_ = IO_BUFFER_LENGTH;
                } else if (revision_ == "D") {
                    clockInSeconds_ = 10e-9; //< one pixie clock is 10 ns
                    adcClockInSeconds_ = 10e-9; //< one ADC clock is 10 ns
                    filterClockInSeconds_ = 10e-9; //< one filter clock is 10 ns
                    maxWords_ = EXTERNAL_FIFO_LENGTH;
                } else if (revision_ == "F") {
                    clockInSeconds_ = 8e-9; //< one pixie clock is 8 ns
                    adcClockInSeconds_ = 4e-9; //< one ADC clock is 4 ns
                    filterClockInSeconds_ = 8e-9; //< one filter clock is 8 ns
                    maxWords_ = EXTERNAL_FIFO_LENGTH; 
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
                eventWidth_ = (int)(eventInSeconds_ / clockInSeconds_);
                ss << "Event width: " << eventInSeconds_ * 1e6 
                << " us" << ", i.e. " << eventWidth_
                << " pixie16 clock tics.";
                m.detail(ss.str());
                ss.str("");

            } else if (std::string(it->name()).compare("EnergyContraction") == 0) {

                energyContraction_ = it->attribute("value").as_double(1);

            } else if (std::string(it->name()).compare("Path") == 0) {

                configPath_ =  it->text().get();
                m.detail("Path to other configuration files: " + configPath_);

            } else {

                ss << "Unknown global parameter " << it->name();
                m.warning(ss.str());
                ss.str("");

            }
        }

        m.detail("Loading rejection regions");
        pugi::xml_node reject = doc.child("Configuration").child("Reject");
        for (pugi::xml_node time = reject.child("Time"); time;
            time = time.next_sibling("Time")) {
            int start = time.attribute("start").as_int(-1);
            int end = time.attribute("end").as_int(-1);

            std::stringstream ss;
            if (start < 0 || end < 0 || start > end) {
                ss << "Globals: incomplete or wrong rejection region "
                <<  "declaration: " << start << ", " << end;
                throw GeneralException(ss.str());
            }

            ss << "Rejection region: " << start << " to " << end << " s";
            m.detail(ss.str(), 1);
            std::pair<int, int> region(start, end);
            reject_.push_back(region);
        }

        if (reject_.size() > 0) {
            hasReject_ = true;
        }

        SanityCheck();
    } catch (std::exception &e) {
        std::cout << "Exception caught at Globals" << std::endl;
        std::cout << "\t" << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
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

    if (hasReject_) {
        ss << "Total number of rejection regions: " << reject_.size();
        m.detail(ss.str());
    } else {
        ss << "Not using rejection regions";
        m.detail(ss.str());
    }

    ss.str("");
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

