/** \file DssdProcessor.hpp
 * \brief Header file for DSSD analysis
 */

#ifndef __DSSD_PROCESSOR_HPP_
#define __DSSD_PROCESSOR_HPP_

#include "EventProcessor.hpp"

class DetectorSummary;
class RawEvent;

/**
 * \brief Handles detectors of type dssd_front and dssd_back
 */
class DssdProcessor : public EventProcessor {
 public:
    DssdProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
 private:
    DetectorSummary *frontSummary; ///< all detectors of type dssd_front
    DetectorSummary *backSummary;  ///< all detectors of type dssd_back
    static const double cutoffEnergy; ///< cutoff energy for implants versus decays
};

#endif // __DSSD_POCESSOR_HPP_
