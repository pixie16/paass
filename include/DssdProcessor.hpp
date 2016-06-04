/** \file DssdProcessor.hpp
 * \brief Header file for DSSD analysis
 */

#ifndef __DSSDPROCESSOR_HPP_
#define __DSSDPROCESSOR_HPP_

#include "EventProcessor.hpp"

class DetectorSummary;
class RawEvent;

///Handles detectors of type dssd_front and dssd_back
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

#endif // __DSSDPOCESSOR_HPP_
