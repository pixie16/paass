/** \file Dssd4SHEProcessor.hpp
 * @authors P. Brionnet , T. T. King
 * @date February 5, 2019
 *
 */

#ifndef __NSHE_PROCESSOR_HPP_
#define __NSHE_PROCESSOR_HPP_

#include <vector>
#include <utility>
#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "SheCorrelator.hpp"

namespace dammIds {
    namespace dssd4she {
        const int D_ENERGY_X = 0; //!< Energy on X strip
        const int D_ENERGY_Y = 1; //!< Energy on Y strip

        const int D_DTIME = 2; //!< Time difference 
        const int D_DTIMEE = 9; //!< Time difference 
        const int D_MWPC_MULTI = 3; //!< Multiplicity of MWPC
        const int D_ENERGY_CORRELATED_SIDE = 4; //!< Energy of Correlated Side
        const int D_DTIME_SIDE = 5; //!< Time Diff on the side
        const int D_DTIMETOF = 8; // DT DSSD TOF1
        const int DD_ENERGY_DT__DSSD_MWPC = 6; //!< Energy vs MWPC
        const int DD_DE_E__DSSD_VETO = 7; //!< Energy vs. Veto

        const int DD_EVENT_POSITION = 10; //!< Event Position 
        const int DD_EVENT_POSITION_FROM_E = 11; //!< Event Pos from Energy
        const int DD_IMPLANT_POSITION = 12; //!< Implant energy 
        const int DD_DECAY_POSITION = 13; //!< Decay Position
        const int DD_LIGHT_POSITION = 14; //!< Light Ion Position 
        const int DD_UNKNOWN_POSITION = 15; //!< Unknown Position
        const int DD_FISSION_POSITION = 16; //!< Fission Position 

        const int DD_EVENT_ENERGY__X_POSITION = 17; //!< Energy vs. X Position
        const int DD_EVENT_ENERGY__Y_POSITION = 18; //!< Energy vs. Y Position
        const int DD_MAXEVENT_ENERGY__X_POSITION = 19; //!< Max E vs X Pos
        const int DD_MAXEVENT_ENERGY__Y_POSITION = 20; //!< Max E vs. Y Pos
        const int DD_FRONTE__BACKE = 21; //!< Front Energy vs. Back Energy
        const int DD_MAXEVENT_FRONTE__BACKE = 28; //!< Front Energy vs. Back Energy

        const int D_ENERGY_IMPLANT = 22; //!< Implant Energy 
        const int D_ENERGY_DECAY = 23; //!< Decay energy 
        const int D_ENERGY_LIGHT = 24; //!< Light ion Energy
        const int D_ENERGY_UNKNOWN = 25; //!< Unknown event energy
        const int D_ENERGY_FISSION = 26; //!< fission energy 
        const int D_ENERGY_DECAY_BEAMSTOP = 27; //!< decay energy of beamstop

        const int D_ENERGY_WITH_VETO = 30; //!< Energy with veto
        const int D_ENERGY_WITH_MWPC = 31; //!< energy with MWPC coincidence
        const int D_ENERGY_WITH_VETO_MWPC = 32; //!< energy with Veto & MWPC
        const int D_ENERGY_NO_VETO_MWPC = 33; //!< energy with no veto or MWPC

        /** Diagnostic **/
        const int DD_ENERGY__POSX_T_MISSING = 40; //!< Energy vs Missing X Pos
        const int DD_ENERGY__POSY_T_MISSING = 41; //!< Energy vs Missing Y pos
        const int DD_DENERGY__DPOS_X_CORRELATED = 42; //!< ??
        const int DD_DENERGY__DPOS_Y_CORRELATED = 43; //!< ??

	const int DD_TOF_ENERGY = 50 ; // TOF ENERGY SPECTRUM	
	const int DD_VETO_ENERGY = 51 ; // VETO ENERGY SPECTRUM	


    }
}

///Class to handle DSSDs for Super heavy element experiments
class NsheProcessor : public EventProcessor {
public:
    /** Constructor taking arguments */
 //   NsheProcessor();

    ~NsheProcessor() {};

    NsheProcessor(double timeWindow,
		      double tofWindow,
              double vetoWindow,
                      double deltaEnergy,
                      double highEnergyCut,
                      double lowEnergyCut,
		      double zero_suppress, 
		      double fisisonEnergyCut,
                      int numFrontStrips,
                      int numBackStrips);

    /** Declare plots */
    virtual void DeclarePlots();

    /** Perform preprocess */
    virtual bool PreProcess(RawEvent &event);

    /** Perform Process */
    virtual bool Process(RawEvent &event);

protected:
    /** Picks what event type we had 
     * \return true if the event type was found(?) */
    bool pickEventType(SheEvent &event);

    ///Structure defining an event on a strip of the DSSD
    struct StripEvent {
        StripEvent() {
            t = 0;
            E = 0;
            pos = -1;
            sat = false;
            pileup = false;
	    trace.clear();
        }

        ///Constructor for StripEvent structure
        StripEvent(double energy, double time, int position,
                   bool saturated,std::vector<unsigned int> tracein, bool pile_up) {
            E = energy;
            t = time;
            pos = position;
            sat = saturated;
            pileup = pile_up;
	    trace.swap(tracein);
	    //else trace.clear();
        }

        double t; //!< the time
        double E; //!< the energy 
        int pos; //!< position
        bool sat; //!< if we had a saturation 
        bool pileup; //!< if we had a pileup
	std::vector<unsigned int> trace;
    };

    processor_struct::DSSD dssdstruc; //!<Working structure  

    //SheCorrelator correlator_; //!< instance of the Correlator 

    /** Events matched based on energy (MaxEvent) **/
    std::vector<std::pair<StripEvent, StripEvent>> xyEventsEMatch_;

    /** Events matched based on timing correlation  **/
    std::vector<std::pair<StripEvent, StripEvent>> xyEventsTMatch_;

    /**Limit in seconds for the time difference between front and
     * back to be correlated. Also to find Si Side detectors correlated
     * events (escapes)*/
    double timeWindow_;

   /** Limit in seconds for the difference in time between tofs/veto signals and DSSD**/

    double tofWindow_;

    /** Limit in the veto correlation with the DSSD **/
    double vetoWindow_;

    /**Limit in keV of difference between front and back events to
     * be considered a good event */
    double deltaEnergy_;

    /** Energy cut to differentiate high-energy events (fission?)
     * from implantation and alpha decays (in keV) **/
    double highEnergyCut_;

    /** Low Energy cut for interesting alphas (in keV) **/
    double lowEnergyCut_;

    /** Fission Energy cut (in keV) **/
    double fissionEnergyCut_;

    /** Zero suppress cut (in keV) **/
    double zero_suppress_;

};

#endif 
