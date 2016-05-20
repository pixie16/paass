/** \file TemplateProcessor.cpp
 *\brief A Template processor class that can be used to build your own.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include <iostream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TemplateProcessor.hpp"

namespace dammIds {
    namespace dettemplate {
        const int D_ENERGY  = 0; //!< ID for the energy of the template detector
        const int DD_TEMPLATE_VS_PULSER = 1; //!< Energy Template vs. Energy Pulser
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::dettemplate;

TemplateProcessor::TemplateProcessor():
    EventProcessor(OFFSET, RANGE, "TemplateProcessor") {
    associatedTypes.insert("template");
}

TemplateProcessor::TemplateProcessor(const double &a):
    EventProcessor(OFFSET, RANGE, "TemplateProcessor") {
    associatedTypes.insert("template");
    a_ = a;
}

void TemplateProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_ENERGY, SA, "Energy");
    DeclareHistogram2D(DD_TEMPLATE_VS_PULSER, SA, SA,
                       "Template Energy vs. Pulser Energy");
}

bool TemplateProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    evts_ = event.GetSummary("template")->GetList();

    for(vector<ChanEvent*>::const_iterator it = evts_.begin();
        it != evts_.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        if(location == 0)
            plot(D_ENERGY, (*it)->GetEnergy()*a_);
    }
    return(true);
}

bool TemplateProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    static const vector<ChanEvent*> & pulserEvents =
        event.GetSummary("pulser")->GetList();

    for(vector<ChanEvent*>::const_iterator it = evts_.begin();
        it != evts_.end(); it++) {
        unsigned int loc = (*it)->GetChanID().GetLocation();
        for(vector<ChanEvent*>::const_iterator itA = pulserEvents.begin();
            itA != pulserEvents.end(); itA++) {
            if(loc == 0)
                plot(DD_TEMPLATE_VS_PULSER, (*it)->GetEnergy(),
                     (*itA)->GetEnergy());
        }
    }
    return(true);
}
