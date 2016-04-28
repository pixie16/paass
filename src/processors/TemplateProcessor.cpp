/** \file TemplateProcessor.cpp
 *\brief A Template processor class that can be used to build your own.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TemplateProcessor.hpp"

namespace dammIds {
    namespace template {
        const int D_ENERGY  = 0; //!< ID for the energy of the template detector
        const int DD_TEMPLATE_VS_PULSER = 1; //!< Energy Template vs. Energy Pulser
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::template;

TemplateProcessor::TemplateProcessor():
    EventProcessor(dammIds::template::OFFSET, dammIds::template::RANGE,
                   "TemplateProcessor") {
    associatedTypes.insert("template");
}

void TemplateProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_ENERGY, SA, "Energy");
    DeclareHistogram2D(DD_TEMPLATE_VS_PULSER, SA, SA,
                       "Template Energy vs. Pulser Energy");
}

bool TemplateProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    static const vector<ChanEvent*> & templateEvents =
        event.GetSummary("template")->GetList();

    for(vector<ChanEvent*>::const_iterator it = templateEvents.begin();
        it != templateEvents.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        if(location == 0)
            plot(D_ENERGY, (*it)->GetEnergy());
    }
    return(true);
}

bool TemplateProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    static const vector<ChanEvent*> & templateEvents =
        event.GetSummary("pulser")->GetList();

    for(vector<ChanEvent*>::const_iterator it = templateEvents.begin();
        it != templateEvents.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        for(vector<ChanEvent*>::const_iterator itA = pulserEvents.begin();
            itA != pulserEvents.end(); itA++) {
                unsigned int location = (*it)->GetChanID().GetLocation();
                if(location == 0)
                    plot(DD_TEMPLATE_VS_PULSER, (*it)->GetEnergy(),
                         (*itA)->GetEnergy());
        }
    }
    return(true)
}
