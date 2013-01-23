#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <vector>
#include <map>
#include <string>

#include "ChanIdentifier.hpp"

/** A list of known walk correction models (functions). */
enum CalibrationModel {
    Raw,
    Poly
};

/** This structure holds walk calibration model identfier, range
 * of calibration and vector of parameters needed for the function. */
struct CalibrationFactor {
    CalibrationModel model;
    double min;
    double max;
    std::vector<double> parameters;
};


/** The purpose of the Calibrator class is to correct certain channels
 * for the walk (connected with the detector response, usually the lower the
 * event energy, the slower is the response). The resulting correction
 * should be subtracted from the raw time in order to compensate the slower response.
 */
class Calibrator {
    public:
        Calibrator() {}

        /** Add new channel, identified by chanID to the list of known channels and 
         * their calibration model and parameters */
        void AddChannel(const Identifier& chanID, const std::string model,
                        double min, double max,
                        const std::vector<double>& par);

        /** Returns calibrated energy for the channel indetified by chanID.*/
        double GetCalEnergy(Identifier& chanID, double ch) const;

    private:
        /** Map where key is a channel Identifier and value is the struct holding correction
         * model and parameters.*/
        std::map<Identifier, std::vector<CalibrationFactor> > channels_;

        /** Returns always the raw channel number. Use if you want to switch off the calibration.*/
        double Model_Raw(double ch) const;

        /** Polynomial calibration, where parameters are assumed to be sorted from the lowest order
         * to the highest
         * f(x) = par0 + par1 * x + par2 * x^2 + ...
         */
        double Model_Poly(const std::vector<double>& par, double ch) const;
};


#endif
