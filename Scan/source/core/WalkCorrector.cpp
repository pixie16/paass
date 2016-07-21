/** \file WalkCorrector.hpp
 * \brief A Class to handle walk corrections for channels
 * \author K. A. Miernik
 * \date January 22, 2013
 */
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
        cf.model = none;
    } else if (model == "A") {
        cf.model = A;
        required_parameters = 5;
    } else if (model == "B1") {
        cf.model = B1;
        required_parameters = 4;
    } else if (model == "B2") {
        cf.model = B2;
        required_parameters = 3;
    } else if (model == "VS") {
	cf.model = VS;
    } else if (model == "VM") {
	cf.model = VM;
    } else if (model == "VL" ){
	cf.model = VL;
    } else if (model == "VB") { 
	cf.model = VB;
    } else if (model == "VD") {
	cf.model = VD;
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

    switch(cf.model) {
        case none:
            break;
        case A:
            if (par[2] <= 0) {
                stringstream ss;
                ss << "WalkCorrector: Model A, parameter 2 must "
                   << "be larger then 0, value " << par[2] << " was found";
                throw GeneralException(ss.str());
            }
            break;
        case B1:
            if (par[3] <= 0) {
                stringstream ss;
                ss << "WalkCorrector: Model B1, parameter 3 must "
                   << "be larger then 0, value " << par[2] << " was found";
                throw GeneralException(ss.str());
            }
            break;
        case B2:
            if (par[2] <= 0) {
                stringstream ss;
                ss << "WalkCorrector: Model B2, parameter 2 must "
                   << "be larger then 0, value " << par[2] << " was found";
                throw GeneralException(ss.str());
            }
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
        if (itf == itch->second.end()) {
            return 0;
        }
        switch(itf->model) {
            case none:
                return Model_None();
                break;
            case A:
                return Model_A(itf->parameters, raw);
                break;
            case B1:
                return Model_B1(itf->parameters, raw);
                break;
            case B2:
                return Model_B2(itf->parameters, raw);
                break;
            case VS:
                return Model_VS(itf->parameters, raw);
                break;
	    case VM:
                return Model_VM(itf->parameters, raw);
                break;
            case VL:
                return Model_VL(itf->parameters, raw);
                break;
            case VD:
                return Model_VD(itf->parameters, raw);
                break;
            case VB:
                return Model_VB(itf->parameters, raw);
                break;
            default:
                break;
        }
    }
    return 0;
}

double WalkCorrector::Model_None() const {
    return(0.0);
}

double WalkCorrector::Model_A(const std::vector<double>& par,
                              double raw) const {
    return(par[0] + 
	   par[1] / (par[2] + raw) + 
	   par[3] * exp(-raw / par[4]));
}

double WalkCorrector::Model_B1(const std::vector<double>& par,
                               double raw) const {
    return(par[0] + 
	   (par[1] + par[2] / (raw + 1.0)) *
           exp(-raw / par[3]));
}

double WalkCorrector::Model_B2(const std::vector<double>& par,
                               double raw) const {
    return(par[0] + 
	   par[1] * exp(-raw / par[2]));
}

double WalkCorrector::Model_VS(const std::vector<double> &par, 
			       double raw) const {
    if(raw < 175)
	return(1.09099*log(raw)-7.76641);
    if(raw > 3700)
	return(0.0);
    return(-(9.13743e-12)*pow(raw,3.) + (1.9485e-7)*pow(raw,2.)
	   -0.000163286*raw-2.13918);
}

double WalkCorrector::Model_VB(const std::vector<double> &par,
			       double raw) const {
    return(-(1.07908*log10(raw)-8.27739));
}

double WalkCorrector::Model_VD(const std::vector<double> &par,
			       double raw) const {
    return(92.7907602830327 * exp(-raw/186091.225414275) +
	   0.59140785215161 * exp(raw/2068.14618331387) -
	   95.5388835298589);
}

double WalkCorrector::Model_VM(const std::vector<double> &par,
			       double raw) const {
    return(0.0);
}

double WalkCorrector::Model_VL(const std::vector<double> &par,
			       double raw) const {
    return(0.0);
}
