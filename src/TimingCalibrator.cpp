/*! \file TimingCalibrator.cpp
 *  \brief A class to handle the timing calibration parameters
 *  \author S. V. Paulauskas
 *  \date October 23, 2014
*/
#include <iostream>
#include <sstream>
#include <cmath>

#include "pugixml.hpp"

#include "Exceptions.hpp"
#include "TimingCalibrator.hpp"

using namespace std;

TimingCalibrator* TimingCalibrator::instance = NULL;

TimingCalibration TimingCalibrator::GetCalibration(const Vandle::BarIdentifier &id) {
    map<Vandle::BarIdentifier, TimingCalibration>::iterator it =
        calibrations_.find(id);

    if(it == calibrations_.end()) {
        stringstream ss;
        ss << "TimingCalibrator: You have attempted to access a "
            << "time calibration that does not exist (" << id.first << ","
            << id.second << "). I refuse to continue "
            << "until you fix this issue." << endl;
        throw GeneralException(ss.str());
    }
    return((*it).second);
}

/*! Instance is created upon first call */
TimingCalibrator* TimingCalibrator::get() {
    if (!instance)
        instance = new TimingCalibrator();
    return instance;
}

void TimingCalibrator::ReadTimingCalXml() {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file("Config.xml");
    if (!result) {
        stringstream ss;
        ss << "TimingCalibrator: error parsing file Config.xml";
        ss << " : " << result.description();
        throw GeneralException(ss.str());
    }

    Messenger m;
    m.start("Loading Time Calibrations");

    pugi::xml_node timeCals =
        doc.child("Configuration").child("TimeCalibration");

    bool verbose = timeCals.attribute("verbose_timing").as_bool();

    for(pugi::xml_node_iterator detType = timeCals.begin();
        detType != timeCals.end(); ++detType) {
        string detName = detType->name();

        for(pugi::xml_node_iterator detSubtype = detType->begin();
            detSubtype != detType->end(); detSubtype++) {
            string barType = detSubtype->name();

            for(pugi::xml_node_iterator bar = detSubtype->begin();
                bar != detSubtype->end(); bar++) {
                int barNumber = bar->attribute("number").as_int(-1);
                Vandle::BarIdentifier id = make_pair(barNumber, barType);

                TimingCalibration temp;
                temp.SetLeftRightTimeOffset(bar->
                    attribute("lroffset").as_double(0.0));
                temp.SetR0(bar->
                    attribute("r0").as_double(0.0));
                temp.SetTofOffset0(bar->
                    attribute("tofOffset0").as_double(0.0));
                temp.SetTofOffset1(bar->
                    attribute("tofOffset1").as_double(0.0));
                temp.SetXOffset(bar->
                    attribute("xOffset").as_double(0.0));
                temp.SetZ0(bar->
                    attribute("z0").as_double(0.0));
                temp.SetZOffset(bar->
                    attribute("zOffset").as_double(0.0));

                if(!calibrations_.insert(make_pair(id, temp)).second) {
                    stringstream ss;
                    ss << "TimingCalibrator: We have found a duplicate "
                       << "entry into the TimingCalibrations at "
                       << bar->path() << barNumber << endl
                       << "Ignoring duplicate. ";
                    m_.warning(ss.str());
                }
                if (verbose) {
                    stringstream ss;
                    ss << detName << ":" << barType << ":" << barNumber
                        << "-> r0 = " << temp.GetR0() << ", lroffset = "
                        << temp.GetLeftRightTimeOffset() << ", xoffset = "
                        << temp.GetXOffset() << ", z0 = " << temp.GetZ0()
                        << ", tofOffset0 = " << temp.GetTofOffset0()
                        << ", tofOffset1 = " << temp.GetTofOffset1() << endl;
                    m.detail(ss.str(), 1);
                }
            }
        }
    }
    m.done();
}
