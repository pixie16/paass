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

TimingCalibration TimingCalibrator::GetCalibration(const TimingDefs::TimingIdentifier &id) {
    map<TimingDefs::TimingIdentifier, TimingCalibration>::iterator it =
        calibrations_.find(id);

    if(it == calibrations_.end() && isVerbose_) {
        stringstream ss;
        ss << "TimingCalibrator: You have attempted to access a "
            << "time calibration that does not exist (" << id.first << ","
            << id.second << "). I will be returning a zero calibration "
            << "until you add one." << endl;
        throw GeneralWarning(ss.str());
        TimingCalibration zero;
        return(zero);
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

    isVerbose_ = timeCals.attribute("verbose_timing").as_bool();

    for(pugi::xml_node_iterator detType = timeCals.begin();
        detType != timeCals.end(); ++detType) {
        string detName = detType->name();

        for(pugi::xml_node_iterator detSubtype = detType->begin();
            detSubtype != detType->end(); detSubtype++) {
            string barType = detSubtype->name();

            for(pugi::xml_node_iterator bar = detSubtype->begin();
                bar != detSubtype->end(); bar++) {
                int barNumber = bar->attribute("number").as_int(-1);
                TimingDefs::TimingIdentifier id = make_pair(barNumber, barType);

                TimingCalibration temp;
                temp.SetLeftRightTimeOffset(bar->
                    attribute("lroffset").as_double(0.0));
                temp.SetTofOffset0(bar->
                    attribute("tofoffset0").as_double(0.0));
                temp.SetTofOffset1(bar->
                    attribute("tofoffset1").as_double(0.0));
                temp.SetXOffset(bar->
                    attribute("xoffset").as_double(0.0));
                temp.SetZ0(bar->
                    attribute("z0").as_double(0.0));
                temp.SetZOffset(bar->
                    attribute("zoffset").as_double(0.0));

                if(!calibrations_.insert(make_pair(id, temp)).second) {
                    stringstream ss;
                    ss << "TimingCalibrator: We have found a duplicate "
                       << "entry into the TimingCalibrations at "
                       << bar->path() << barNumber << endl
                       << "Ignoring duplicate. ";
                    m_.warning(ss.str());
                }
                if (isVerbose_) {
                    stringstream ss;
                    ss << detName << ":" << barType << ":" << barNumber
                        << " lroffset = " << temp.GetLeftRightTimeOffset()
                        << ", xoffset = " << temp.GetXOffset()
                        << ", z0 = " << temp.GetZ0() << ", tofOffset0 = "
                        << temp.GetTofOffset0() << ", tofOffset1 = "
                        << temp.GetTofOffset1() << endl;
                    m.detail(ss.str(), 1);
                }
            }
        }
    }
    m.done();
}
