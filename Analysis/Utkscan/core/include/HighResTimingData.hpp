/*! \file HighResTimingData.hpp
 *  \brief Class to hold all of the information for high resolution timing
 *  \author S. V. Paulauskas, T. T. King
 *  \date November 9, 2014
 */
#ifndef __HIGHRESTIMINGDATA_HPP__
#define __HIGHRESTIMINGDATA_HPP__

#include "ChanEvent.hpp"
#include "Constants.hpp"
#include "Globals.hpp"

//! Class for holding information for high resolution timing. All times more
//! precise than the filter time will be in nanoseconds (phase, highResTime).
class HighResTimingData : public ChanEvent {
public:
    /** Default constructor */
    HighResTimingData() {};

    /** Default destructor */
    virtual ~HighResTimingData() {};

    /** Constructor using the channel event
    * \param [in] chan : the channel event for grabbing values from */
    HighResTimingData(ChanEvent &evt) : ChanEvent(evt) {}

    /** Calculate the energy from the time of flight, using a correction
    * \param [in] tof : The time of flight to use for the calculation in ns
    * \param [in] z0 : The perpendicular distance between the bar and the source in cm
    * \return The particle energy in MeV*/
    double CalcEnergy(const double &tof, const double &z0) {
        return ((0.5 * Physical::neutronMassInMeVcc *
                 pow((z0 / tof) / Physical::speedOfLightInCmPerNs, 2)));
    }

    ///@return True if the trace was successfully analyzed and we managed to
    /// find a phase.
    bool GetIsValid() const {
        if (GetTrace().HasValidAnalysis() && GetTrace().GetPhase() != 0.0)
            return (true);
        return (false);
    }

    /**This baseline value is calculated from the trace by averaging the
     * points preceding the waveform
        \return The current value of aveBaseline_ .*/
    double GetAveBaseline() const { return GetTrace().GetBaselineInfo().first; }

    /** \return The current value of discrimination_ */
    double GetDiscrimination() const { return GetTrace().GetTailRatio(); }

    /** \return The current value of maxpos_ */
    double GetMaximumPosition() const { return GetTrace().GetMaxInfo().first; }

    /** \return The current value of maxval_ */
    double GetMaximumValue() const { return GetTrace().GetMaxInfo().second; }

    /** \return The current value of phase_ in nanoseconds*/
    double GetPhaseInNs() const {
        return GetTrace().GetPhase() *
               Globals::get()->GetAdcClockInSeconds() * 1e9;
    }

    /** \return The current value of stdDevBaseline_  */
    double GetStdDevBaseline() const {
        return GetTrace().GetBaselineInfo().second;
    }

    /** \return The current value of tqdc_ */
    double GetTraceQdc() const {
        return GetTrace().GetQdc();
    }

#ifdef useroot
    struct HrtRoot {
        double qdc;
        double time;
        double snr;
        double wtime;
        double phase;
        double abase;
        double sbase;
        unsigned int id;
    };

    void FillRootStructure(HrtRoot &s) const {
        s.time = GetHighResTimeInNs();
        s.abase = GetTrace().GetBaselineInfo().first;
        s.sbase = GetTrace().GetBaselineInfo().second;
        s.wtime = GetWalkCorrectedTime();
        s.phase = GetPhaseInNs();
        s.snr = GetTrace().GetSignalToNoiseRatio();
        s.qdc = GetTrace().GetQdc();
        s.id = GetChanID().GetLocation();
    }

    void ZeroRootStructure(HrtRoot &s) const {
        s.time = -9999.;
        s.abase = -9999.;
        s.sbase = -9999.;
        s.wtime = -9999.;
        s.phase = -9999.;
        s.snr = -9999.;
        s.qdc = -9999.;
        s.id = 9999;
    }

#endif
};

/** Defines a map to hold timing data for a channel. */
typedef std::map<TimingDefs::TimingIdentifier, HighResTimingData> TimingMap;
#endif // __HIGHRESTIMINGDATA_HPP__
