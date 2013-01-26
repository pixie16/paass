#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <vector>
#include <map>
#include <string>

#include "ChanIdentifier.hpp"

/** A list of known walk correction models (functions). Add here a new name
 * if you need a different model. Then add a new function to the Calibrator
 * class, and and else-if loop to the AddChannel and GetCalEnergy functions. */
enum CalibrationModel {
    cal_raw,
    cal_linear,
    cal_quadratic,
    cal_polynomial,
    cal_hyplin
};

/** This structure holds walk calibration model identfier, range
 * of calibration and vector of parameters needed for the function. */
struct CalibrationParams {
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

        /** Add new channel, identified by chanID,
         * to the list of known channels and 
         * their calibration model and parameters */
        void AddChannel(const Identifier& chanID, const std::string model,
                        double min, double max,
                        const std::vector<double>& par);

        /** Returns calibrated energy for the channel indetified by chanID.*/
        double GetCalEnergy(const Identifier& chanID, double raw) const;

    private:
        /** Map where key is a channel Identifier 
         * and value is a vector holding struct with calibration range
         * and calibration model and parameters.*/
        std::map<Identifier, std::vector<CalibrationParams> > channels_;

        /** Returns always the raw channel number. Use if you want to switch off the calibration.*/
        double ModelRaw(double raw) const;

        /** Linear calibration, parameters are assumed to be sorted 
         * in order par0, par1 
         * f(x) = par0 + par1 * x 
         */
        double ModelLinear(const std::vector<double>& par,
                                double raw) const;

        /** Quadratic calibration, parameters are assumed to be sorted 
         * in order par0, par1, par2
         * f(x) = par0 + par1 * x  + par2 * x^2
         */
        double ModelQuadratic(const std::vector<double>& par,
                                double raw) const;

        /** Polynomial calibration, where parameters are assumed to be sorted
         * from the lowest order to the highest
         * f(x) = par0 + par1 * x + par2 * x^2 + ...
         *
         * Note that this model covers also Linear and Quadratic, however
         * it is slower due to looping over unknown apriori number
         * of parameters.
         */
        double ModelPolynomial(const std::vector<double>& par,
                                double raw) const;

        /** Linear plus hyperbolic calibration,
         * parameters are assumed to be sorted
         * from the lowest order to the highest
         * f(x) = par0 / x + par1 + par2 * x
         */
        double ModelHypLin(const std::vector<double>& par,
                           double raw) const;
};


#endif
