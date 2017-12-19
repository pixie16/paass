/*! \file TimingCalibrator.cpp
 *  \brief A class to handle the timing calibration parameters
 *  \author S. V. Paulauskas
 *  \date October 23, 2014
*/
#include <iostream>
#include <sstream>
#include <cmath>

#include "Exceptions.hpp"
#include "TimingCalibrator.hpp"
#include "XmlInterface.hpp"

using namespace std;

TimingCalibrator *TimingCalibrator::instance = NULL;

TimingCalibration
TimingCalibrator::GetCalibration(const TimingDefs::TimingIdentifier &id) {
    map<TimingDefs::TimingIdentifier, TimingCalibration>::iterator it = calibrations_.find(id);

    if (it == calibrations_.end())
        return (default_);
    return ((*it).second);
}

TimingCalibrator *TimingCalibrator::get() {
    if (!instance)
        instance = new TimingCalibrator();
    return instance;
}

void TimingCalibrator::ReadTimingCalXml() {
    Messenger m;
    m.start("Loading Time Calibrations");

    pugi::xml_document TimeCalFile; //!< A seperate Time cal file (for Jeremy)

    pugi::xml_node timeCals = XmlInterface::get()->GetDocument()->child("Configuration").child("TimeCalibration");

    std::string TcalFilePath_ = timeCals.attribute("path").as_string();
    isVerbose_ = timeCals.attribute("verbose").as_bool();

    if(TcalFilePath_ !=""){
        pugi::xml_parse_result result = TimeCalFile.load_file(TcalFilePath_.c_str());
        if (!result) {
            std::stringstream ss;
            ss << "XmlInterface::XmlInterface : We were unable to open the Time Calibration file named \"" << TcalFilePath_ << "\"."
               << " Received the following from pugixml : "<< result.description();
            throw std::invalid_argument(ss.str());
        }

        std::cout << "XmlInterface - Successfully loaded TcalFilePath \"" << TcalFilePath_ << "\" into memory." << std::endl;
        timeCals = TimeCalFile.child("TimeCalibration");
    }

    for (pugi::xml_node_iterator detType = timeCals.begin(); detType != timeCals.end(); ++detType) {
        string detName = detType->name();
        std::cout<<" detName:"<<detName<<std::endl;

        for (pugi::xml_node_iterator detSubtype = detType->begin(); detSubtype != detType->end(); detSubtype++) {
            string barType = detSubtype->name();
            std::cout<<" barType: "<<barType<<std::endl;

            for (pugi::xml_node_iterator bar = detSubtype->begin(); bar != detSubtype->end(); bar++) {
                int barNumber = bar->attribute("number").as_int(-1);
                std::cout<<" barNumber: "<<barNumber<<std::endl;
                TimingDefs::TimingIdentifier id = make_pair(barNumber, barType);

                TimingCalibration temp;
                temp.SetLeftRightTimeOffset(bar->attribute("lroffset").as_double(0.0));
                std::cout<<'\t'<<"lroffset "<< bar->attribute("lroffset").as_double(0.0) << std::endl;
                temp.SetXOffset(bar->attribute("xoffset").as_double(0.0));
                std::cout<<'\t'<<"xoffset "<< bar->attribute("xoffset").as_double(0.0) << std::endl;
                temp.SetZ0(bar->attribute("z0").as_double(0.0));
                std::cout<<'\t'<<"z0 "<< bar->attribute("z0").as_double(0.0) << std::endl;
                temp.SetZOffset(bar->attribute("zoffset").as_double(0.0));
                std::cout<<'\t'<<"zoffset "<< bar->attribute("zoffset").as_double(0.0) << std::endl;

                for (pugi::xml_node::iterator tofoffset = bar->begin(); tofoffset != bar->end(); tofoffset++)
                    temp.SetTofOffset(tofoffset->attribute("location").as_int(-1),
                                      tofoffset->attribute("offset").as_double(0.0));

                if (!calibrations_.insert(make_pair(id, temp)).second) {
                    stringstream ss;
                    ss << "TimingCalibrator: We have found a duplicate " << "entry into the TimingCalibrations at "
                       << bar->path() << barNumber << endl << "Ignoring duplicate. ";
                    m_.warning(ss.str());
                }

                if (isVerbose_) {
                    stringstream ss;
                    ss << detName << ":" << barType << ":" << barNumber
                       << " lroffset = " << temp.GetLeftRightTimeOffset() << ", xoffset = " << temp.GetXOffset()
                       << ", z0 = " << temp.GetZ0() << ", ToF Offsets = ";
                    for (unsigned int i = 0; i < temp.GetNumTofOffsets(); i++)
                        ss << i << ":" << temp.GetTofOffset(i) << ", ";
                    ss << endl;
                    m.detail(ss.str(), 1);
                }
            }
        }
    }
    m.done();
}
