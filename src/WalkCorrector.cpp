#include <cmath>
#include "WalkCorrector.hpp"
#include "Exceptions.hpp"

using namespace std;


void WalkCorrector::AddChannel(const Identifier& chanID,
                               const std::string model,
                               const std::vector<double>& par) {
    if (channels_.find(chanID) != channels_.end()) {
        stringstream ss;
        ss << "WalkCorrector: attempt to add more then " 
           << " one correction function for the channel " 
           << chanID.GetPlaceName();
        throw GeneralException(ss.str());
    }

    CorrectionFactor cf;
    unsigned required_length = 0;
    if (model == "A") {
        cf.model = A;
        required_length = 5;
    } else if (model == "None") {
        cf.model = None;
        required_length = 0;
    } else
        cf.model = None;
    
    if (par.size() != required_length) {
        stringstream ss;
        ss << "WalkCorrector: wrong number of parameters, expected "
           << required_length << " but " << par.size() << " were found.";
        throw GeneralException(ss.str());
    }

    for (vector<double>::const_iterator it = par.begin(); it != par.end();
         ++it) {
        cf.parameters.push_back(*it);
    }


    channels_[chanID] = cf;
}

double WalkCorrector::GetCorrection(Identifier& chanID, double ch) const {
    map<Identifier, CorrectionFactor>::const_iterator ch_it =
        channels_.find(chanID);
    if (ch_it != channels_.end()) {
        WalkModel model = ch_it->second.model;
        switch(model) {
            case A: 
                return Model_A(ch_it->second.parameters, ch);
                break;
            case None:
                return Model_None();
                break;
            default: return 0;
        }
    }
    return 0;
}

double WalkCorrector::Model_None() const {
    return 0;
}

double WalkCorrector::Model_A(const std::vector<double>& par, double ch) const {
    if ((par[2] + ch <= 0) || ch <= 0)
        return 0;

    return par[0] + par[1] / (par[2] + ch) + par[3] * exp(-ch / par[4]);
}
