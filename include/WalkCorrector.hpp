/** \file WalkCorrector.hpp
 * \brief A Class to handle walk corrections for channels
 * \author K. A. Miernik
 * \date January 22, 2013
 */
#ifndef WALKCORRECTOR_H
#define WALKCORRECTOR_H

#include <vector>
#include <map>
#include <string>

#include "Identifier.hpp"

/** A list of known walk correction models (functions). */
enum WalkModel {
    none, A, B1, B2, VS, VM, VL, VB, VD
};

/** \brief This structure holds walk calibration model identfier and
 * vector of parameters needed for the function. */
struct CorrectionParams {
    WalkModel model; //!< The walk model that is used for the params
    double min; //!< minimum of range for the correction
    double max;//!< maximum of range for the correction
    std::vector<double> parameters;//!< coefficients for function
};

/** \brief Class to correct channels for walk in the onboard filters.
 *
 * The purpose of the WalkCorrector class is to correct certain channels
 * for the walk (connected with the detector response, usually the lower the
 * event energy, the slower is the response). The resulting correction
 * should be subtracted from the raw time in order to compensate the slower
 * response.
 */
class WalkCorrector {
public:
    /** Default Constructor */
    WalkCorrector() {};

    /** Add new channel, identified by chanID to the list of
     * known channels and their calibration model, range and parameters
     * \param[in] chanID : The ID to add to the known list of corrections
     * \param[in] model : the model to use to correct the channel
     * \param[in] min : The lower bound of the correction range
     * \param[in] max : The upper bound of the correction range
     * \param[in] par : The vector of parameters to use for the calibration */
    void AddChannel(const Identifier& chanID, const std::string model,
                    double min, double max, const std::vector<double>& par);

    /** Returns time correction that should be subtracted from
     * the raw time. The channel is identified by Identifier class,
     * the correction is return accordingly to the selected model
     * and parameters for the particular channel.
     * If channel has not been added to the list,
     * it returns 0 as a default value.
     * \param [in] chanID : The channel identifier to get
     * \param [in] raw : The raw value to perform the correction on
     * \return The walk corrected value of raw */
    double GetCorrection(Identifier& chanID, double raw) const;

private:
    /** Map where key is a channel Identifier
     * and value is a vector holding struct with calibration range
     * and walk correction model and parameters. */
    std::map<Identifier, std::vector<CorrectionParams> > channels_;

    /** \return always 0.
     * Use if you want to switch off the correction. Also not adding
     * the channel to the list results in returning 0 from GetCorrection
     * function. */
    double Model_None() const;

    /** This model describes the correction as a
     * function of raw energy (ch number):
     * f(x) = a0 + a1 / (a2 + x) + a3 * exp(-x / a4)
     * the returned value is in 'natural' pixie units
     * Developed for 85,86Ga experiment
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return The corrected time in pixie units */
    double Model_A(const std::vector<double>& par, double raw) const;

    /** This model was developed for the 93Br experiment
     * f(x) = a0 + a1 * x + a2 * x^2 + a3 * x^3 +
     *        a4 / (1 + exp((x - a5) / a6))
     * the returned value is in 'natural' pixie units
     *
     * This function is intended for low energy part, for high energy
     * part use B2 model.
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return the corrected time in pixie units */
    double Model_B1(const std::vector<double>& par, double raw) const;

    /** This function is the second part of 'B' model developed
     * for the 93Br experiment
     * f(x) = a0 + a1 * x + (a2 + a3 * x + a4 * x^2) * exp(-x / a5)
     * the returned value is in 'natural' pixie units
     *
     * This function is intended for high energy part, for low energy
     * part use B1 model.
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return corrected time in pixie units */
    double Model_B2(const std::vector<double>& par, double raw) const;

    /** The correction for Small VANDLE bars 
     * the returned value is in ns
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return corrected time in ns */
    double Model_VS(const std::vector<double>& par, double raw) const;
    /** The correction for Medium VANDLE bars 
     * the returned value is in ns
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return corrected time in ns */
    double Model_VM(const std::vector<double>& par, double raw) const;
    /** The correction for Large VANDLE bars 
     * the returned value is in ns
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return corrected time in ns */
    double Model_VL(const std::vector<double>& par, double raw) const;
    /** The correction for betas used with VANDLE
     * the returned value is in ns
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return corrected time in ns */
    double Model_VB(const std::vector<double>& par, double raw) const;
    /** The correction for Small VANDLE bars in RevD
     * the returned value is in ns
     * \param [in] par : the vector of parameters for calibration
     * \param [in] raw : the raw value to calibrate
     * \return corrected time in ns */
    double Model_VD(const std::vector<double>& par, double raw) const;
};
#endif
