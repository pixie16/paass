/** \file Correlator.hpp
 * \brief Header file for correlation
 *
 * Class which handles implant/decay correlations
 */

#ifndef __CORRELATOR_PROCESSOR_HPP_
#define __CORRELATOR_PROCESSOR_HPP_

#include <utility>
#include <vector>

#include <cmath>

#include "Plots.hpp"
#include "DammPlotIds.hpp"
#include "Globals.hpp"

class LogicProcessor;
class RawEvent;

//! Structure to contain the event info
struct EventInfo {
    /// types of events passed to the correlator
    enum EEventTypes {IMPLANT_EVENT, ALPHA_EVENT, BETA_EVENT, FISSION_EVENT,
		      PROTON_EVENT, DECAY_EVENT, PROJECTILE_EVENT, GAMMA_EVENT,
		      UNKNOWN_EVENT};

    EEventTypes type; ///< event type
    double time;     ///< timestamp of event
    double dtime;    ///< time since implant [pixie units]
    double energy;   ///< energy of event
    double energyBox; ///< energy depositied into the box
    double offTime;  ///< length of time beam has been off
    double foilTime; ///< time difference to foil event
    double tof;      ///< time of flight for an implant
    double position; ///< calculated strip position
    short  boxMult;  ///< numebr of box hits
    short  boxMax;   ///< location of maximum energy in box
    short  impMult;  ///< number of implant hits
    short  mcpMult;  ///< number of mcp hits
    short  generation; ///< generation number (0 = implant)
    bool   flagged;  ///< flagged of interest
    bool   hasTof;   ///< has time of flight data
    bool   hasVeto;  ///< veto detector has been hit
    bool   beamOn;   ///< beam is on target
    bool   pileUp;   ///< trace is piled-up

    unsigned long clockCount;//!< the counter on the clock
    unsigned char logicBits[dammIds::logic::MAX_LOGIC+1];//!< array of logic bits

    /** Default Constructor */
    EventInfo();
    /** Constructor taking some default values
     * \param [in] t : the time
     * \param [in] e : the energy
     * \param [in] lp : an instance of the logic processor */
    EventInfo(double t, double e, LogicProcessor *lp);
};

//! The list of correlations
class CorrelationList : public std::vector<EventInfo> {
private:
    bool flagged;//!< flag telling if something has been flagged
public:
    /** Default Constructor */
    CorrelationList();
    /** \return the decay time */
    double GetDecayTime(void) const;
    /** \return the implant time */
    double GetImplantTime(void) const;
    /** flag an event */
    void Flag(void);
    /** \return true if something is flagged */
    bool IsFlagged(void) const;
    //! overide the vector clear function so that the flag is also removed
    void clear(void);
    /** Print the decay list */
    void PrintDecayList(void) const;
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
class Correlator {
public:
    /// correlator condition based on the given events
    enum EConditions {INVALID_LOCATION     = 4,
		      VALID_IMPLANT        = 12,
		      VALID_DECAY          = 16,
		      BACK_TO_BACK_IMPLANT = 32,
		      DECAY_TOO_LATE       = 48,
		      IMPLANT_TOO_SOON     = 52,
		      UNKNOWN_CONDITION    = 100};
    /** Default Constructor */
    Correlator();
    /** Default Destructor */
    virtual ~Correlator();

    /** Declare plots for the correlator */
    void DeclarePlots(void);
    /** Initialize the correlator - Does nothing
     * \param [in] rawev : the raw even for initialization */
    void Init(RawEvent &rawev) {};
    /** Correlate two locations in the event
     * \param [in] event : the event to use to correlate
     * \param [in] fch : the first channel to correlate
     * \param [in] bch : the second channel to correlate */
    void Correlate(EventInfo &event, unsigned int fch, unsigned int bch);
    /** \brief Correlates all positions
    *
    *  This correlates an event with all positions in the setup.
    *  This is useful for cases where the event is interesting but can not be assigned
    *  to a particular implant location as in the case of external gamma-ray detectors
    * \param [in] event : the even to correlate with */
    void CorrelateAll(EventInfo &event);
    /** Correlates all X positions in the event
     * \param [in] event : the event to correlate with
     * \param [in] bch : the locations to correlate with */
    void CorrelateAllX(EventInfo &event, unsigned int bch);
    /** Correlates all Y positions in the event
     * \param [in] event : the even to correlate with
     * \param [in] fch : the locations to correlate with */
    void CorrelateAllY(EventInfo &event, unsigned int fch);
    /** Print the decay list
     * \param [in] fch : the first location to print
     * \param [in] bch : the second location to print  */
    void PrintDecayList(unsigned int fch, unsigned int bch) const;

    /** \return the decay time */
    double GetDecayTime(void) const;
    /** \return the decay time for a given pair of channels
     * \param [in] fch : the first channel to look for
     * \param [in] bch : the second channel to look for */
    double GetDecayTime(int fch, int bch) const;
    /** \return get the implant time */
    double GetImplantTime(void) const;
    /** \return the implant time for a given pair of channels
     * \param [in] fch : the first channel to look for
     * \param [in] bch : the second channel to look for  */
    double GetImplantTime(int fch, int bch) const;

    /** Set the flag for the two locations
     * \param [in] fch : first channel that we'll set
     * \param [in] bch : the second channel to set */
    void Flag(int fch, int bch);

    /** Check if the two channels that we have are flagged
     * \param [in] fch : the first channel to check
     * \param [in] bch : the second channel to check
     * \return true if it is flagged */
    bool IsFlagged(int fch, int bch);

    /** \return The conditions for correlation */
    EConditions GetCondition(void) const {
        return condition;
    }

private:
    Plots histo; //!< Instance of the Plots class

    /*! \brief plots data into a histogram with provided DAMM ID
    * \param [in] dammId  : the dammID to plot into
    * \param [in] val1 : the x value
    * \param [in] val2 : the y value
    * \param [in] val3 : the z value
    * \param [in] name : the name of the histogram */
    virtual void plot(int dammId, double val1, double val2 = -1,
                      double val3 = -1, const char* name="h") {
        histo.Plot(dammId, val1, val2, val3, name);
    }

    /*! \brief Declares a 1D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] title : The title for the histogram */
    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }

    /*! \brief Declares a 2D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] ySize : The range of the y-axis
    * \param [in] title : The title of the histogram */
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize,
                                    const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }

    static const size_t arraySize = 40; /**< Size of the 2D array to hold the decay lists */

    static const double minImpTime; /**< The minimum amount of time that must
				       pass before an implant will be considered
				       for correlation in clock ticks */
    static const double corrTime;   /**< The maximum amount of time allowed
				       between a decay and its previous implant
				       for a correlation between the two to occur in clock ticks*/
    static const double fastTime;   /**< Times shorter than this are output as
                                         a fast decay */

    EventInfo   *lastImplant;  ///< last implant processed by correlator
    EventInfo   *lastDecay;    ///< last decay procssed by correlator

    EConditions condition;     ///< condition for last processed event
    CorrelationList decaylist[arraySize][arraySize]; ///< list of event data for a particular pixel since implant
};
#endif // __CORRELATOR_PROCESSOR_HPP_
