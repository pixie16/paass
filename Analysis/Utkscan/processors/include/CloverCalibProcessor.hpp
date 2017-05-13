/*! \file CloverCalibProcessor.hpp
 * \brief Processor for germanium detectors - calibration
 * \author K. A. Miernik
 */
#ifndef __CloverCalibProcessor_HPP_
#define __CloverCalibProcessor_HPP_

#include <map>
#include <vector>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "CloverProcessor.hpp"

//! A class to handle the calibrations for the Ge Processor
class CloverCalibProcessor : public CloverProcessor {
public:
    /** Constructor that takes the threshold and the ratios for
     * \param [in] gammaThreshold : the gamma threshold for the calibration
     * \param [in] lowRatio : the low ratio for the calibration
     * \param [in] highRatio : the high ratio for the calibration */
    CloverCalibProcessor(double gammaThreshold, double lowRatio, double highRatio);

    /** Performs the preprocessing, which cannot depend on other processors
    * \param [in] event : the event to process
    * \return true if preprocessing was successful */
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
     * \param [in] event : the event to process
     * \return true if successful */
    virtual bool Process(RawEvent &event);

    /** Declare the plots for the processor */
    virtual void DeclarePlots(void);
};

#endif // __CloverCalibProcessor_HPP_
