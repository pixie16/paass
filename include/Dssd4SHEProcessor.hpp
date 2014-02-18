/** \file Dssd4SHEProcessor.hpp
 *
 */

#ifndef __DSSD4SHE_PROCESSOR_HPP_
#define __DSSD4SHE_PROCESSOR_HPP_

#include <vector>
#include <utility>
#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "SheCorrelator.hpp"

namespace dammIds { 
    namespace dssd4she {
        const int D_ENERGY_X = 0;
        const int D_ENERGY_Y = 1;

        const int D_DTIME = 2;
        const int D_MWPC_MULTI = 3;
        const int D_ENERGY_CORRELATED_SIDE = 4;
        const int D_DTIME_SIDE = 5;

        const int DD_ENERGY_DT__DSSD_MWPC = 6;
        const int DD_DE_E__DSSD_VETO = 7;

        const int DD_EVENT_POSITION = 10;
        const int DD_EVENT_POSITION_FROM_E = 11;
        const int DD_IMPLANT_POSITION = 12;
        const int DD_DECAY_POSITION = 13;
        const int DD_LIGHT_POSITION = 14;
        const int DD_UNKNOWN_POSITION = 15;
        const int DD_FISSION_POSITION = 16;

        const int DD_EVENT_ENERGY__X_POSITION = 17;
        const int DD_EVENT_ENERGY__Y_POSITION = 18;
        const int DD_MAXEVENT_ENERGY__X_POSITION = 19;
        const int DD_MAXEVENT_ENERGY__Y_POSITION = 20;
        const int DD_FRONTE__BACKE = 21;

        const int D_ENERGY_IMPLANT = 22;
        const int D_ENERGY_DECAY = 23;
        const int D_ENERGY_LIGHT = 24;
        const int D_ENERGY_UNKNOWN = 25;
        const int D_ENERGY_FISSION = 26;
        const int D_ENERGY_DECAY_BEAMSTOP = 27;

        const int D_ENERGY_WITH_VETO = 30;
        const int D_ENERGY_WITH_MWPC = 31;
        const int D_ENERGY_WITH_VETO_MWPC = 32;
        const int D_ENERGY_NO_VETO_MWPC = 33;

        /** Diagnostic **/
        const int DD_ENERGY__POSX_T_MISSING = 40;
        const int DD_ENERGY__POSY_T_MISSING = 41; 
        const int DD_DENERGY__DPOS_X_CORRELATED = 42; 
        const int DD_DENERGY__DPOS_Y_CORRELATED = 43; 
    }
}


class Dssd4SHEProcessor : public EventProcessor {
public:
    Dssd4SHEProcessor(double frontBackTimeWindow, 
                      double deltaEnergy,
                      double highEnergyCut, 
                      double lowEnergyCut, 
                      double fisisonEnergyCut, 
                      int numFrontStrips, 
                      int numBackStrips);
    virtual void DeclarePlots();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);

protected:
    bool pickEventType(SheEvent& event);

    struct StripEvent {
        StripEvent() {
            t = 0;
            E = 0;
            pos = -1;
            sat = false;
            pileup = false;
        }

        StripEvent(double energy, double time, int position,
                   bool saturated) {
            E = energy;
            t = time;
            pos = position;
            sat = saturated;
            pileup = false;
        }

        double t;
        double E;
        int pos;
        bool sat;
        bool pileup;
    };

    SheCorrelator correlator_;
    /** Events matched based on energy (MaxEvent) **/
    std::vector<std::pair<StripEvent, StripEvent> > xyEventsEMatch_; 

    /** Events matched based on timing correlation  **/
    std::vector<std::pair<StripEvent, StripEvent> > xyEventsTMatch_; 

    /**Limit in seconds for the time difference between front and
     * back to be correlated. Also to find Si Side detectors correlated
     * events (escapes)*/
    double timeWindow_;

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

};

#endif 
