/** \file IS600GeProcessor.cpp
 *
 * implementation for germanium processor for 3Hen experiment
 */

//? Make a clover specific processor

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <utility>
#include <limits>

#include <cmath>
#include <cstdlib>

#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "DammPlotIds.hpp"

#include "Correlator.hpp"
#include "DetectorLibrary.hpp"
#include "GeProcessor.hpp"
#include "IS600GeProcessor.hpp"
#include "Messenger.hpp"
#include "RawEvent.hpp"
#include "TreeCorrelator.hpp"

using namespace std;

namespace dammIds {
    namespace ge {
        namespace neutron {
            const int D_ENERGY = 200;
            const int D_ENERGY_MOVE = 206;
            const int DD_ENERGY = 202;
            const int DD_ENERGY_NEUTRON_LOC = 203;
            const int D_MULT = 209;
            const int DD_ENERGY__TIMEX = 221; 
            const int DD_ENERGY__TIMEX_GROW = 223; 
            const int DD_ENERGY__TIMEX_DECAY = 225; 

            const int D_ADD_ENERGY = 250; 
            const int DD_ADD_ENERGY = 252;
            const int D_ADD_ENERGY_TOTAL = 255;
            const int DD_ADD_ENERGY__TIMEX = 271;
	 
	  
            namespace betaGated {
                const int D_ENERGY = 210;
                const int D_ENERGY_PROMPT = 211;
                const int DD_ENERGY = 212;
                const int DD_ENERGY__TIMEX = 231; 

                const int D_ADD_ENERGY = 260; 
                const int D_ADD_ENERGY_TOTAL = 265;
                const int DD_ADD_ENERGY = 262;
                const int DD_ADD_ENERGY__TIMEX = 281;
	        const int DD_PROTONGAMMATDIFF_VS_GAMMAEN = 213;
            } 
        }
    }
}

IS600GeProcessor::IS600GeProcessor(double gammaThreshold, double lowRatio,
                         double highRatio, double subEventWindow,
                         double gammaBetaLimit, double gammaGammaLimit,
                         double cycle_gate1_min, double cycle_gate1_max,
                         double cycle_gate2_min, double cycle_gate2_max) :

                         GeProcessor(gammaThreshold, lowRatio, highRatio,
                                     subEventWindow, gammaBetaLimit, 
                                     gammaGammaLimit,
                                     cycle_gate1_min, cycle_gate1_max,
                                     cycle_gate2_min, cycle_gate2_max)
{
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void IS600GeProcessor::DeclarePlots(void) 
{
    GeProcessor::DeclarePlots();
    using namespace dammIds::ge;

    const int energyBins1  = SD;
    const int energyBins2  = SC;
    const int granTimeBins = SA;

    DeclareHistogram1D(neutron::D_ENERGY, energyBins1,
                      "Gamma singles ungated");
    DeclareHistogram1D(neutron::betaGated::D_ENERGY, energyBins1,
                       "Gamma singles beta gated");
    // DeclareHistogram1D(neutron::D_ENERGY_MOVE, energyBins1,
    //                    "Gamma singles VANDLE gated");
    //DeclareHistogram2D(neutron::betaGated::DD_PROTONGAMMATDIFF_VS_GAMMAEN, SD, SD, "GammaProton TDIFF vs. Gamma Energy");
}

bool IS600GeProcessor::Process(RawEvent &event) {
    using namespace dammIds::ge;

    if (!EventProcessor::Process(event))
        return false;

    // Call base class processing
    GeProcessor::Process(event);

    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();
     double clockInSeconds = Globals::get()->clockInSeconds();
    // plot with 10 ms bins
    const double plotResolution = 10e-3 / clockInSeconds;
    double lastProtonTime =  TreeCorrelator::get()->place("mtc_t1_0")->last().time;

    for (vector<ChanEvent*>::iterator it1 = geEvents_.begin(); 
	 it1 != geEvents_.end(); ++it1) {
        ChanEvent *chan = *it1;
        
        double gEnergy = chan->GetCalEnergy();
        double gTime   = chan->GetCorrectedTime();
        //double decayTime = (gTime - cycleTime) * clockInSeconds;
        if (gEnergy < gammaThreshold_)
            continue;

        plot(neutron::D_ENERGY, gEnergy);
	if(hasBeta)
	  plot(neutron::betaGated::D_ENERGY, gEnergy);

	// granploty(neutron::DD_ENERGY__TIMEX,
        //             gEnergy, decayTime, timeResolution);
     // iteration over events


	plot(neutron::betaGated::DD_PROTONGAMMATDIFF_VS_GAMMAEN, gEnergy ,(gTime - lastProtonTime) / plotResolution) ;
    }
    EndProcess(); 
    return true;
}
