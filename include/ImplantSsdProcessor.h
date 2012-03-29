/** \file ImplantSsdProcessor.h
 *
 * Header file for an SSD detector for implantation and correlation
 */

#ifndef __IMPLANT_SSD_PROCESSOR_H_
#define __IMPLANT_SSD_PROCESSOR_H_

#include "EventProcessor.h"

// forward declarations
class EventInfo;
class RawEvent;
enum  EventInfo::EEventTypes;
enum  Correlator::EConditions;
/**
 * \brief Handles detectors of type ssd:implant
 */
class ImplantSsdProcessor : public EventProcessor 
{
 private:
    static const double cutoffEnergy; ///< cutoff energy for implants versus decays
    static const double implantTof;   ///< minimum time-of-flight for an implant
    static const double goodAlphaCut; ///< interesting alpha energy
    static const double fissionThresh; ///< minimum energy for a fission event

    static const unsigned int numTraces = 100;

    unsigned int fastTracesWritten;
    unsigned int highTracesWritten;

    EventInfo::EEventTypes SetType(EventInfo &info) const;
    void PlotType(EventInfo &info, int pos, Correlator::EConditions cond);
 public:
    ImplantSsdProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
};

#endif // __IMPLANT_SSD_PROCESSOR_H_
