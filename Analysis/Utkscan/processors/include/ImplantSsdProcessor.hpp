/** \file ImplantSsdProcessor.hpp
 * \brief Header file for an SSD detector for implantation and correlation
 */
#ifndef __IMPLANT_SSD_PROCESSOR_HPP_
#define __IMPLANT_SSD_PROCESSOR_HPP_

#include "EventProcessor.hpp"

class RawEvent;

//! Handles detectors of type ssd:implant
class ImplantSsdProcessor : public EventProcessor {
private:
    static const double cutoffEnergy; ///< cutoff energy for implants versus decays
    static const double implantTof;   ///< minimum time-of-flight for an implant
    static const double goodAlphaCut; ///< interesting alpha energy
    static const double fissionThresh; ///< minimum energy for a fission event

    static const unsigned int numTraces = 100;//!< number of traces

    unsigned int fastTracesWritten;//!< Number of fast traces written
    unsigned int highTracesWritten;//!< Number of high traces written

    /** Sets the event type
     * \param [in] info : the event information to set
     * \return The event types that were set */
    EventInfo::EEventTypes SetType(EventInfo &info) const;

    /** Plots a specific type
     * \param [in] info : the information to plot
     * \param [in] loc : the location to plot
     * \param [in] cond : the conditions for the plotting */
    void PlotType(EventInfo &info, int loc, Correlator::EConditions cond);

    /** Correlates the events
     * \param [in] corr : the correlator
     * \param [in] info : the information to correlate
     * \param [in] location : the location for the correlations */
    void Correlate(Correlator &corr, EventInfo &info, int location);
public:
    /** Default Constructor */
    ImplantSsdProcessor();
    /** Default Destructor */
    ~ImplantSsdProcessor(){};
    /** Declares the plots */
    virtual void DeclarePlots(void);
    /** Process an event
    * \param [in] event : the event to process
    * \return true if the processing was successful */
    virtual bool Process(RawEvent &event);
};
#endif // __IMPLANT_SSD_PROCESSOR_HPP_
