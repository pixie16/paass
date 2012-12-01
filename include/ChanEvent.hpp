#ifndef __CHANEVENT_HPP
#define __CHANEVENT_HPP

#include <vector>
#include "DetectorLibrary.hpp"
#include "pixie16app_defs.h"
#include "ChanIdentifier.hpp"
#include "Globals.hpp"
#include "Trace.hpp"

/**
 * \brief A channel event
 * 
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels 
 * identifier, calibrated energies, trace analysis information.
 * Note that this currently stores raw values internally through pixie word types
 *   but returns data values through native C types. This is potentially non-portable. 
 */
class ChanEvent
{
private:
    double energy;             /**< Raw channel energy */
    double calEnergy;          /**< Calibrated channel energy,
				  calibration performed in ThreshAndCal
				  function in the detector_driver.cpp */
    double calTime;            /**< Calibrated time, currently unused */
    double correctedTime;      /**< Energy-walk corrected time */
    double highResTime;        /**< timing resolution less than 1 adc size */
    Trace trace;               /**< Channel trace if present */
    pixie::word_t trigTime;    /**< The channel trigger time, trigger time and the lower 32 bits
				  of the event time are not necessarily the same but could be
				  separated by a constant value.*/
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
    int    modNum;             /**< Module number */
    int    chanNum;            /**< Channel number */

    bool   virtualChannel;     /**< Flagged if generated virtually in Pixie DSP */
    bool   pileupBit;          /**< Pile-up flag from Pixie */
    bool   saturatedBit;       /**< Saturation flag from Pixie */

    void ZeroNums(void);       /**< Zero members which do not have constructors associated with them */
    
    // make the front end responsible for reading the data able to set the channel data directly
    friend int ReadBuffData(pixie::word_t *, unsigned long *, std::vector<ChanEvent *> &);
public:
    static double pixieEnergyContraction; ///< energies from pixie16 are contracted by this number
    void SetEnergy(double a)    {energy = a;}    /**< Set the raw energy in case we want
						    to extract it from the trace ourselves */
    void SetCalEnergy(double a) {calEnergy = a;} /**< Set the calibrated energy */
    void SetTime(double a)      {time = a;}      /**< Set the raw time */
    void SetCorrectedTime(double a) {correctedTime = a;} /**< Set the corrected time */
    void SetCalTime(double a)   {calTime = a;}   /**< Set the calibrated time */
    void SetHighResTime(double a) {highResTime =a;} /**< Set the high resolution time */
    double GetEnergy() const      {return energy;}      /**< Get the raw energy */
    double GetCalEnergy() const   {return calEnergy;}   /**< Get the calibrated energy */
    double GetCorrectedTime() const {return correctedTime;} /**< Get the corrected time */
    double GetTime() const        {return time;}        /**< Get the raw time */
    double GetCalTime() const     {return calTime;}    /**< Get the calibrated time */
    double GetHighResTime() const {return highResTime;} /**< Get the high-resolution time */
    double GetEventTime() const   {return eventTime;}  /**< Get the event time */
    const Trace& GetTrace() const {return trace;} /**< Get a reference to the trace */
    Trace& GetTrace() {return trace;} /** Get a reference which can alter the trace */
    unsigned long GetTrigTime() const    
    {return trigTime;}    /**< Return the channel trigger time */
    unsigned long GetEventTimeLo() const
    {return eventTimeLo;} /**< Return the lower 32 bits of event time */
    unsigned long GetEventTimeHi() const
    {return eventTimeHi;} /**< Return the upper 32 bits of event time */
    unsigned long GetRunTime0() const
    {return runTime0;}    /**< Return the lower bits of run time */
    unsigned long GetRunTime1() const
    {return runTime1;}    /**< Return the middle bits of run time */
    unsigned long GetRunTime2() const
    {return runTime2;}    /**< Return the higher bits of run time */
    bool IsPileup() const {
	return pileupBit;
    }
    bool IsSaturated() const { /**< Return whether the trace is saturated */
	return saturatedBit;
    }
    const Identifier& GetChanID() const; /**< Get the channel identifier */
    int GetID() const;                   /**< Get the channel id defined as
					    pixie module # * 16 + channel number */
    unsigned long GetQdcValue(int i) const; /**< Get an onboard QDC value */

    ChanEvent();
    void ZeroVar();
};

// some global functions for sorting vectors
bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b);
bool CompareTime(const ChanEvent *a, const ChanEvent *b);

#endif
