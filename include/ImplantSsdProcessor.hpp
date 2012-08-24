/** \file ImplantSsdProcessor.hpp
 * \brief Header file for an SSD detector for implantation and correlation
 */

#ifndef __IMPLANT_SSD_PROCESSOR_HPP_
#define __IMPLANT_SSD_PROCESSOR_HPP_

#include "EventProcessor.hpp"

#include "Correlator.hpp"

// forward declarations
class RawEvent;

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
    void PlotType(EventInfo &info, int loc, Correlator::EConditions cond);
    void Correlate(Correlator &corr, EventInfo &info, int location);
 public:
    ImplantSsdProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
};

#endif // __IMPLANT_SSD_PROCESSOR_HPP_
