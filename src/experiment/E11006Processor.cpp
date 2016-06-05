/** \file E11006Processor.cpp
 * \brief A class specific to the e11006 experiment
 *\author S. V. Paulauskas 
 *\date June 18, 2014
 */
#include <iostream>
#include <string>

#include <cmath>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "E11006Processor.hpp"

//declare the namespace for the histogram numbers
namespace dammids {
    namespace e11006 {
        const int D_TEMP 0; //!< Temporary histogram 
    }
}

using namespace std;
using namespace dammids::e11006;

void E11006Processor::DeclarePlots(void) {
    //declare a single histogram
    DeclareHistogram1D(D_TEMP, S5, "1D Debugging");
}


E11006Processor::E11006Processor(): EventProcessor(OFFSET,RANGE) {
    name = "e11006";
    //associate the processor with both the tac and scintillator event types
    associatedTypes.insert("tac"); 
    associatedTypes.insert("scint"); 
}

bool E11006Processor::Process(RawEvent &event) 
{
    if (!EventProcessor::Process(event))
	return false;

    //get the list of all the tacs in the current event
    static const vector<ChanEvent*> &tacEvents = 
        event.GetSummary("vandleSmall")->GetList();
    //get the list of all the scintillators with the subtype "de" in the 
    //current event
    static const vector<ChanEvent*> &scintEvents = 
        event.GetSummary("scint:de")->GetList();


    //let's just loop over all of the tac events currently
    for(vector<ChanEvent*>::iterator it = tacEvents.begin(); it != tacEvents.end();
        it++) {
        //for a summary of the nonsense you can access from the ChanEvent see 
        //ChanEvent.hpp (it will be listed under the long list of Get methods)

        //get the location of the tac event
        unsigned int location = (*it)->GetChanID().GetLocation();
        //get the subtype of the tac event (maybe not useful but instructive)
        string subType = (*it)->GetChanID().GetSubtype();
        //how about we do something useful like plot an energy?
        if(location == 0) 
            plot(it->GetEnergy());
    }

    EndProcess();
    return true;
}
