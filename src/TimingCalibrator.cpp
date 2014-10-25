/*! \file TimingCalibrator.cpp
 *  \brief A class to handle the timing calibration parameters
 *  \author S. V. Paulauskas
 *  \date October 23, 2014
*/
#include <sstream>
#include <cmath>

#include "pugixml.hpp"

#include "Exceptions.hpp"
#include "Messenger.hpp"
#include "TimingCalibrator.hpp"

using namespace std;

TimingCalibrator* TimingCalibrator::instance = NULL;

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
        ss << "TimingCalibration: error parsing file Config.xml";
        ss << " : " << result.description();
        throw GeneralException(ss.str());
    }

    Messenger m;
    m.start("Loading Timing Calibrations");

    pugi::xml_node map = doc.child("Configuration").child("TimingCalibration");
    bool verbose = map.attribute("verbose_timing").as_bool();
//    for (pugi::xml_node module = map.child("Vandle"); module;
//         module = module.next_sibling("Module")) {
//        int module_number = module.attribute("number").as_int(-1);
//        for (pugi::xml_node channel = module.child("Channel"); channel;
//             channel = channel.next_sibling("Channel")) {
//            int ch_number = channel.attribute("number").as_int(-1);
//
//            bool corrected = false;
//            for (pugi::xml_node timecorr = channel.child("TimingCalibration");
//                timecorr; timecorr = timecorr.next_sibling("TimingCalibration")) {
//                double r0  = timecorr.attribute("r0").as_double(0);
//                double xoffset = timecorr.attribute("xoffset").as_double(0);
//                double lroffset = timecorr.attribute("lroffset").as_double(0);
//                double tofoffset0 = timecorr.attribute("tofoffset0").as_double(0);
//                double tofoffset1 = timecorr.attribute("tofoffset1").as_double(0);
//
//                if (verbose) {
//                    stringstream ss;
//                    ss << "Module " << module_number
//                       << ", channel " << ch_number << ": ";
//                    ss << " r0 = " << r0;
//                    m.detail(ss.str(), 1);
//                }
//                tcali->AddChannel(chanID, r0, xoffset, lroffset);
//                corrected = true;
//            }
//            if (!corrected && verbose) {
//                stringstream ss;
//                ss << "Module " << module_number << ", channel "
//                << ch_number << ": ";
//                ss << " not corrected for walk";
//                m.detail(ss.str(), 1);
//            }
//        }
//    }
    m.done();
}
