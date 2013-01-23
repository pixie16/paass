#include <cmath>
//TEMP
#include <iostream>

#include "Calibrator.hpp"
#include "Exceptions.hpp"

using namespace std;


void Calibrator::AddChannel(const Identifier& chanID, const std::string model,
                            double min, double max, 
                            const std::vector<double>& par) {
    CalibrationFactor cf;
    if (model == "Poly") {
        cf.model = Poly;
    } else if (model == "Raw") {
        cf.model = Raw;
    } else {
        stringstream ss;
        ss << "Calibrator: unknown calibration model " << model;
        throw GeneralException(ss.str());
    }

    if (min > max) {
        stringstream ss;
        ss << "Calibrator: wrong calibration range, channels "
           << min << " to " << max;
        throw GeneralException(ss.str());
    }
    cf.min = min;
    cf.max = max;
    
    for (vector<double>::const_iterator it = par.begin(); it != par.end();
        ++it) {
        cf.parameters.push_back(*it);
    }

    if (channels_.find(chanID) != channels_.end()) {
        channels_[chanID].push_back(cf);
    } else {
        vector<CalibrationFactor> vcf;
        vcf.push_back(cf);
        channels_[chanID] = vcf;
    }
}

double Calibrator::GetCalEnergy(Identifier& chanID, double ch) const {
    map<Identifier, vector<CalibrationFactor> >::const_iterator itch =
        channels_.find(chanID);
    if (itch != channels_.end()) {
        vector<CalibrationFactor>::const_iterator itf;
        for (itf = itch->second.begin(); itf != itch->second.end(); ++itf) {
            if (itf->min <= ch && ch <= itf->max)
                break;
        }
        // Parts of spectrum that are not within some min-max range are
        // zeroed
        if (itf == itch->second.end()) {
            return 0;
        }
        switch(itf->model) {
            case Poly: 
                return Model_Poly(itf->parameters, ch);
                break;
            case Raw:
                return Model_Raw(ch);
                break;
            default: return ch;
        }
    }
    // If no calibration found, return raw channel
    return ch;
}

double Calibrator::Model_Raw(double ch) const {
    return ch;
}

double Calibrator::Model_Poly(const std::vector<double>& par, double ch) const {
    int p = 0;
    double r = 0;
    for (vector<double>::const_iterator it = par.begin(); it != par.end();
        ++it) {
        r += (*it) * pow(ch, p);
        ++p;
    }
    return r;
}
