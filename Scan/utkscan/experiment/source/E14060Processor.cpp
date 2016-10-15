/** \file E14060Processor.cpp
 * \brief Experiment specific processor to handle the e14060 experiment at
 * the NSCL.
 *\author S. V. Paulauskas
 *\date September 15, 2016
 */
#include <iostream>
#include <TimingMapBuilder.hpp>

#include "DetectorDriver.hpp"
#include "GeProcessor.hpp"
#include "E14060Processor.hpp"
#include "PspmtProcessor.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    namespace experiment {
        const int DD_QDCVSTOF = 0; //!<QDC vs ToF
        const int DD_TACS = 6; //!< Spectra from TACS
        const int D_HAGRID = 7; //!< HAGRiD spectra
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void E14060Processor::DeclarePlots(void) {
    DeclareHistogram2D(DD_QDCVSTOF, SC, SD, "QDC CTof- No Tape Move");
    DeclareHistogram2D(DD_TACS, S3, SD, "Tacs");
    DeclareHistogram1D(D_HAGRID, SE, "HAGRiD");
}

E14060Processor::E14060Processor() : EventProcessor(OFFSET, RANGE,
                                                    "E14060PRocessor") {
    associatedTypes.insert("vandle");
    associatedTypes.insert("labr3");
    associatedTypes.insert("pspmt");
    associatedTypes.insert("ge");
    associatedTypes.insert("tac");
    associatedTypes.insert("si");
}

bool E14060Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);

    static const double plotMult = 2;
    static const double plotOffset = 1000;

    BarMap vbars;
    vector<ChanEvent *> geEvts;
    vector<vector<AddBackEvent>> geAddback;
    pair<double, double> position;

    if (event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();
    if (event.GetSummary("pspmt:anode")->GetList().size() != 0)
        position = ((PspmtProcessor *) DetectorDriver::get()->
                GetProcessor("PspmtProcessor"))->GetPosition("qdc");
    if (event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetGeEvents();
        geAddback = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetAddbackEvents();
    }

    static const vector<ChanEvent *> &pin =
            event.GetSummary("si:pin")->GetList();

    static const vector<ChanEvent *> &dynode =
            event.GetSummary("pspmt:dynode")->GetList();

    TimingMapBuilder startbuilder(dynode);
    TimingMap tdynode = startbuilder.GetMap();

    static const vector<ChanEvent *> &hagrid =
            event.GetSummary("hagrid")->GetList();

    static const vector<ChanEvent *> &tac =
            event.GetSummary("tac")->GetList();

    //Here we will check some of the correlation information
    bool hasIon = pin.size() != 0;
    bool hasImplant = hasIon && dynode.size() != 0;
    bool hasDecay = !hasIon && dynode.size() != 0;


    for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
        BarDetector bar = (*it).second;

        if (!bar.GetHasEvent())
            continue;

        TimingCalibration cal = bar.GetCalibration();

        for (TimingMap::iterator itStart = tdynode.begin();
             itStart != tdynode.end(); itStart++) {

            if (!(*itStart).second.GetIsValid())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            HighResTimingData start = (*itStart).second;

            double tof = bar.GetCorTimeAve() -
                         start.GetCorrectedTime() + cal.GetTofOffset(startLoc);

            double corTof =
                    ((VandleProcessor *) DetectorDriver::get()->
                            GetProcessor("VandleProcessor"))->
                            CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            plot(DD_QDCVSTOF, tof * plotMult + plotOffset, bar.GetQdc());
        }//for(TimingMap::iterator start = tdynode.begin();
    } //for(BarMap::iterator it = vbars.begin()

    //-------------- LaBr3 Processing ---------------
    for (vector<ChanEvent *>::const_iterator it = hagrid.begin();
         it != hagrid.end(); it++)
        plot(D_HAGRID, (*it)->GetEnergy());

    //-------------- TAC Processing ---------------
    for (vector<ChanEvent *>::const_iterator it = tac.begin();
         it != tac.end(); it++)
        plot(DD_TACS, (*it)->GetEnergy()), (*it)->GetChanID().GetLocation();

    EndProcess();
    return (true);
}
