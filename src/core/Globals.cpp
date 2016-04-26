/** \file Globals.cpp
 * \brief constant parameters used in pixie16 analysis
 * \author K. A. Miernik
 */
#include <iostream>

#include "Exceptions.hpp"
#include "Globals.hpp"

Globals* Globals::instance = NULL;

Globals::Globals() {
    clockInSeconds_ = -1;
    adcClockInSeconds_ = -1;
    filterClockInSeconds_ = -1;
    eventInSeconds_ = -1;
    energyContraction_ = 1.0;
    hasReject_ = false;
    revision_ = "None";
    numTraces_  = 16;

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
                    clockInSeconds_ = 10e-9;
                    adcClockInSeconds_ = 10e-9;
                    filterClockInSeconds_ = 10e-9;
                    maxWords_ = IO_BUFFER_LENGTH;
                } else if (revision_ == "D") {
                    clockInSeconds_ = 10e-9;
                    adcClockInSeconds_ = 10e-9;
                    filterClockInSeconds_ = 10e-9;
                    maxWords_ = EXTERNAL_FIFO_LENGTH;
                } else if (revision_ == "F" || revision_ == "DF") {
                    clockInSeconds_ = 8e-9;
                    adcClockInSeconds_ = 4e-9;
                    filterClockInSeconds_ = 8e-9;
                    maxWords_ = EXTERNAL_FIFO_LENGTH;
                } else {
                    throw GeneralException("Globals: unknown revision version "
                                           + revision_);
                }

            } else if (std::string(it->name()).compare("EventWidth") == 0) {

                std::string units = it->attribute("unit").as_string("None");
                double value = it->attribute("value").as_double(-1);

                if (units == "ns")
                    value *= 1e-9;
                else if (units == "us")
                    value *= 1e-6;
                else if (units == "ms")
                    value *= 1e-3;
                else if (units == "s")
                    value *= 1.0;
                else
                    throw GeneralException("Globals: unknown units " + units);

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
            } else if (std::string(it->name()).compare("NumOfTraces") == 0) {
                numTraces_ =  it->attribute("value").as_uint();
            } else
                WarnOfUnknownParameter(m, it);
        }

        unsigned int power2 = 1;
        unsigned int maxDammSize = 16384;
        while (power2 < numTraces_ && power2 < maxDammSize) {
            power2 *= 2;
        }
        ss << "Number of traces set to " << power2 << " ("
           << numTraces_ << ")";
        m.detail(ss.str());
        ss.str("");
        numTraces_ = power2;

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

        pugi::xml_node timing = doc.child("Configuration").child("Timing");

        for(pugi::xml_node_iterator it = timing.child("Physical").begin();
            it != timing.child("Physical").end(); ++it) {
            if(std::string(it->name()).compare("NeutronMass") == 0)
                neutronMass_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("SpeedOfLight") == 0)
                speedOfLight_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("SpeedOfLightSmall") == 0)
                speedOfLightSmall_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("SpeedOfLightBig") == 0)
                speedOfLightBig_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("SpeedOfLightMedium") == 0)
                speedOfLightMedium_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("SmallLength") == 0)
                smallLength_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("MediumLength") == 0)
                mediumLength_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("BigLength") == 0)
                bigLength_ = it->attribute("value").as_double();
            else
                WarnOfUnknownParameter(m, it);
        }

	


        for(pugi::xml_node_iterator it = timing.child("Trace").begin();
            it != timing.child("Trace").end(); ++it) {
	    if(std::string(it->name()).compare("DiscriminationStart") == 0)
		discriminationStart_ = it->attribute("value").as_double();
	    else if(std::string(it->name()).compare("TrapezoidalWalk") == 0)
		trapezoidalWalk_ = it->attribute("value").as_double();
	    else if(std::string(it->name()).compare("TraceDelay") == 0)
		traceDelay_ = it->attribute("value").as_double();
	    else if(std::string(it->name()).compare("TraceLength") == 0)
		traceLength_ = it->attribute("value").as_double();
	    else if(std::string(it->name()).compare("QdcCompression") == 0)
		qdcCompression_ = it->attribute("value").as_double();
	    else if(std::string(it->name()).compare("WaveformRange") == 0) {
		for(pugi::xml_node_iterator waveit = it->begin(); 
		    waveit != it->end(); ++waveit) {
		    waveformRanges_.insert(std::make_pair(waveit->attribute("name").as_string(),
							  std::make_pair(waveit->child("Low").attribute("value").as_int(5),
									 waveit->child("High").attribute("value").as_int(10))));
		} 
	    } else
		WarnOfUnknownParameter(m, it);
	}
	
	for(pugi::xml_node_iterator it = timing.child("Fitting").begin();
            it != timing.child("Fitting").end(); ++it) {
	    
            if(std::string(it->name()).compare("SigmaBaselineThresh") == 0)
                sigmaBaselineThresh_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("SiPmtSigmaBaselineThresh") == 0)
                siPmtSigmaBaselineThresh_ = it->attribute("value").as_double();
	    else if(std::string(it->name()).compare("Parameters") == 0) {
		for(pugi::xml_node_iterator parit = it->begin();
		    parit != it->end(); ++parit) {
		    fitPars_.insert(std::make_pair(parit->attribute("name").as_string(),
						   std::make_pair(parit->child("Beta").attribute("value").as_double(0.),
								  parit->child("Gamma").attribute("value").as_double(0.))));
		}
	    } else
                WarnOfUnknownParameter(m, it);
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
            << energyContraction_ << ". I'd better stop the program.";
        throw GeneralException(ss.str());
    } else {
        ss << "Energy contraction: " << energyContraction_;
        m.detail(ss.str());
    }

    m.done();
}

void Globals::WarnOfUnknownParameter(Messenger &m,
                                     pugi::xml_node_iterator &it) {
    std::stringstream ss;
    ss << "Unknown parameter in Config.xml : " << it->path();
    m.warning(ss.str());
}

/** Instance is created upon first call */
Globals* Globals::get() {
    if (!instance) {
        instance = new Globals();
    }
    return instance;
}

Globals::~Globals() {
    delete instance;
    instance = NULL;
}
