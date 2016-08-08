/** \file LaBr3TestProcessor.cpp
 *\brief A LaBr3Test processor class that can be used to build your own.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "HighResTimingData.hpp"
#include "RawEvent.hpp"
#include "Labr3TestProcessor.hpp"

namespace dammIds {
    namespace experiment {
        const int D_TDIFF  = 0; //!< ID for the energy of the template detector
        const int DD_PP = 1; //!< Energy Labr3Test vs. Energy Pulser
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

Labr3TestProcessor::Labr3TestProcessor():
    EventProcessor(OFFSET, RANGE, "labr3Processor") {
    associatedTypes.insert("labr3");
}

void Labr3TestProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TDIFF, SE, "Time Difference");
    DeclareHistogram2D(DD_PP, SE, SE,
                       "Phase - Phase");
}

bool Labr3TestProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    static const vector<ChanEvent*> & labr3Events =
        event.GetSummary("labr3")->GetList();
    
    for(vector<ChanEvent*>::const_iterator it = labr3Events.begin();
        it != labr3Events.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        // if(location == 0)
        //     plot(D_ENERGY, (*it)->GetEnergy());
    }
    return(true);
}

bool Labr3TestProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    static const vector<ChanEvent*> & labr3Events =
        event.GetSummary("labr3")->GetList();
    static const vector<ChanEvent*> & tvandleEvents =
        event.GetSummary("tvandle")->GetList();

    double timeRes  = 2; //20 ps/bin
    double timeOff  = 2000.;
    double phaseX   = -2000.;

    for(vector<ChanEvent*>::const_iterator it = labr3Events.begin();
         it != labr3Events.end(); it++) {
	HighResTimingData labr3((*it));
	
	for(vector<ChanEvent*>::const_iterator itA = tvandleEvents.begin();
	    itA != tvandleEvents.end(); itA++) {

	    HighResTimingData tvandle((*itA));

	    if(tvandle.GetIsValidData() && labr3.GetIsValidData()) {
		//cout << (labr3.GetHighResTime() - tvandle.GetHighResTime())*
		    // timeRes+timeOff << " " << labr3.GetPhase()*timeRes << " " << 
		    // tvandle.GetPhase()*timeRes << endl;
		
		plot(D_TDIFF, (labr3.GetHighResTime() - tvandle.GetHighResTime())*
		     timeRes+timeOff);
		plot(DD_PP, labr3.GetPhase()*timeRes-phaseX, 
		     tvandle.GetPhase()*timeRes-phaseX);
	    }
	}
    }
    return(true);
}
