/** \file SsdProcessor.h
 *
 * Header file for SSD analysis
 */

#ifndef __SSD_PROCESSOR_H_
#define __SSD_PROCESSOR_H_

#include "EventProcessor.h"

class RawEvent;

/**
 * \brief Handles detectors of type ssd
 */
class SsdProcessor : public EventProcessor 
{
  public:
    SsdProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
};

#endif // __SSD_PROCESSOR_H_
