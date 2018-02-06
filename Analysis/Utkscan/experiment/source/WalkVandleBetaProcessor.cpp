/** \file WalkVandleBetaProcessor.cpp
 *\brief A class to determine the walk between Vandle and the LeRIBSS betas
 *\author S. V. Paulauskas
 *\date February 24, 2015
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "WalkVandleBetaProcessor.hpp"

namespace dammIds {
    namespace vandle {
        const unsigned int DEBUGGING_OFFSET = 70;
        const int DD_DEBUGGING0  = 0+DEBUGGING_OFFSET;
        const int DD_DEBUGGING1  = 1+DEBUGGING_OFFSET;
        const int DD_DEBUGGING2  = 2+DEBUGGING_OFFSET;
        const int DD_DEBUGGING3  = 3+DEBUGGING_OFFSET;
        const int DD_DEBUGGING4  = 4+DEBUGGING_OFFSET;
        const int DD_DEBUGGING5  = 5+DEBUGGING_OFFSET;
        const int DD_DEBUGGING6  = 6+DEBUGGING_OFFSET;
        const int DD_DEBUGGING7  = 7+DEBUGGING_OFFSET;
        const int DD_DEBUGGING8  = 8+DEBUGGING_OFFSET;
        const int DD_DEBUGGING9  = 9+DEBUGGING_OFFSET;
        const int DD_DEBUGGING10  = 10+DEBUGGING_OFFSET;
        const int DD_DEBUGGING11  = 11+DEBUGGING_OFFSET;
        const int DD_DEBUGGING12  = 12+DEBUGGING_OFFSET;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

void WalkVandleBetaProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DEBUGGING0, SA, SE, "");
    DeclareHistogram2D(DD_DEBUGGING1, SA, SE, "");
    DeclareHistogram2D(DD_DEBUGGING2, SA, SD, "");
    DeclareHistogram2D(DD_DEBUGGING3, SA, SD, "");
    DeclareHistogram2D(DD_DEBUGGING4, SD, S1, "");
    DeclareHistogram2D(DD_DEBUGGING5, SA, SD, "");
    DeclareHistogram2D(DD_DEBUGGING6, SA, SD, "");
    DeclareHistogram2D(DD_DEBUGGING7, SA, SD, "");
    DeclareHistogram2D(DD_DEBUGGING8, SA, SD, "");
}

WalkVandleBetaProcessor::WalkVandleBetaProcessor(const std::vector<std::string> &typeList,
    const double &res, const double &offset, const double &numStarts) :
    VandleProcessor(typeList,res,offset,numStarts) {
    associatedTypes.insert("vandle");
}

bool WalkVandleBetaProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    if (!VandleProcessor::Process(event))
        return(false);

    for (BarMap::iterator it = bars_.begin(); it !=  bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;
        if(!bar.GetHasEvent())
            continue;

        unsigned int barLoc = barId.first;
        bool isLower = barLoc > 6 && barLoc < 16;
        if(!isLower)
            continue;

        TimingCalibration cal = bar.GetCalibration();

        if(barLoc == 12) {
            plot(DD_DEBUGGING4, bar.GetLeftSide().GetTraceQdc(), 0);
            plot(DD_DEBUGGING4, bar.GetRightSide().GetTraceQdc(), 1);
            if(bar.GetLeftSide().GetTraceQdc() > 1500)
                plot(DD_DEBUGGING5,
                     bar.GetWalkCorTimeDiff()*plotMult_+plotOffset_,
                     bar.GetRightSide().GetTraceQdc());
            if(bar.GetRightSide().GetTraceQdc() > 1500)
                plot(DD_DEBUGGING6,
                     bar.GetWalkCorTimeDiff()*plotMult_+plotOffset_,
                     bar.GetLeftSide().GetTraceQdc());
            plot(DD_DEBUGGING7,
                bar.GetWalkCorTimeDiff()*plotMult_+plotOffset_,
                bar.GetRightSide().GetTraceQdc());
            plot(DD_DEBUGGING8,
                bar.GetTimeDifference()*plotMult_+plotOffset_,
                bar.GetRightSide().GetTraceQdc());
        }


        for(TimingMap::iterator itStart = starts_.begin();
        itStart != starts_.end(); itStart++) {
            HighResTimingData start = (*itStart).second;
            if(!start.GetIsValidData())
                continue;

            unsigned int startLoc = (*itStart).first.first;

            double tofOffset = cal.GetTofOffset(startLoc);
            double tofBarWalkCor = bar.GetWalkCorTimeAve() -
                start.GetWalkCorrectedTime() + tofOffset;
            double tofBarAve = bar.GetTimeAverage() -
                start.GetWalkCorrectedTime() + tofOffset;

            double thresh = 1500;
            if(startLoc == 0) {
                plot(DD_DEBUGGING2, tofBarWalkCor*plotMult_+plotOffset_, bar.GetQdc());
                plot(DD_DEBUGGING3, tofBarAve*plotMult_+plotOffset_, bar.GetQdc());

                if(bar.GetQdc() > thresh) {
                    plot(DD_DEBUGGING0, tofBarWalkCor*plotMult_+plotOffset_,
                        start.GetPixieEnergy());
                    plot(DD_DEBUGGING1, tofBarAve*plotMult_+plotOffset_,
                        start.GetPixieEnergy());
                }
            }

        }//loop over starts
    }//loop over bars
    EndProcess();
    return(true);
}
