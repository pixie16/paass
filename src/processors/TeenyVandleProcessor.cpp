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
        const int D_TIMEDIFF         = 0;//!< Time Difference
        const int DD_TQDC            = 1;//!< QDC for the bars
        const int DD_MAX             = 2;//!< Maximum values for the bars
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
                   "TeenyVandleProcessor") {
    associatedTypes.insert("tvandle");
}

void TeenyVandleProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TIMEDIFF, SE, "Time Difference");
    DeclareHistogram2D(DD_TQDC, SD, S1,"QDC");
	DeclareHistogram2D(DD_MAX, SC, S1, "Max");
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
    data_.clear();
    if (!EventProcessor::PreProcess(event))
        return(false);

    static const vector<ChanEvent*> & events =
        event.GetSummary("tvandle")->GetList();

    for(vector<ChanEvent*>::const_iterator it = events.begin();
        it != events.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
        string subType = (*it)->GetChanID().GetSubtype();
        TimingDefs::TimingIdentifier id(location, subType);
        data_.insert(make_pair(id, HighResTimingData(*it)));
    }

    if(data_.size() != 2)
        return(false);

    HighResTimingData right = (*data_.find(make_pair(0,"right"))).second;
    HighResTimingData left  = (*data_.find(make_pair(0,"left"))).second;

    double timeDiff = left.GetHighResTime() - right.GetHighResTime();
    double corTimeDiff = left.GetCorrectedTime() - right.GetCorrectedTime();

    plot(DD_QDCVSMAX, right.GetMaximumValue(), right.GetTraceQdc());

    if(right.GetIsValidData() && left.GetIsValidData()) {
            double timeRes = 50; //20 ps/bin
            double timeOff = 500;

            plot(D_TIMEDIFF, timeDiff*timeRes + timeOff);
            plot(DD_PVSP, right.GetPhase()*timeRes, left.GetPhase()*timeRes);
            plot(DD_MAXRIGHTVSTDIFF, timeDiff*timeRes+timeOff, right.GetMaximumValue());
            plot(DD_MAXLEFTVSTDIFF, timeDiff*timeRes+timeOff, left.GetMaximumValue());

            plot(DD_MAX, right.GetMaximumValue(), 0);
            plot(DD_MAX, left.GetMaximumValue(), 1);
            plot(DD_TQDC, right.GetTraceQdc(), 0);
            plot(DD_TQDC, left.GetTraceQdc(), 1);
            plot(DD_SNRANDSDEV, right.GetSignalToNoiseRatio()+50, 0);
            plot(DD_SNRANDSDEV, right.GetStdDevBaseline()*timeRes+timeOff, 1);
            plot(DD_SNRANDSDEV, left.GetSignalToNoiseRatio()+50, 2);
            plot(DD_SNRANDSDEV, left.GetStdDevBaseline()*timeRes+timeOff, 3);

            double ampDiff = fabs(right.GetMaximumValue()-left.GetMaximumValue());
            if(ampDiff <=50)
                plot(DD_MAXLVSTDIFFAMP, timeDiff*timeRes+timeOff,
                     left.GetMaximumValue());

            plot(DD_MAXLCORGATE, corTimeDiff*timeRes+timeOff,
                 left.GetMaximumValue());

            if(right.GetMaximumValue() > 2500) {
                plot(DD_MAXLVSTDIFFGATE, timeDiff*timeRes+timeOff,
                     left.GetMaximumValue());
            }
    }
    return(true);
}

bool TeenyVandleProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    return(true);
}
