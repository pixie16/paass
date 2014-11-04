/** \file SsdProcessor.hpp
 * \brief Header file for SSD analysis
 */

#ifndef __SSD_PROCESSOR_HPP_
#define __SSD_PROCESSOR_HPP_

#include "EventProcessor.hpp"

class RawEvent;

/**
 * \brief Handles detectors of type ssd
 */
class SsdProcessor : public EventProcessor {
  public:
    /** Default Constructor */
    SsdProcessor(); // no virtual c'tors
    /** Declare plots for processor */
    virtual void DeclarePlots(void);
    /** Process an event
    * \param [in] event : the event to process
    * \return true if the processing was successful */
    virtual bool Process(RawEvent &event);
};
#endif // __SSD_PROCESSOR_HPP_
