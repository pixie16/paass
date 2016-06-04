/** \file ChanEvent.hpp
 * \brief A Class to define what a channel event is
 */
#ifndef __CHANEVENT_HPP
#define __CHANEVENT_HPP

#include <vector>
#include "DetectorLibrary.hpp"
#include "pixie16app_defs.h"
#include "Identifier.hpp"
#include "Globals.hpp"
#include "Trace.hpp"

/*! \brief A channel event
 *
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels
 * identifier, calibrated energies, trace analysis information.
 * Note that this currently stores raw values internally through pixie word types
 *   but returns data values through native C types. This is potentially non-portable.
 */
class ChanEvent {
public:
    /** Default constructor that zeroes all values */
    ChanEvent(){ZeroNums();};

    /** Set the raw energy in case we do not want to extract it from the trace
     * ourselves
     * \param [in] a : the energy to set */
    void SetEnergy(double a) {energy = a;}

    /** Set the calibrated energy
     * \param [in] a : the calibrated energy */
    void SetCalEnergy(double a) {calEnergy = a;}

    /** Set the time
     * \param [in] a : the time to set */
    void SetTime(double a) {time = a;}

    /** Set the Walk corrected time
     * \param [in] a : the walk corrected time */
    void SetCorrectedTime(double a) { correctedTime = a;}

    /** Set the Calibrated time
     * \param [in] a : the calibrated time */
    void SetCalTime(double a) {calTime = a;}

    /** Set the high resolution time (Filter time + phase )
     * \param [in] a : the high resolution time */
    void SetHighResTime(double a) {highResTime =a;}

    /** \return the CFD source bit */ 
    bool GetCfdSourceBit() const {
	return(cfdTrigSource);
    }
    /** \return true if the CFD was forced trigger */ 
    bool CfdForceTrig() const {
	return(cfdForceTrig); 
    }

    double GetEnergy() const {
        return energy;   /**< \return the raw energy */
    }
    double GetCalEnergy() const {
        return calEnergy;   /**< \return the calibrated energy */
    }
    double GetCorrectedTime() const {
        return correctedTime;   /**< \return the corrected time */
    }
    double GetTime() const {
        return time;   /**< \return the raw time in clock ticks*/
    }
    double GetCalTime() const {
        return calTime;   /**< \return the calibrated time */
    }
    double GetHighResTime() const {
        return highResTime;   /**< \return the high-resolution time in ns*/
    }
    double GetEventTime() const {
        return eventTime;   /**< \return the event time */
    }
    const Trace& GetTrace() const {
        return trace;   /**< \return a reference to the trace */
    }
    Trace& GetTrace() {
        return trace;   /** \return a reference which can alter the trace */
    }
    unsigned long GetTrigTime() const {
        return trigTime;   /**< \return the channel trigger time */
    }
    unsigned long GetEventTimeLo() const {
        return eventTimeLo;   /**< \return the lower 32 bits of event time */
    }
    unsigned long GetEventTimeHi() const {
        return eventTimeHi;   /**< \return the upper 32 bits of event time */
    }
    unsigned long GetRunTime0() const {
        return runTime0;   /**< \return the lower bits of run time */
    }
    unsigned long GetRunTime1() const {
        return runTime1;   /**< \return the middle bits of run time */
    }
    unsigned long GetRunTime2() const {
        return runTime2;   /**< \return the higher bits of run time */
    }
    bool IsPileup() const {
        return pileupBit;   //!< \return true if channel is pileup
    }
    bool IsSaturated() const { /**< \return whether the trace is saturated */
        return saturatedBit;
    }

    //! \return The identifier in the map for the channel event
    const Identifier& GetChanID() const;
    /** \return the channel id defined as pixie module # * 16 + channel number */
    int GetID() const;
    /** \return The Onboard QDC value at i
     * \param [in] i : the QDC number to obtain, possible values [0,7] */
    unsigned long GetQdcValue(int i) const;

    /** Channel event zeroing
     * All numerical values are set to -1, and the trace,
     * and traceinfo vectors are cleared and the channel
     * identifier is zeroed using its identifier::zeroid method. */
    void ZeroVar();
private:
    double energy;             /**< Raw channel energy */
    double calEnergy;          /**< Calibrated channel energy,
                  calibration performed in ThreshAndCal
                  function in the detector_driver.cpp */
    double calTime;            /**< Calibrated time, currently unused */
    double correctedTime;      /**< Energy-walk corrected time */
    double highResTime;        /**< timing resolution less than 1 adc sample */
    Trace trace;               /**< Channel trace if present */
    pixie::word_t trigTime;    /**< The channel trigger time, trigger time and the lower 
				  32 bits of the event time are not necessarily the 
				  same but could be separated by a constant value.*/
    pixie::word_t cfdTime;     /**< CFD trigger time in units of 1/256 pixie clock ticks */
    pixie::word_t eventTimeLo; /**< Lower 32 bits of pixie16 event time */
    pixie::word_t eventTimeHi; /**< Upper 32 bits of pixie16 event time */
    pixie::word_t runTime0;    /**< Lower bits of run time */
    pixie::word_t runTime1;    /**< Upper bits of run time */
    pixie::word_t runTime2;    /**< Higher bits of run time */
    static const int numQdcs = 8;     /**< Number of QDCs onboard */
    pixie::word_t qdcValue[numQdcs];  /**< QDCs from onboard */

    double time;               /**< Raw channel time, 64 bit from pixie16 channel event time */
    double eventTime;          /**< The event time recorded by Pixie */
    int modNum;             /**< Module number */
    int chanNum;            /**< Channel number */

    bool virtualChannel; /**< Flagged if generated virtually in Pixie DSP */
    bool pileupBit;      /**< Pile-up flag from Pixie */
    bool saturatedBit;   /**< Saturation flag from Pixie */
    bool cfdForceTrig;   //!< CFD was forced to trigger
    bool cfdTrigSource;  //!< The ADC that the CFD/FPGA synched with

    void ZeroNums(void); /**< Zero members which do not have constructors associated with them */

    /** Make the front end responsible for reading the data able to set the
     * channel data directly from ReadBuffDataA - REVISION A */
    friend int ReadBuffDataA(pixie::word_t *, unsigned long *, std::vector<ChanEvent *> &);
    /** Make the front end responsible for reading the data able to set the
     * channel data directly from ReadBuffDataA - REVISION D */
    friend int ReadBuffDataD(pixie::word_t *, unsigned long *, std::vector<ChanEvent *> &);
    /** Make the front end responsible for reading the data able to set the
     * channel data directly from ReadBuffDataA - REVISION F */
    friend int ReadBuffDataF(pixie::word_t *, unsigned long *, std::vector<ChanEvent *> &);
};

/** Sort by increasing corrected time
 * \param [in] a : the left hand side for comparison
 * \param [in] b : the right hand side for comparison
 * \return True if LHS is less the RHS */
bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b);
/** Sort by increasing raw time
 * \param [in] a : the left hand side for comparison
 * \param [in] b : the right hand side for comparison
 * \return True if LHS is less the RHS*/
bool CompareTime(const ChanEvent *a, const ChanEvent *b);
#endif
