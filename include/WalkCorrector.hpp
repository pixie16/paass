#ifndef WALKCORRECTOR_H
#define WALKCORRECTOR_H

#include <vector>
#include <map>
#include <string>

#include "ChanIdentifier.hpp"

/** A list of known walk correction models (functions). */
enum WalkModel {
    None,
    A
};

/** This structure holds walk correction model identfier and
 * vector of parameters needed for the function. */
struct CorrectionFactor {
    WalkModel model;
    std::vector<double> parameters;
};


/** The purpose of the WalkCorrector class is to correct certain channels
 * for the walk (connected with the detector response, usually the lower the
 * event energy, the slower is the response). The resulting correction
 * should be subtracted from the raw time in order to compensate the slower response.
 */
class WalkCorrector {
    public:
        WalkCorrector() {}

        /** Add new channel, identified by chanID to the list of known channels and 
         * their correction model and parameters */
        void AddChannel(const Identifier& chanID, const std::string model,
                        const std::vector<double>& par);

        /** Returns time correction that should be subtracted from the raw time.
         * The channel is identified by Identifier class, the correction is return 
         * accordingly to the stored model and parameters for the particular channel.
         * If channel has not been added to the list, it returns 0 as a default value.*/
        double GetCorrection(Identifier& chanID, double ch) const;

    private:
        /** Map where key is a channel Identifier and value is the struct holding correction
         * model and parameters.*/
        std::map<Identifier, CorrectionFactor> channels_;

        /** Returns always 0. Use if you want to switch off the correction. Also not adding
         * the channel to the list results in returning 0 from GetCorrection function.*/
        double Model_None() const;

        /** This model describes the correction as a function of raw energy (ch number):
         * f(x) = a0 + a1 / (a2 + x) + a3 * exp(-x / a4)
         * the returned value is in 'natural' pixie units
         */
        double Model_A(const std::vector<double>& par, double ch) const;
};


#endif
