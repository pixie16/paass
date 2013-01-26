#include <cmath>
#include "WalkCorrector.hpp"
#include "Exceptions.hpp"

using namespace std;


void WalkCorrector::AddChannel(const Identifier& chanID,
                               const std::string model,
                               double min, double max, 
                               const std::vector<double>& par) {
    CorrectionParams cf;

    unsigned required_parameters = 0;
    if (model == "None") {
        cf.model = walk_none;
    } else if (model == "A") {
        cf.model = walk_A;
        required_parameters = 5;
    } else if (model == "B1") {
        cf.model = walk_B1;
        required_parameters = 7;
    } else if (model == "B2") {
        cf.model = walk_B2;
        required_parameters = 6;
    } else {
        stringstream ss;
        ss << "WalkCorrector: unknown walk model " << model;
        throw GeneralException(ss.str());
    }

    if (min > max) {
        stringstream ss;
        ss << "WalkCorrector: wrong calibration range, channels "
           << min << " to " << max;
        throw GeneralException(ss.str());
    }

    cf.min = min;
    cf.max = max;
    
    for (vector<double>::const_iterator it = par.begin(); it != par.end();
        ++it) {
        cf.parameters.push_back(*it);
    }

    if (cf.parameters.size() < required_parameters) {
        stringstream ss;
        ss << "WalkCorrector: selected model needs at least " 
           << required_parameters
           << " but only " << cf.parameters.size() << " where found";
        throw GeneralException(ss.str());
    }

    // Some parameters check are needed to avoid e.g. division by 0
    switch(cf.model) {
        case walk_none:
            break;
        case walk_A: 
            if (par[2] <= 0)
                throw GeneralException("WalkCorrector: Model A, parameter 2 must be larger then 0.");
            break;
        case walk_B1:
            if (par[6] <= 0)
                throw GeneralException("WalkCorrector: Model B1, parameter 6 must be larger then 0.");
            break;
        case walk_B2:
            if (par[5] <= 0)
                throw GeneralException("WalkCorrector: Model B2, parameter 5 must be larger then 0.");
            break;
        default: 
            break;
    }

    if (channels_.find(chanID) != channels_.end()) {
        channels_[chanID].push_back(cf);
    } else {
        vector<CorrectionParams> vcf;
        vcf.push_back(cf);
        channels_[chanID] = vcf;
    }
}

double WalkCorrector::GetCorrection(Identifier& chanID, double raw) const {
    map<Identifier, vector<CorrectionParams> >::const_iterator itch =
        channels_.find(chanID);
    if (itch != channels_.end()) {
        vector<CorrectionParams>::const_iterator itf;
        for (itf = itch->second.begin(); itf != itch->second.end(); ++itf) {
            if (itf->min <= raw && raw <= itf->max)
                break;
        }
        // If some min-max range is missing zero is returned 
        if (itf == itch->second.end()) {
            return 0;
        }
        switch(itf->model) {
            case walk_none:
                return Model_None();
                break;
            case walk_A: 
                return Model_A(itf->parameters, raw);
                break;
            case walk_B1: 
                return Model_B1(itf->parameters, raw);
                break;
            case walk_B2: 
                return Model_B2(itf->parameters, raw);
                break;
            default: 
                break;
        }
    }

    // If no walk correction found, return 0
    return 0;
}

double WalkCorrector::Model_None() const {
    return 0;
}

double WalkCorrector::Model_A(const std::vector<double>& par, 
                              double raw) const {
    return par[0] + par[1] / (par[2] + raw) + par[3] * exp(-raw / par[4]);
}

double WalkCorrector::Model_B1(const std::vector<double>& par, 
                               double raw) const {
    return par[0] + par[1] * raw + par[2] * raw * raw + 
           par[3] * raw * raw * raw + 
           par[4] / (1 + exp((raw - par[5]) / par[6]));
}

double WalkCorrector::Model_B2(const std::vector<double>& par, 
                               double raw) const {
    return par[0] + par[1] * raw + 
           (par[2] + par[3] * raw + par[4] * raw * raw) * exp(-raw / par[5]);
}
