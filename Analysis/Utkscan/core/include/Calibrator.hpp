/** \file Calibrator.hpp
 * \brief Class to handle energy calibrations for the channels
 * \author K. A. Miernik
 * \date 2012
 */
#ifndef __CALIBRATOR_HPP__
#define __CALIBRATOR_HPP__

#include <vector>
#include <map>
#include <string>

#include "Identifier.hpp"

/** A list of known walk correction models (functions). Add here a new name
 * if you need a different model. Then add a new function to the Calibrator
 * class, and and else-if loop to the AddChannel and GetCalEnergy functions. */
enum CalibrationModel {
    cal_raw,
    cal_off,
    cal_linear,
    cal_quadratic,
    cal_cubic,
    cal_polynomial,
    cal_hyplin,
    cal_exp
};

/** \brief This structure holds walk calibration model identfier, range
 * of calibration and vector of parameters needed for the function. */
struct CalibrationParams {
    CalibrationModel model; //!< Calibration model to use
    double min;//!< Minimum of range for calibration
    double max;//!< Maximum of range for calibration
    std::vector<double> parameters; //!< coefficients for calibration eqn.
};

/** \brief Class to handle energy calibrations
 *
 * The Calibrator class returns calibrated energy for the raw channel
 * number. The calibration model and parameters are loaded from Config.xml
 * file (Map section)
 */
class Calibrator {
public:
    /*! Default constructor */
    Calibrator() {};
    /*! Default Destructor */
    ~Calibrator() {};

    /** Add new channel, identified by chanID,
     * to the list of known channels and
     * their calibration model and parameters
     * \param [in] chanID : the channel ID to add to the calibrator
     * \param [in] model : the model to associate with the channel
     * \param [in] min : the minimum value for the range of the calibration
     * \param [in] max : the maximum value for the range of the calibration
     * \param [in] par : the vector of coefficients for the model */
    void AddChannel(const Identifier& chanID, const std::string model,
                    double min, double max,
                    const std::vector<double>& par);

    /** \return calibrated energy for the channel indetified by chanID.
     * \param [in] chanID : the channel ID to get the energy for
     * \param [in] raw : the raw value to use for the calibration */
    double GetCalEnergy(const Identifier& chanID, double raw) const;

private:
    /** Map where key is a channel Identifier
     * and value is a vector holding struct with calibration range
     * and calibration model and parameters.*/
    std::map<Identifier, std::vector<CalibrationParams> > channels_;

    /** Use if you want to switch off the calibration.
     * \param [in] raw : the raw value to calibrate
     * \return the raw channel number. */
    double ModelRaw(double raw) const;

    /** Use if you want to switch off the channel
     * \return 0. */
    double ModelOff() const;

    /** Linear calibration, parameters are assumed to be sorted
     * in order par0, par1
     * f(x) = par0 + par1 * x
     * \param [in] par : the vector of calibration coeffs
     * \param [in] raw : the raw value to calibrate
     * \return Calibrated energy */
    double ModelLinear(const std::vector<double>& par, double raw) const;

    /** Quadratic calibration, parameters are assumed to be sorted
     * in order par0, par1, par2
     * f(x) = par0 + par1 * x  + par2 * x^2
     * \param [in] par : the vector of calibration coeffs
     * \param [in] raw : the raw value to calibrate
     * \return Calibrated energy */
    double ModelQuadratic(const std::vector<double>& par, double raw) const;

    /** Cubic calibration, parameters are assumed to be sorted
     * in order par0, par1, par2, par3
     * f(x) = par0 + par1 * x  + par2 * x^2 + par3 * x^3
     * \param [in] par : the vector of calibration coeffs
     * \param [in] raw : the raw value to calibrate
     * \return Calibrated energy */
    double ModelCubic(const std::vector<double>& par, double raw) const;

    /** Polynomial calibration, where parameters are assumed to be sorted
     * from the lowest order to the highest
     * f(x) = par0 + par1 * x + par2 * x^2 + ...
     *
     * Note that this model covers also Linear and Quadratic, however
     * it is slower due to looping over unknown apriori number
     * of parameters.
     * \param [in] par : the vector of calibration coeffs
     * \param [in] raw : the raw value to calibrate
     * \return Calibrated energy */
    double ModelPolynomial(const std::vector<double>& par, double raw) const;

    /** Linear plus hyperbolic calibration,
     * parameters are assumed to be sorted
     * from the lowest order to the highest
     * f(x) = par0 / x + par1 + par2 * x
     * \param [in] par : the vector of calibration coeffs
     * \param [in] raw : the raw value to calibrate
     * \return Calibrated energy */
         double ModelHypLin(const std::vector<double>& par, double raw) const;

    /** Exponential (for logarithmic preamp)
     * f(x) = par0 * exp(x / par[1]) + par2
     * \param [in] par : the vector of calibration coeffs
     * \param [in] raw : the raw value to calibrate
     * \return Calibrated energy */
    double ModelExp(const std::vector<double>& par, double raw) const;
};
#endif
