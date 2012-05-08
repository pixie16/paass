// -*-c++-*- 
/**   \file RawEvent.h
 *    \brief Header file for the rawevent
 *
 *    The variables and structures contained in a raw event
 *
 *    SNL - 7-2-07
 */

#ifndef __RAWEVENT_H_
#define __RAWEVENT_H_ 1

#include <vector>
#include <string>
#include <map>
#include <set>

#include "Correlator.h"

#include "pixie16app_defs.h"
#include "param.h"
#include "Trace.h"

/**
 * \brief Channel identification

 * All parameters needed to uniquely specify the detector connected to a
 * specific channel are set here.  A vector of identifiers for all channels is
 * created on initialization in InitID().  Each event that is created in
 * ScanList() in PixieStd.cpp has its channel identifier attached to it in
 * the variable chanID inside RawEvent
 *
 * Identifier is a class that will contain basic channel information that
 * will not change including the damm spectrum number where the raw energies
 * will be plotted, the detector type and subtype, and the detector's physical
 * location (strip number, detector location, ...)
 */

class Identifier
{
public:
    typedef int TagValue;

    void SetDammID(int a)     {dammID = a;}   /**< Set the dammid */
    void SetType(const std::string &a)    {type = a;}     /**< Set the detector type */
    void SetSubtype(const std::string &a) {subtype = a;}  /**< Set the detector subtype */
    void SetLocation(int a)   {location = a;} /**< Set the detector location */
    
    int GetDammID() const                 {return dammID;}   /**< Get the dammid */
    const std::string& GetType() const    {return type;}     /**< Get the detector type */
    const std::string& GetSubtype() const {return subtype;}  /**< Get the detector subtype */
    int GetLocation() const               {return location;} /**< Get the detector location */
    
    void AddTag(const std::string &s, TagValue n) {tag[s] = n;} /**< Insert a tag */
    bool HasTag(const std::string &s) const {return tag.count(s);} /**< True if the tag s has been inserted */
    TagValue GetTag(const std::string &s) {return tag[s];} /**< Return the value of a tag */

    Identifier();
    void Zero();
    static void PrintHeaders(void);
    void Print(void) const;
    
    bool operator==(const Identifier &x) const {
	return (type == x.type &&
		subtype == x.subtype &&
		location == x.location);
    } /**< Compare this identifier with another */
    bool operator!=(const Identifier &x) const {
	return !operator==(x);
    }
private:
    std::string type;      /**< Specifies the detector type */
    std::string subtype;   /**< Specifies the detector sub type */
    int dammID;            /**< Damm spectrum number for plotting calibrated energies */
    int location;          /**< Specifies the real world location of the channel.
  			        For the DSSD this variable is the strip number */
    std::map<std::string, TagValue> tag;  /**< A list of tags associated with the identifer */ 
};

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
    static const double pixieEnergyContraction; ///< energies from pixie16 are contracted by this number

    void SetEnergy(double a)    {energy = a;}    /**< Set the raw energy in case we want
						    to extract it from the trace ourselves */
    void SetCalEnergy(double a) {calEnergy = a;} /**< Set the calibrated energy */
    void SetTime(double a)      {time = a;}      /**< Set the raw time */
    void SetCalTime(double a)   {calTime = a;}   /**< Set the calibrated time */
    void SetHighResTime(double a) {highResTime =a;} /**< Set the high resolution time */
    double GetEnergy() const      {return energy;}      /**< Get the raw energy */
    double GetCalEnergy() const   {return calEnergy;}   /**< Get the calibrated energy */
    double GetTime() const        {return time;}        /**< Get the raw time */
    double GetCalTime() const     {return calTime;}    /**< Get the calibrated time */
    double GetHighResTime() const {return highResTime;} /**< Get the high-resolution time */
    double GetEventTime() const   {return eventTime;}  /**< Get the event time */
    const Trace &GetTrace() const {return trace;} /**< Get a reference to the trace */
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

/** \brief Summary of all channels of one detector type
 * For each group of detectors that exists in the analysis, a detector summary
 * is created.  The detector summary includes the multiplicity, maximum
 * deposited energy, and physical location (strip number, detector number ...)
 * where the maximum energy was deposited for a type of detectors.  Also 
 * pointers to all channels of this type are stored in a vector, as well as the
 * channel event where the maximum energy was deposited.  Lastly, the detector
 * summary records the detector name to which it applies.
 */
class DetectorSummary
{
private:
    std::string name;                  /**< name associated with this summary */
    std::string type;                  /**< detector type associated with this summary */
    std::string subtype;               /**< detector subtype associated with this summary */
    std::vector<ChanEvent*> eventList; /**< list of events associated with this detector group */
    ChanEvent* maxEvent;               /**< event with maximum energy deposition */
public:
    DetectorSummary();
    DetectorSummary(const std::string &str, const std::vector<ChanEvent *> &fullList);
    void Zero();
    void AddEvent(ChanEvent *ev); /**< Add a channel event to the summary */

    void SetName(const std::string& a) {name = a;} /**< Set the detector type name */
    
    const ChanEvent* GetMaxEvent(void) const 
    {return maxEvent;}          /**< Get the max event */
    int GetMult() const 
    {return eventList.size();} /**< Get the multiplicity */
    const std::string& GetName() const          
    {return name;}              /**< Get the detector type name */
    const std::vector<ChanEvent*>& GetList() const 
    {return eventList;}         /**< Get the list of all channels in the
				   rawevent that are of this detector type */
};

class EventContainer : public std::vector<ChanEvent *>
{
    // simple derived class for now
};

/** \brief The all important raw event
 *
 * The rawevent serves as the basis for the experimental analysis.  The rawevent 
 * includes a vector of individual channels that have been deemed to be close to
 * each other in time.  This determination is performed in ScanList() from
 * PixieStd.cpp.  The rawevent also includes a map of detector summaries which
 * contains a detector summary for each detector type that is used in the analysis
 *
 *  The rawevent is intended to be versatile enough to remain unaltered unless
 * LARGE changes are made to the pixie16 code.  Be careful when altering the
 * rawevent.
 */
class RawEvent
{
private:
    std::map<std::string, DetectorSummary> sumMap; /**< An STL map containing DetectorSummary classes
					    associated with detector types */
    // This only controls whether we output warnings, so it's free to change
    mutable std::set<std::string> nullSummaries;   /**< Summaries which were requested but don't exist */
    std::vector<ChanEvent*> eventList;             /**< Pointers to all the channels that are close
					             enough in time to be considered a single event */
    Correlator correlator;                         /**< class to correlate decay data with implantation data */
public:   
    RawEvent();
    void Clear(void);
    size_t Size(void) const;
    void Init(const std::set<std::string> &usedTypes);
    void AddChan(ChanEvent* event);       
    void Zero(const std::set<std::string> &);

    Correlator &GetCorrelator()
    {return correlator;} /**< get the correlator */
    DetectorSummary *GetSummary(const std::string& a, bool construct = true);
    const DetectorSummary *GetSummary(const std::string &a) const;
    const std::vector<ChanEvent *> &GetEventList(void) const
    {return eventList;} /**< Get the list of events */
};

class StatsData {
private:
    static const size_t statSize = N_DSP_PAR - DSP_IO_BORDER;
    static const size_t maxVsn = 14;

    double firstTime; /**< Store the time of the first statistics block */
    pixie::word_t oldData[maxVsn][statSize]; /**< Older statistics data to calculate the change in statistics */
    pixie::word_t data[maxVsn][statSize];    /**< Statistics data from each module */
public:
    static const pixie::word_t headerLength = 1;

    StatsData(void);
    void DoStatisticsBlock(pixie::word_t *buf, int vsn);

    double GetCurrTime(unsigned int id) const;
    double GetDiffPeaks(unsigned int id) const;
    double GetDiffTime(unsigned int id) const;
    double GetRealTime(unsigned int mod = 0) const;
};

#endif // __RAWEVENT_H_
