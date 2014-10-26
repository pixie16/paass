/** \file SsdBetaProcessor.hpp
 *
 * Header file for DSSD analysis
 */

#ifndef __SSD_PROCESSOR_H_
#define __SSD_PROCESSOR_H_

#include "EventProcessor.h"

class DetectorSummary;
class RawEvent;

/**
 * \brief Handles detectors of type dssd_front and dssd_back
 */
class SsdProcessor : public EventProcessor
{
 private:
    DetectorSummary *ssdSummary; ///< all detectors of type dssd_front

 public:
    SsdProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
};

#endif // __SSD_POCESSOR_H_
