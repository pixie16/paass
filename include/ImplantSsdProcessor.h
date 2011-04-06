/** \file ImplantSsdProcessor.h
 *
 * Header file for an SSD detector for implantation and correlation
 */

#ifndef __IMPLANT_SSD_PROCESSOR_H_
#define __IMPLANT_SSD_PROCESSOR_H_

#include "EventProcessor.h"

class RawEvent;

/**
 * \brief Handles detectors of type ssd:implant
 */
class ImplantSsdProcessor : public EventProcessor 
{
 private:
    static const double cutoffEnergy; ///< cutoff energy for implants versus decays
 public:
    ImplantSsdProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
};

#endif // __IMPLANT_SSD_PROCESSOR_H_
