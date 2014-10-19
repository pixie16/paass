#ifndef WALKCORRECTOR_H
#define WALKCORRECTOR_H

#include <vector>
#include <map>
#include <string>

#include "ChanIdentifier.hpp"

/** A list of known walk correction models (functions). */
enum WalkModel {
    walk_none,
    walk_A,
    walk_B1, 
    walk_B2
};

/** This structure holds walk calibration model identfier and
 * vector of parameters needed for the function. */
struct CorrectionParams {
    WalkModel model;
    double min;
    double max;
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

        /** Add new channel, identified by chanID to the list of 
         * known channels and their calibration model, range and parameters */
        void AddChannel(const Identifier& chanID,
                        const std::string model,
                        double min, double max, 
                        const std::vector<double>& par);

        /** Returns time correction that should be subtracted from 
         * the raw time. The channel is identified by Identifier class,
         * the correction is return accordingly to the selected model
         * and parameters for the particular channel.
         * If channel has not been added to the list,
         * it returns 0 as a default value.*/
        double GetCorrection(Identifier& chanID, double raw) const;

    private:
        /** Map where key is a channel Identifier 
         * and value is a vector holding struct with calibration range
         * and walk correction model and parameters.*/
        std::map<Identifier, std::vector<CorrectionParams> > channels_;

        /** Returns always 0. 
         * Use if you want to switch off the correction. Also not adding
         * the channel to the list results in returning 0 from GetCorrection function.*/
        double Model_None() const;

        /** This model describes the correction as a 
         * function of raw energy (ch number):
         * f(x) = a0 + a1 / (a2 + x) + a3 * exp(-x / a4)
         * the returned value is in 'natural' pixie units
         * Developed for 85,86Ga experiment
         */
        double Model_A(const std::vector<double>& par, double raw) const;

        /** This model was developed for the 93Br experiment
         * f(x) = a0 + a1 * x + a2 * x^2 + a3 * x^3 + 
         *        a4 / (1 + exp((x - a5) / a6))
         * the returned value is in 'natural' pixie units
         *
         * This function is intended for low energy part, for high energy
         * part use B2 model.
         */
        double Model_B1(const std::vector<double>& par, double raw) const;

        /** This function is the second part of 'B' model developed
         * for the 93Br experiment
         * f(x) = a0 + a1 * x + (a2 + a3 * x + a4 * x^2) * exp(-x / a5)
         * the returned value is in 'natural' pixie units
         *
         * This function is intended for high energy part, for low energy
         * part use B1 model.
         */
        double Model_B2(const std::vector<double>& par, double raw) const;
};


#endif
