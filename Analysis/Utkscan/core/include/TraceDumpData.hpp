/*! \file TraceDumpData.hpp
 *  \brief Class to hold all of the information for high resolution timing
 *  \author S. V. Paulauskas
 *  \date November 9, 2014
 */
#ifndef __TRACEDUMPDATA_HPP__
#define __TRACEDUMPDATA_HPP__

#include "ChanEvent.hpp"
#include "Globals.hpp"

//! Class for dumping basic trace info (qdc, amplitude, baseline stats, ...)
class TraceDumpData : public ChanEvent {
public:
    /** Default constructor */
    TraceDumpData() {};
    /** Default destructor */
    virtual ~TraceDumpData() {};

    /** Constructor using the channel event
    * \param [in] chan : the channel event for grabbing values from */
    TraceDumpData(ChanEvent &evt) : ChanEvent(evt) {}

    /** Calculate the energy from the time of flight, using a correction
    * \param [in] tof : The time of flight to use for the calculation in ns
    * \param [in] z0 : The perpendicular distance between the bar and the source in cm
    * \return The particle energy in MeV*/
    double CalcEnergy(const double &tof, const double &z0) {
        return((0.5*Globals::get()->neutronMass()*
                pow((z0/tof)/Globals::get()->speedOfLight(), 2)));
    }

    /** \return True if maxval,tqdc and sigmaBaseline were not NAN */
    bool GetIsValid() const {
        if(!std::isnan(GetTrace().GetMaxInfo().second) &&
           !std::isnan(GetTrace().GetQdc()) &&
           !std::isnan(GetTrace().GetBaselineInfo().first) ) {
            return(true);
        }
        return(false);
    }

    /** \return The current value of aveBaseline_ */
    double GetAveBaseline() const { return GetTrace().GetBaselineInfo().first; }
    /** \return The current value of discrimination_ */
    double GetDiscrimination() const { return GetTrace().GetTailRatio(); }
    /** \return The current value of maxpos_ */
    double GetMaximumPosition() const { return GetTrace().GetMaxInfo().first; }
    /** \return The current value of maxval_ */
    double GetMaximumValue() const { return GetTrace().GetMaxInfo().second; }


    /** \return The current value of stdDevBaseline_  */
    double GetStdDevBaseline() const {
        return GetTrace().GetBaselineInfo().second;
    }

    /** \return The current value of tqdc_ */
    double GetTraceQdc() const {
        return GetTrace().GetQdc() ;
    }

#ifdef useroot
    struct HrtRoot {
        double qdc;
        double amp;   ///JNH
        double snr;
        double abase;
        double sbase;
        unsigned int id;
    };

    void FillRootStructure(HrtRoot &s) const {
        s.abase = GetTrace().GetBaselineInfo().first;
        s.sbase = GetTrace().GetBaselineInfo().second;
        s.snr = GetTrace().GetSignalToNoiseRatio();
        s.qdc = GetTrace().GetQdc();
        s.amp = GetTrace().GetMaxInfo().second;  ///JNH
        s.id = GetChanID().GetLocation();
    }

    void ZeroRootStructure(HrtRoot &s) const {
        s.abase = -9999.;
        s.sbase = -9999.;
        s.snr = -9999.;
        s.qdc = -9999.;
        s.amp = -9999.;    ///JNH
        s.id = 9999;
    }
#endif
};

/** Defines a map to hold timing data for a channel. */
typedef std::map<TimingDefs::TimingIdentifier, TraceDumpData> TimingMap;
#endif // __TRACEDUMPDATA_HPP__
