///@file Ornl2016Processor.cpp
///@brief A class to process data from the ORNL 2016 OLTF experiment using
///VANDLE.
///@author S. V. Paulauskas
///@date February 10, 2016
#include <fstream>

#include "BarBuilder.hpp"
#include "DetectorDriver.hpp"
#include "DoubleBetaProcessor.hpp"
#include "GeProcessor.hpp"
#include "Ornl2016Processor.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    namespace experiment {
        const int D_VANDLEMULT = 0;

        const int DD_QDCTOFNOGATE = 2;
        const int DD_QDCVSCORTOFMULT1 = 4;
        const int DD_LIGLEN = 6;
        const int DD_LIGLTOF = 7;

        const int D_LABR3SUM = 8;
        const int D_LABR3BETA = 9;

        const int D_NAISUM = 10;
        const int D_NAIBETA = 11;

        const int DD_TOFVSNAI = 12;
        const int DD_TOFVSHAGRID = 13;
        const int DD_TOFVSGE = 14;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void Ornl2016Processor::DeclarePlots(void) {
    DeclareHistogram1D(D_VANDLEMULT, S7, "Vandle Multiplicity");
    DeclareHistogram2D(DD_QDCTOFNOGATE, SC, SD, "QDC ToF Ungated");
    DeclareHistogram2D(DD_QDCVSCORTOFMULT1, SC, SC, "QDC vs Cor Tof Mult1");
    DeclareHistogram2D(DD_LIGLEN, SC, S5, "E - LiGlass");
    DeclareHistogram2D(DD_LIGLTOF, SC, SC, "E vs ToF - LiGlass");

    DeclareHistogram1D(D_LABR3SUM, SC, "HAGRiD summed");
    DeclareHistogram1D(D_LABR3BETA, SC, "HAGRiD summed - BETA GATED");
    DeclareHistogram1D(D_NAISUM, SC, "NaI summed");
    DeclareHistogram1D(D_NAIBETA, SC, "NaI summed - BETA GATED");

    DeclareHistogram2D(DD_TOFVSNAI, SC, SB, "ToF vs. NaI");
    DeclareHistogram2D(DD_TOFVSHAGRID, SC, SB, "ToF vs. HAGRiD");
    DeclareHistogram2D(DD_TOFVSGE, SC, SB, "ToF vs. Ge");

}

Ornl2016Processor::Ornl2016Processor() : EventProcessor(OFFSET, RANGE,
                                                        "Ornl2016Processor"){
    associatedTypes.insert("vandle");
    associatedTypes.insert("liglass");
    associatedTypes.insert("nai");
    associatedTypes.insert("labr3");
    associatedTypes.insert("beta");
}

bool Ornl2016Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);

    double plotMult_ = 2;
    double plotOffset_ = 1000;

    BarMap vbars, betas;
    map<unsigned int, pair<double, double> > lrtBetas;
    vector<ChanEvent *> geEvts;
    vector<vector<AddBackEvent> > geAddback;

    //Ask the VandleProcessor for the bars that it built
    if (event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();

    //Ask the DoubleBetaProcessor for the high and low res bars that it build
    if (event.GetSummary("beta:double")->GetList().size() != 0) {
        betas = ((DoubleBetaProcessor *) DetectorDriver::get()->
                GetProcessor("DoubleBetaProcessor"))->GetBars();
        lrtBetas = ((DoubleBetaProcessor *) DetectorDriver::get()->
                GetProcessor("DoubleBetaProcessor"))->GetLowResBars();
    }

    //Ask the GeProcessor for the raw and addback clover events.
    if (event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetGeEvents();
        geAddback = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetAddbackEvents();
    }

    static const vector<ChanEvent *> &liEvts =
            event.GetSummary("liglass")->GetList();
    static const vector<ChanEvent *> &labr3Evts =
            event.GetSummary("labr3")->GetList();
    static const vector<ChanEvent *> &naiEvts =
            event.GetSummary("nai")->GetList();

    bool hasMultOne = vbars.size() == 1;

    if (vbars.size() != 0)
        plot(D_VANDLEMULT, vbars.size());

    for (BarMap::iterator itStart = betas.begin();
         itStart != betas.end(); itStart++) {

        unsigned int startLoc = (*itStart).first.first;
        BarDetector start = (*itStart).second;

        if (vbars.size() != 0) {
            for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
                TimingDefs::TimingIdentifier barId = (*it).first;
                BarDetector bar = (*it).second;

                if (!bar.GetHasEvent())
                    continue;

                TimingCalibration cal = bar.GetCalibration();

                double tofOffset = cal.GetTofOffset(startLoc);
                double tof = bar.GetCorTimeAve() -
                             start.GetCorTimeAve() + tofOffset;

                double corTof = ((VandleProcessor *) DetectorDriver::get()
                        ->GetProcessor("VandleProcessor"))->
                        CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

                plot(DD_QDCTOFNOGATE, tof * plotMult_ + plotOffset_,
                     bar.GetQdc());
                if (hasMultOne)
                    plot(DD_QDCVSCORTOFMULT1, corTof * plotMult_ + plotOffset_,
                         bar.GetQdc());

                for (vector<ChanEvent *>::const_iterator it = labr3Evts.begin();
                     it != labr3Evts.end(); it++)
                    plot(DD_TOFVSHAGRID, (*it)->GetCalEnergy(),
                         tof * plotMult_ + 200);

                for (vector<ChanEvent *>::const_iterator naiIt = naiEvts.begin();
                     naiIt != naiEvts.end(); naiIt++)
                    plot(DD_TOFVSNAI, (*naiIt)->GetCalEnergy(),
                         tof * plotMult_ + 200);

                for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
                     itGe != geEvts.end(); itGe++)
                    plot(DD_TOFVSGE, (*itGe)->GetCalEnergy(),
                         tof * plotMult_ + 200);
            } //(BarMap::iterator itBar
        } //if(vbars.size != 0)

        for (vector<ChanEvent *>::const_iterator liIt = liEvts.begin();
             liIt != liEvts.end(); liIt++) {
            unsigned int id = (*liIt)->GetID();

            //Beta gated energy
            plot(DD_LIGLEN, (*liIt)->GetEnergy(), id);

            double li_tof = ((*liIt)->GetTime() -
                             0.5 * (start.GetLeftSide().GetFilterTime() +
                                    start.GetRightSide().GetFilterTime()));
            if ((*liIt)->GetID() != 12)
                plot(DD_LIGLTOF, li_tof + 1000, (*liIt)->GetEnergy());
        }
    } // for(TimingMap::iterator itStart

    ///PLOTTING THE SINGLES SPECTRA FOR THE LIGLASS
    for (vector<ChanEvent *>::const_iterator liIt = liEvts.begin();
         liIt != liEvts.end(); liIt++)
        plot(DD_LIGLEN, (*liIt)->GetEnergy(), (*liIt)->GetID() - 11);

    ///PLOTTING THE SINGLES AND BETA GATED SPECTRA FOR HAGRiD
    for (vector<ChanEvent *>::const_iterator it = labr3Evts.begin();
         it != labr3Evts.end(); it++) {
        plot(D_LABR3SUM, (*it)->GetCalEnergy());
        if (lrtBetas.size() != 0)
            plot(D_LABR3BETA, (*it)->GetCalEnergy());
    }

    ///PLOTTING THE SINGLES AND BETA GATED SPECTRA FOR NAI
    for (vector<ChanEvent *>::const_iterator naiIt = naiEvts.begin();
         naiIt != naiEvts.end(); naiIt++) {
        plot(D_NAISUM, (*naiIt)->GetCalEnergy());
        if (lrtBetas.size() != 0)
            plot(D_NAIBETA, (*naiIt)->GetCalEnergy());
    }

    EndProcess();
    return (true);
}