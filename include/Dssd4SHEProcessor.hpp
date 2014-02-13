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

        const int D_DTIME_MISSING = 2;
        const int D_DE_MISSING = 3;

        const int D_ENERGY_CORRELATED_SIDE = 4;

        const int DD_EVENT_POSITION_FROM_T = 10;
        const int DD_EVENT_POSITION_FROM_E = 11;

        const int DD_EVENT_ENERGY__X_POSITION = 15;
        const int DD_EVENT_ENERGY__Y_POSITION = 16;

        const int DD_ENERGY__POSX_T_MISSING = 21;
        const int DD_ENERGY__POSY_T_MISSING = 22; 

        const int DD_DENERGY__DPOS_X_CORRELATED = 31; 
        const int DD_DENERGY__DPOS_Y_CORRELATED = 32; 

    }
}


class Dssd4SHEProcessor : public EventProcessor {
public:
    Dssd4SHEProcessor(double frontBackTimeWindow, 
                      double highEnergyCut, 
                      double lowEnergyCut, 
                      int numFrontStrips, 
                      int numBackStrips);
    virtual void DeclarePlots();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);

protected:
    bool pickEventType(SheEvent& event);

    SheCorrelator correlator;
    /** Events matched based on energy (MaxEvent) **/
    std::vector<std::pair<ChanEvent*, ChanEvent*> > xyEventsEMatch_; 

    /** Events matched based on timing correlation  **/
    std::vector<std::pair<ChanEvent*, ChanEvent*> > xyEventsTMatch_; 

    /**Limit in seconds for the time difference between front and
     * back to be correlated. Also to find Si Side detectors correlated
     * events (escapes)*/
    double timeWindow_;
    /** Energy cut to differentiate high-energy events (fission?)
     * from implantation and alpha decays (in keV) **/
    double highEnergyCut_;
    /** Energy cut to reject noise (in keV) **/
    double lowEnergyCut_;

};

#endif 
