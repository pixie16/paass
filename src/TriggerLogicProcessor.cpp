/** \file TriggerLogicProcessor.cpp
 * \brief Make a nifty trigger/logic graph derived from MtcProcessor.cpp
 */

#include <string>
#include <vector>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TriggerLogicProcessor.hpp"

using namespace std;
using namespace dammIds::logic;
using namespace dammIds::triggerlogic;

namespace dammIds {
    namespace triggerlogic {
        const int DD_RUNTIME_LOGIC = 80;
    }
} 

TriggerLogicProcessor::TriggerLogicProcessor(void) : LogicProcessor(dammIds::triggerlogic::OFFSET, 
                                                                    dammIds::triggerlogic::RANGE)
{
    name = "triggerlogic";
}

void TriggerLogicProcessor::DeclarePlots(void)
{

}

bool TriggerLogicProcessor::Process(RawEvent &event)
{


    EndProcess(); // update processing time
    return true;
}
