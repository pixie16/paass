/*! \file Correlator.h
 *  \brief Header file for correlation
 
 *  Class which handles implant/decay correlations
 */

#ifndef __CORRELATOR_PROCESSOR_H_
#define __CORRELATOR_PROCESSOR_H_

#include <fstream>
#include <utility>
#include <vector>

#include <cmath>

#include "param.h"

// forward declarations
class LogicProcessor;
class RawEvent;

struct ImplantData
{
    double time;    ///< time of an implant
    double dtime;   ///< time elapsed since previous implant
    bool implanted; ///< previous implant flag
    double tacValue; ///< a TAC value
    bool flagged; ///< previous event in pixel is flagged as interesting

    ImplantData() {
	Clear();
    }
    void Clear(void) { 
	time = dtime = tacValue = NAN; 
	flagged = implanted = false;
    }
};

struct DecayData
{
    double time;    ///< time of a decay
    double dtime;   ///< time elapsed since previous implant
  
    DecayData() {
	Clear();
    }
    void Clear(void) {
	time = dtime = NAN;
    }
};

struct ListData
{
    double time;    ///< pixie time of the event
    double energy;  ///< energy of the event
    double offTime; ///< relative time of the event with respect to some logic
    
    unsigned long clockCount;    ///< number of clock pulses received
    std::vector<bool> logicBits; ///< status of logic bits

    ListData(double t, double e, LogicProcessor *lp = NULL);
};

/*!
  \brief correlate decays with previous implants
  
  The class controls the correlations of decays with previous implants.  There
  are arrays of size MAX_STRIP %x MAX_STRIP that store implants and decays.
  When an event has been identified as either an implant or decay, its
  information is placed in the appropriate array based on its pixel location.
  If a decay was identified, it is correlated with a previous implant.  The
  correlator checks to make sure that the time between implants is
  sufficiently long and that the correlation time has not been exceeded
  before correlating an implant with a decay.
*/
class Correlator
{
 public:
    /// types of events passed to the correlator
    enum EEventType{IMPLANT_EVENT, DECAY_EVENT, UNKNOWN_TYPE};
    /// correlator condition based on the given events
    enum EConditions{INVALID_LOCATION = 4,
		     VALID_IMPLANT = 12,
		     VALID_DECAY = 16,
		     BACK_TO_BACK_IMPLANT = 32,
		     DECAY_TOO_LATE = 48,
		     IMPLANT_TOO_SOON = 52,
		     UNKNOWN_EVENT = 90,
		     OTHER_EVENT = 100};
  
    Correlator();
    ~Correlator();

    void DeclarePlots(void) const;
    void Init(void);
    void Correlate(RawEvent &event, EEventType type, unsigned int fch, 
		   unsigned int bch, double time, double energy = 0);  
    void PrintDecayList(unsigned int fch, unsigned int bch) const;
  
    double GetDecayTime(void) const {
	return lastDecay->dtime;
    }
    double GetDecayTime(int fch, int bch) const {
	return decay[fch][bch].dtime;
    }
    double GetImplantTime(void) const {
	return lastImplant->time;
    }
    double GetImplantTime(int fch, int bch) const {
	return implant[fch][bch].time;
    }
    void SetTACValue(double d) {
	lastImplant->tacValue = d;
    }
    void Flag(int fch, int bch) {
	implant[fch][bch].flagged = true;
    }
    EConditions GetCondition(void) const {
	return condition;
    }
  
 private:
    typedef std::vector<ListData> corrlist_t;

    ImplantData implant[MAX_STRIP][MAX_STRIP]; /**< 2D array containing the most
						  recent implant information in
						  each pixel that is correlated 
						  with a subsequent decay	*/
    DecayData   decay[MAX_STRIP][MAX_STRIP]; /**< 2D array containing the most 
						recent decay information in
						each pixel that is correlated 
					      with a previous implant */  
    // in units of pixie clocks
    static const double minImpTime; /**< The minimum amount of time that must
				       pass before an implant will be considered
				       for correlation */
    static const double corrTime;   /**< The maximum amount of time allowed
				       between a decay and its previous implant
				       for a correlation between the two to occur */
    static const double fastTime;   /**< Times shorter than this are output as a fast decay */
    
    ImplantData *lastImplant; ///< last implant processed by correlator
    DecayData   *lastDecay; ///< last decay procssed by correlator
    LogicProcessor *logicProc; ///< a logic processor from the detector driver
    
    EConditions condition; ///< condition for last processed event
    corrlist_t decaylist[MAX_STRIP][MAX_STRIP]; ///< list of event data for a particular pixel since implant
};

#endif // __CORRELATOR_PROCESSOR_H_
