/** \file Globals.cpp
 * \brief constant parameters used in pixie16 analysis
 * \author K. A. Miernik
 */
#include <iostream>

#include <sys/times.h>
#include <unistd.h>

#include "Exceptions.hpp"
#include "Globals.hpp"

Globals* Globals::instance = NULL;

Globals::Globals() {
    sysClockFreqInHz_ = sysconf(_SC_CLK_TCK);
    clockInSeconds_ = -1;
    adcClockInSeconds_ = -1;
    filterClockInSeconds_ = -1;
    eventInSeconds_ = -1;
    hasReject_ = false;
    hasRaw_ = true;
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
            } else if (std::string(it->name()).compare("Path") == 0) {
                configPath_ =  it->text().get();
                m.detail("Path to other configuration files: " + configPath_);
            } else if (std::string(it->name()).compare("NumOfTraces") == 0) {
                numTraces_ =  it->attribute("value").as_uint();
            } else if (std::string(it->name()).compare("HasRaw") == 0) {
                hasRaw_ = it->attribute("value").as_bool(true);
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
            reject_.push_back(std::make_pair(start,end));
        }

        if (reject_.size() > 0) {
            hasReject_ = true;
        }

	pugi::xml_node phys = doc.child("Configuration").child("Physical");
        for(pugi::xml_node_iterator it = phys.begin(); it != phys.end(); ++it) {
            if(std::string(it->name()).compare("NeutronMass") == 0)
                neutronMass_ = it->attribute("value").as_double(939.565560);
            else if(std::string(it->name()).compare("SpeedOfLight") == 0)
                speedOfLight_ = it->attribute("value").as_double(29.9792458);
            else if(std::string(it->name()).compare("SpeedOfLightSmall") == 0)
                speedOfLightSmall_ = it->attribute("value").as_double(12.65822);
            else if(std::string(it->name()).compare("SpeedOfLightBig") == 0)
                speedOfLightBig_ = it->attribute("value").as_double(15.22998);
            else if(std::string(it->name()).compare("SpeedOfLightMedium") == 0)
                speedOfLightMedium_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("SmallLength") == 0)
                smallLength_ = it->attribute("value").as_double(60);
            else if(std::string(it->name()).compare("MediumLength") == 0)
                mediumLength_ = it->attribute("value").as_double(120);
            else if(std::string(it->name()).compare("BigLength") == 0)
                bigLength_ = it->attribute("value").as_double(200);
            else
                WarnOfUnknownParameter(m, it);
        }

	pugi::xml_node trc = doc.child("Configuration").child("Trace");
        for(pugi::xml_node_iterator it = trc.begin(); it != trc.end(); ++it) {
            if(std::string(it->name()).compare("DiscriminationStart") == 0)
		discriminationStart_ = it->attribute("value").as_double(3);
            else if(std::string(it->name()).compare("TrapezoidalWalk") == 0)
		trapezoidalWalk_ = it->attribute("value").as_double(266);
            else if(std::string(it->name()).compare("TraceDelay") == 0)
		traceDelay_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("TraceLength") == 0)
		traceLength_ = it->attribute("value").as_double();
            else if(std::string(it->name()).compare("QdcCompression") == 0)
		qdcCompression_ = it->attribute("value").as_double(1.0);
            else if(std::string(it->name()).compare("WaveformRange") == 0) {
		for(pugi::xml_node_iterator waveit = it->begin();
		    waveit != it->end(); ++waveit) {
		    waveformRanges_.insert(std::make_pair(waveit->attribute("name").as_string(),
		        std::make_pair(waveit->child("Low").attribute("value").as_int(5),
			waveit->child("High").attribute("value").as_int(10))));
		}
	    } else if (std::string(it->name()).compare("TrapFilters") == 0) { 
		for(pugi::xml_node_iterator trapit = it->begin();
		    trapit != it->end(); ++trapit) {
		    pugi::xml_node trig = trapit->child("Trigger");
		    TrapFilterParameters tfilt(trig.attribute("l").as_double(125),
					       trig.attribute("g").as_double(125),
					       trig.attribute("t").as_double(10));
		    pugi::xml_node en = trapit->child("Energy");
		    TrapFilterParameters efilt(en.attribute("l").as_double(300),
					       en.attribute("g").as_double(300),
					       en.attribute("t").as_double(50));
		    trapFiltPars_.insert(std::make_pair(trapit->attribute("name").as_string(),
							std::make_pair(tfilt,efilt)));
								       
		}
	    } else
		WarnOfUnknownParameter(m, it);
	}
	
	pugi::xml_node fit = doc.child("Configuration").child("Fitting");
	for(pugi::xml_node_iterator it = fit.begin(); it != fit.end(); ++it) {
            if(std::string(it->name()).compare("SigmaBaselineThresh") == 0)
                sigmaBaselineThresh_ = it->attribute("value").as_double(3.0);
            else if(std::string(it->name()).compare("SiPmtSigmaBaselineThresh") == 0)
                siPmtSigmaBaselineThresh_ = it->attribute("value").as_double(25.0);
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
