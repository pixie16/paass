/** \file SsdBetaProcessor.hpp
 *
 * Header file for SsdBeta analysis
 */
#ifndef __SSDBETAPROCESSOR_HPP_
#define __SSDBETAPROCESSOR_HPP_

#include "EventProcessor.h"

class DetectorSummary;
class RawEvent;

//!Handles detectors of type dssd_front and dssd_back (Deprecated - Duplicate!)
class SsdProcessor : public EventProcessor {
private:
    DetectorSummary *ssdSummary; ///< all detectors of type dssd_front
public:
    /** Default Constructor */
    SsdProcessor();
    /** Declare plots for processor */
    virtual void DeclarePlots(void) const;
    /** Process an event
    * \param [in] event : the event to process
    * \return true if the processing was successful */
    virtual bool Process(RawEvent &event);
};
#endif
