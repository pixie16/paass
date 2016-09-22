/*! \file HighResTimingData.hpp
 *  \brief Class to hold all of the information for high resolution timing
 *  \author S. V. Paulauskas
 *  \date November 9, 2014
 */
#ifndef __HIGHRESTIMINGDATA_HPP__
#define __HIGHRESTIMINGDATA_HPP__

#include "ChanEvent.hpp"
#include "Globals.hpp"

//! Class for holding information for high resolution timing. All times more
//! precise than the filter time will be in nanoseconds (phase, highResTime).
class HighResTimingData {
public:
    /** Default constructor */
    HighResTimingData() {};
    /** Default destructor */
    virtual ~HighResTimingData() {};

    /** Constructor using the channel event
    * \param [in] chan : the channel event for grabbing values from */
    HighResTimingData(ChanEvent *chan) {chan_ = chan;}

    /** Calculate the energy from the time of flight, using a correction
    * \param [in] tof : The time of flight to use for the calculation in ns
    * \param [in] z0 : The perpendicular distance between the bar and the source in cm
    * \return The particle energy in MeV*/
    double CalcEnergy(const double &tof, const double &z0) {
        return((0.5*Globals::get()->neutronMass()*
                pow((z0/tof)/Globals::get()->speedOfLight(), 2)));
    }

    /** \return The channel event that holds most of our information */
    const ChanEvent* GetChan(void) const {return(chan_);}

    /** \return True if maxval,tqdc and sigmaBaseline were not NAN */
    bool GetIsValid() const {
        if(!std::isnan(chan_->GetTrace().GetValue("maxval")) &&
           !std::isnan(chan_->GetTrace().GetValue("qdc")) &&
           !std::isnan(chan_->GetTrace().GetValue("sigmaBaseline")) ) {
            return(true);
        }else
            return(false);
        return(false);
    }

    ///\return the CFD source trigger bit
    bool GetCfdSourceBit() const { return(chan_->GetCfdSourceBit());}
    /** \return The current value of aveBaseline_ */
    double GetAveBaseline() const { return(chan_->GetTrace().GetValue("baseline")); }
    /** \return The current value of discrimination_ */
    double GetDiscrimination() const { return(chan_->GetTrace().GetValue("discrim")); }
    /** \return The current value of highResTime_ */
    double GetHighResTime() const { return(chan_->GetHighResTime()); }
    /** \return The current value of maxpos_ */
    double GetMaximumPosition() const { return(chan_->GetTrace().GetValue("maxpos")); }
    /** \return The current value of maxval_ */
    double GetMaximumValue() const { return(chan_->GetTrace().GetValue("maxval")); }
    /** \return The current value of numAboveThresh_  */
    int GetNumAboveThresh() const {
        return(chan_->GetTrace().GetValue("numAboveThresh"));
    }
    /** \return The current value of phase_ in nanoseconds*/
    double GetPhase() const {
        return(chan_->GetTrace().GetValue("phase") *
               Globals::get()->clockInSeconds() * 1e9);
    }
    /** \return The pixie Energy */
    double GetFilterEnergy() const { return(chan_->GetEnergy()); }
    /** \return The pixie Energy */
    double GetFilterTime() const { return(chan_->GetTime()); }
    /** \return The current value of snr_ */
    double GetSignalToNoiseRatio() const {
	return(20*log10(chan_->GetTrace().GetValue("maxval") /
			chan_->GetTrace().GetValue("sigmaBaseline")));
    }
    /** \return The current value of stdDevBaseline_  */
    double GetStdDevBaseline() const {
        return(chan_->GetTrace().GetValue("sigmaBaseline"));
    }

    /** \return Get the trace associated with the channel */
    const Trace* GetTrace() const { return(&chan_->GetTrace()); }

    /** \return The current value of tqdc_ */
    double GetTraceQdc() const {
        return(chan_->GetTrace().GetValue("qdc"));
    }
    /** \return Walk corrected time  */
    double GetCorrectedTime() const {
        return(chan_->GetCorrectedTime());
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
        s.time = GetHighResTime();
        s.abase = GetAveBaseline();
        s.sbase = GetStdDevBaseline();
        s.wtime = GetCorrectedTime();
        s.phase = GetPhase();
        s.snr = GetSignalToNoiseRatio();
        s.qdc = GetTraceQdc();
        s.id = chan_->GetChanID().GetLocation();
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
private:
    ChanEvent *chan_; //!< a pointer to the channel event for the high res time
};

/** Defines a map to hold timing data for a channel. */
typedef std::map<TimingDefs::TimingIdentifier, HighResTimingData> TimingMap;
#endif // __HIGHRESTIMINGDATA_HPP__
