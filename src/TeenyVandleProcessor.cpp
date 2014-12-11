/** \file TeenyVandleProcessor.cpp
 *\brief A Template processor class that can be used to build your own.
 *\author S. V. Paulauskas
 *\date December 11, 2014
 */
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TeenyVandleProcessor.hpp"

namespace dammIds {
    namespace teenyvandle {
        const int DD_TQDCBARS        = 0;//!< QDC for the bars
        const int DD_MAXIMUMBARS     = 1;//!< Maximum values for the bars
        const int D_TIMEDIFF         = 2;//!< Time Difference
        const int DD_PVSP            = 3;//!< Phase-Phase
        const int DD_MAXRIGHTVSTDIFF = 4;//!< Max Right vs. Tdiff
        const int DD_MAXLEFTVSTDIFF  = 5;//!< Max Left vs. Tdiff
        const int DD_MAXLVSTDIFFGATE = 6;//!< MaxLeft vs. Tdiff - Gated
        const int DD_MAXLVSTDIFFAMP  = 7;//!< MaxLeft vs. TDiff - Amp gated
        const int DD_MAXLCORGATE     = 8;//!< MaxLeft vs. Cor ToF - gated
        const int DD_QDCVSMAX        = 9;//!< QDC vs. Max Val
        const int DD_SNRANDSDEV      = 10;//!< SNR and Standard Dev Baseline
    }
}

using namespace std;
using namespace dammIds::teenyvandle;

TeenyVandleProcessor::TeenyVandleProcessor():
    EventProcessor(dammIds::teenyvandle::OFFSET, dammIds::teenyvandle::RANGE,
                   "template") {
    associatedTypes.insert("template");
}

void TeenyVandleProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_TQDCBARS, SD, S1,"QDC");
	DeclareHistogram2D(DD_MAXIMUMBARS, SC, S1, "Max");
	DeclareHistogram1D(D_TIMEDIFF, SE, "Time Difference");
	DeclareHistogram2D(DD_PVSP, SE, SE,"Phase vs. Phase");
	DeclareHistogram2D(DD_MAXRIGHTVSTDIFF, SA, SD,"Max Right vs. Time Diff");
	DeclareHistogram2D(DD_MAXLEFTVSTDIFF, SA, SD, "Max Left vs. Time Diff");
	DeclareHistogram2D(DD_MAXLVSTDIFFGATE, SA, SD,
			   "Max Left vs. Time Diff - gated on max right");
	DeclareHistogram2D(DD_MAXLVSTDIFFAMP, SA, SD,
			   "Max Left vs. Time Diff - amp diff");
	DeclareHistogram2D(DD_MAXLCORGATE, SA, SD,
			   "Max Left vs. Cor Time Diff");
	DeclareHistogram2D(DD_QDCVSMAX, SC, SD,"QDC vs Max - Right");
	DeclareHistogram2D(DD_SNRANDSDEV, S8, S2, "SNR and SDEV R01/L23");
}

bool TeenyVandleProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    static const vector<ChanEvent*> & templateEvents =
        event.GetSummary("vandle:teeny")->GetList();

    for(vector<ChanEvent*>::const_iterator it = templateEvents.begin();
        it != templateEvents.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        string subType = (*it)->GetChanID().GetSubtype();

        TimingDefs::TimingIdentifier id(location, "teeny");
        data_.insert(make_pair(id, HighResTimingData(*it)));
    }
    return(true);
}

bool TeenyVandleProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    static const vector<ChanEvent*> & pulserEvents =
        event.GetSummary("pulser")->GetList();

//    unsigned int location = (*it)->GetChanID().GetLocation();
//    for(vector<ChanEvent*>::const_iterator itA = templateEvents.begin();
//        itA != templateEvents.end(); itA++) {
//        unsigned int location = (*it)->GetChanID().GetLocation();
//        if(location == 0)
//            plot(DD_TEMPLATE_VS_PULSER, (*it)->GetEnergy(),
//                    (*itA)->GetEnergy());
//    }
    return(true);
}


