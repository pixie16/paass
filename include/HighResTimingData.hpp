/*! \file HighResTimingData.hpp
 *  \brief Class to hold all of the information for high resolution timing
 *  \author S. V. Paulauskas
 *  \date November 9, 2014
 */
#ifndef __HIGHRESTIMINGDATA_HPP__
#define __HIGHRESTIMINGDATA_HPP__

#include "ChanEvent.hpp"
#include "Globals.hpp"
#include "Trace.hpp"

//! Class for holding information for high resolution timing
class HighResTimingData {
public:
    /** Default constructor */
    HighResTimingData() {};
    /** Default destructor */
    virtual ~HighResTimingData() {};

    /** Constructor using the channel event
    * \param [in] chan : the channel event for grabbing values from */
    HighResTimingData(ChanEvent *chan) {
        trace_ = &chan->GetTrace();
        highResTime_ = chan->GetHighResTime()*1e9;
        pixieEnergy_ = chan->GetEnergy();

        snr_ =
            20*log10(trace_->GetValue("maxval") /
                     trace_->GetValue("sigmaBaseline"));

        if(!std::isnan(trace_->GetValue("maxval")) &&
           !std::isnan(trace_->GetValue("phase")) &&
           !std::isnan(trace_->GetValue("tqdc")) &&
           !std::isnan(trace_->GetValue("sigmaBaseline")) ) {
            isValidData_ = true;
        }else
            isValidData_ = false;
    };

    /** Calculate the energy from the time of flight, using a correction
    * \param [in] tof : The time of flight to use for the calculation in ns
    * \param [in] z0 : The perpendicular distance between the bar and the source in cm
    * \return The particle energy in MeV*/
    double CalcEnergy(const double &tof, const double &z0) {
        return((0.5*Globals::get()->neutronMass()*
        pow((z0/tof)/Globals::get()->speedOfLight(), 2)));
    }

    /** \return The current value of isValidData_ */
    bool GetIsValidData() const { return isValidData_; };
    /** \return The current value of aveBaseline_ */
    double GetAveBaseline() const { return trace_->GetValue("baseline"); };
    /** \return The current value of discrimination_ */
    double GetDiscrimination() const { return trace_->GetValue("discrim"); };
    /** \return The current value of highResTime_ */
    double GetHighResTime() const { return highResTime_; };
    /** \return The current value of maxpos_ */
    double GetMaximumPosition() const { return trace_->GetValue("maxpos"); }
    /** \return The current value of maxval_ */
    double GetMaximumValue() const { return trace_->GetValue("maxval"); };
    /** \return The current value of numAboveThresh_  */
    int GetNumAboveThresh() const { return trace_->GetValue("numAboveThresh");; };
    /** \return The current value of phase_ */
    double GetPhase() const { return trace_->GetValue("phase")*
                        (Globals::get()->clockInSeconds()*1e+9); };
    /** \return The pixie Energy */
    double GetPixieEnergy() const { return(pixieEnergy_); };
    /** \return The current value of snr_ */
    double GetSignalToNoiseRatio() const { return snr_; };
    /** \return The current value of stdDevBaseline_  */
    double GetStdDevBaseline() const { return trace_->GetValue("sigmaBaseline"); };
    /** \return Get the trace associated with the channel */
    Trace* GetTrace() const { return trace_; };
    /** \return The current value of tqdc_ */
    double GetTraceQdc() const { return trace_->GetValue("tqdc") /
        Globals::get()->qdcCompression(); };
    /** \return The current value of walk_ */
    double GetWalk() const { return trace_->GetValue("walk"); };
    /** \return Walk corrected time  */
    double GetWalkCorrectedTime() const
        { return highResTime_ - trace_->GetValue("walk"); };

    /** Set dataValid_
     * \param val New value to set */
    void SetDataValid(const bool &val) { isValidData_ = val; };

private:
    Trace *trace_; //!< the trace for the channel

    bool isValidData_; //!< Member variable "dataValid_"
    double highResTime_; //!< Member variable "highResTime_"
    double snr_; //!< Member variable "snr_"
    double pixieEnergy_; //!< The energy from pixie
};

/** Defines a map to hold timing data for a channel. */
typedef std::map<TimingDefs::TimingIdentifier, HighResTimingData> TimingMap;
#endif // __HIGHRESTIMINGDATA_HPP__
