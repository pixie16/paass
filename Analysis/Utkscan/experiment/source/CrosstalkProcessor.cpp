/** \file CrosstalkProcessor.cpp
 *\brief A Template processor class that can be used to build your own.
 *\author S. V. Paulauskas
 *\date October 26, 2014
 */
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "CrosstalkProcessor.hpp"

namespace dammIds {
    namespace vandle {
        namespace crosstalk {
            const int DD_TOFBARBVSBARA      = 0;//!< ToF Bar A vs. ToF Bar B
            const int DD_GATEDTQDCAVEVSTOF  = 1;//!< Gated QDC vs. ToF
            const int D_CROSSTALK           = 2;//!< Cross talk histogram
        }
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::template;

CrosstalkProcessor::CrosstalkProcessor():
    EventProcessor(dammIds::template::OFFSET, dammIds::template::RANGE,
                   "template") {
    associatedTypes.insert("template");
}

void CrosstalkProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_CROSSTALK, SC, "CrossTalk Between Two Bars");
    DeclareHistogram2D(DD_GATEDTQDCAVEVSTOF, SC, SD,
    "<E> vs. TOF0 (0.5ns/bin) - Gated");
    DeclareHistogram2D(DD_TOFBARBVSBARA, SC, SC, "TOF Bar1 vs. Bar2");
    //DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
    //DeclareHistogram1D(, SD, "Muons");
    //DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
}

bool CrosstalkProcessor::PreProcess(RawEvent &event) {
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

bool CrosstalkProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    VandleProcessor::Process(event);

        //This whole routine is stupidly written, it needs cleaned up something fierce.
    if(barMap.size() < 2)
        return;

    for(BarMap::iterator itBarA = barMap.begin();
    itBarA != barMap.end(); itBarA++) {
            BarMap::iterator itTemp = itBarA;
            itTemp++;

            const double &timeAveA = (*itBarA).second.timeAve;
            const unsigned int &locA = (*itBarA).first.first;

            for(BarMap::iterator itBarB = itTemp; itBarB != barMap.end();
            itBarB++) {
                    if((*itBarA).first.second != (*itBarB).first.second)
                        continue;
                    if((*itBarA).first == (*itBarB).first)
                        continue;

                    const double &timeAveB = (*itBarB).second.timeAve;
                    const unsigned int &locB = (*itBarB).first.first;

                    CrossTalkKey bars(locA, locB);
                    crossTalk.insert(make_pair(bars, timeAveB - timeAveA));
            }
    }

    string barType = "small";
    TimingDefs::BarIdentifier barA(0, barType);
    TimingDefs::BarIdentifier barB(1, barType);

    CrossTalkKey barsOfInterest(barA.first, barB.first);

    CrossTalkMap::iterator itBars =
        crossTalk.find(barsOfInterest);

    const int resMult = 2; //!<set resolution of histograms
    const int resOffset = 200; //!< set offset of histograms

    if(itBars != crossTalk.end())
        plot(D_CROSSTALK, (*itBars).second * resMult + resOffset);

    BarMap::iterator itBarA = barMap.find(barA);
    BarMap::iterator itBarB = barMap.find(barB);

    if(itBarA == barMap.end() || itBarB == barMap.end())
        return;

//    TimeOfFlightMap::iterator itTofA =
//  (*itBarA).second.timeOfFlight.find(startLoc);
//     TimeOfFlightMap::iterator itTofB =
//  (*itBarB).second.timeOfFlight.find(startLoc);

//     if(itTofA == (*itBarA).second.timeOfFlight.end() ||
//        itTofB == (*itBarB).second.timeOfFlight.end())
//  return;

//     double tofA = (*itTofA).second;
//     double tofB = (*itTofB).second;
    double tdiffA = (*itBarA).second.walkCorTimeDiff;
    double tdiffB = (*itBarB).second.walkCorTimeDiff;
    double qdcA = (*itBarA).second.qdc;
    double qdcB = (*itBarB).second.qdc;

    //bool onBar = (tdiffA + tdiffB <= 0.75 && tdiffA + tdiffB >= 0.25);
    bool muon = (qdcA > 7500 && qdcB > 7500);

    double muonTOF =
        (*itBarA).second.timeAve - (*itBarB).second.timeAve;

    plot(3950, tdiffA*resMult+100, tdiffB*resMult+100);

    if(muon) {
            plot(3951, tdiffA*resMult+100, tdiffB*resMult+100);
            plot(3952, muonTOF*resMult*10 + resOffset);
    }

//     plot(DD_TOFBARBVSBARA, tofA*resMult+resOffset,
//       tofB*resMult+resOffset);

//     if((tofB > tofA) && (tofB < (tofA+150))) {
//      plot(DD_GATEDTQDCAVEVSTOF, tofA*resMult+resOffset,
//           (*itBarA).second.qdc);
//     }




    return(true)
}
