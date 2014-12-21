/** \file LiquidScintProcessor.cpp
 *
 * implementation for scintillator processor
 */
#include <vector>
#include <sstream>

#include <cmath>

#include "BarDetector.hpp"
#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "LiquidScintProcessor.hpp"
#include "HighResTimingData.hpp"
#include "TimingCalibrator.hpp"
#include "Trace.hpp"

using namespace std;
using namespace dammIds::liquid_scint;

namespace dammIds {
    namespace liquid_scint {
        const int DD_TQDCLIQUID       = 0;//!< QDC
        const int DD_MAXLIQUID        = 1;//!< Maximum values
        const int DD_DISCRIM          = 2;//!< Discrimination plots
        const int DD_TOFLIQUID        = 3;//!< ToF for liquids
        const int DD_TRCLIQUID        = 4;//!< Traces for the liquid
        const int DD_TQDCVSDISCRIM    = 5;//!< QDC vs. Discrimination
        const int DD_TOFVSDISCRIM     = 6;//!< ToF vs. Discrimination
        const int DD_NEVSDISCRIM      = 8;//!< Neutron Energy vs. Discrimination
        const int DD_TQDCVSLIQTOF     = 10;//!< QDC vs Liquid ToF
        const int DD_TQDCVSENERGY     = 12;//!< QDC vs. Energy
    }
}

LiquidScintProcessor::LiquidScintProcessor() :
    EventProcessor(OFFSET, RANGE, "liquid_scint") {
    associatedTypes.insert("liquid_scint");
}

void LiquidScintProcessor::DeclarePlots(void) {
    /** WARNING
     * This part was commented in the old ScintProcessor and is
     * copied as is. */

    //To handle Liquid Scintillators
    // DeclareHistogram2D(DD_TQDCLIQUID, SC, S3, "Liquid vs. Trace QDC");
    // DeclareHistogram2D(DD_MAXLIQUID, SC, S3, "Liquid vs. Maximum");
    // DeclareHistogram2D(DD_DISCRIM, SA, S3, "N-Gamma Discrimination");
    // DeclareHistogram2D(DD_TOFLIQUID, SE, S3,"Liquid vs. TOF");
    // DeclareHistogram2D(DD_TRCLIQUID, S7, S7, "LIQUID TRACES");

    // for(unsigned int i=0; i < 2; i++) {
    // 	DeclareHistogram2D(DD_TQDCVSDISCRIM+i, SA, SE,"Trace QDC vs. NG Discrim");
    // 	DeclareHistogram2D(DD_TOFVSDISCRIM+i, SA, SA, "TOF vs. Discrim");
    // 	DeclareHistogram2D(DD_NEVSDISCRIM+i, SA, SE, "Energy vs. Discrim");
    // 	DeclareHistogram2D(DD_TQDCVSLIQTOF+i, SC, SE, "Trace QDC vs. Liquid TOF");
    // 	DeclareHistogram2D(DD_TQDCVSENERGY+i, SD, SE, "Trace QDC vs. Energy");
    // }
}

bool LiquidScintProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;
    return true;
}

/**
 * WARNING!
 * This part was the LiquidAnalysis function in the old ScintProcessor.
 * It looks like written for some older version of code.
 * Before using, examine it carefully!
 */
bool LiquidScintProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &liquidEvents =
	event.GetSummary("liquid_scint:liquid")->GetList();
    static const vector<ChanEvent*> &betaStartEvents =
	event.GetSummary("liquid_scint:beta:start")->GetList();
    static const vector<ChanEvent*> &liquidStartEvents =
	event.GetSummary("liquid_scint:liquid:start")->GetList();

    vector<ChanEvent*> startEvents;
    startEvents.insert(startEvents.end(), betaStartEvents.begin(),
		       betaStartEvents.end());
    startEvents.insert(startEvents.end(), liquidStartEvents.begin(),
		       liquidStartEvents.end());

    for(vector<ChanEvent*>::const_iterator itLiquid = liquidEvents.begin();
	itLiquid != liquidEvents.end(); itLiquid++) {
        unsigned int loc = (*itLiquid)->GetChanID().GetLocation();
        HighResTimingData liquid((*itLiquid));

        if(liquid.GetDiscrimination() == 0) {
            for(Trace::const_iterator i = liquid.GetTrace()->begin();
            i != liquid.GetTrace()->end(); i++)
                plot(DD_TRCLIQUID, int(i-liquid.GetTrace()->begin()),
                    counter, int(*i)-liquid.GetAveBaseline());
            counter++;
        }

        if(liquid.GetIsValidData()) {
            plot(DD_TQDCLIQUID, liquid.GetTraceQdc(), loc);
            plot(DD_MAXLIQUID, liquid.GetMaximumValue(), loc);

            double discrimNorm =
            liquid.GetDiscrimination()/liquid.GetTraceQdc();

            double discRes = 1000;
            double discOffset = 100;

            TimingCalibration cal =
                TimingCalibrator::get()->GetCalibration(make_pair(loc, "liquid"));

            if(discrimNorm > 0)
                plot(DD_DISCRIM, discrimNorm*discRes+discOffset, loc);
            plot(DD_TQDCVSDISCRIM, discrimNorm*discRes+discOffset,
            liquid.GetTraceQdc());

            if((*itLiquid)->GetChanID().HasTag("start"))
                continue;

            for(vector<ChanEvent*>::iterator itStart = startEvents.begin();
            itStart != startEvents.end(); itStart++) {
                unsigned int startLoc = (*itStart)->GetChanID().GetLocation();
                HighResTimingData start((*itStart));
                int histLoc = loc + startLoc;
                const int resMult = 2;
                const int resOffset = 2000;

                if(start.GetIsValidData()) {
                    double tofOffset = cal.GetTofOffset(startLoc);
                    double TOF = liquid.GetHighResTime() -
                        start.GetHighResTime() - tofOffset;

                    plot(DD_TOFLIQUID, TOF*resMult+resOffset, histLoc);
                    plot(DD_TOFVSDISCRIM+histLoc,
                        discrimNorm*discRes+discOffset, TOF*resMult+resOffset);
                    plot(DD_TQDCVSLIQTOF+histLoc, TOF*resMult+resOffset,
                        liquid.GetTraceQdc());
                }
            }
        }
    }
    EndProcess();
    return true;
}
