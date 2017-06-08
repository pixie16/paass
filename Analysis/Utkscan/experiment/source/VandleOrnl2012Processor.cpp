///@file VandleOrnl2012Processor.cpp
///@brief A class to process data from the LeRIBSS 2012 campaign.
///@author S. V. Paulauskas
///@date February 5, 2015
#include <fstream>

#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "CloverProcessor.hpp"
#include "VandleProcessor.hpp"
#include "VandleOrnl2012Processor.hpp"

///@TODO We need to implement some ROOT output for this.

namespace dammIds {
    namespace experiment {
        const unsigned int DEBUGGING_OFFSET = 70;
        const int DD_DEBUGGING0 = 0 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING1 = 1 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING2 = 2 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING3 = 3 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING4 = 4 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING5 = 5 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING6 = 6 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING7 = 7 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING8 = 8 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING9 = 9 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING10 = 10 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING11 = 11 + DEBUGGING_OFFSET;
        const int DD_DEBUGGING12 = 12 + DEBUGGING_OFFSET;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void VandleOrnl2012Processor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DEBUGGING0, SC, S7, "General Purpose");
    DeclareHistogram2D(DD_DEBUGGING1, SA, SD, "TOF - banana gated");
    DeclareHistogram2D(DD_DEBUGGING2, SA, SD, "TOF - Lower Bars Banana Gated");
    DeclareHistogram2D(DD_DEBUGGING3, SA, SD, "TOF - Upper Bars Banana Gated");
    DeclareHistogram2D(DD_DEBUGGING4, SA, SD, "TOF - Gated 600 keV");
    DeclareHistogram2D(DD_DEBUGGING5, SA, SD, "TOF - Gated 700 keV");
    DeclareHistogram2D(DD_DEBUGGING6, SA, SC, "TOF - Bar 12 Banana Gated");
    DeclareHistogram2D(DD_DEBUGGING7, SA, SA, "Currently Unused");
    DeclareHistogram2D(DD_DEBUGGING8, SA, SA, "Currently Unused");
    DeclareHistogram2D(DD_DEBUGGING9, SC, S7, "Bar Loc vs. TOF - gated");
    DeclareHistogram2D(DD_DEBUGGING10, SB, SA, "Decay Time vs. ToF");
    DeclareHistogram2D(DD_DEBUGGING11, SC, SA, "Decay Time vs. GEnergy");
    DeclareHistogram2D(DD_DEBUGGING12, SA, SA, "Decay Time vs. ToF - banana");

    DeclareHistogram2D(DEBUGGING_OFFSET + 20, SA, SA, "QDC TOF - Lower 0");
    DeclareHistogram2D(DEBUGGING_OFFSET + 21, SA, SA, "QDC TOF - Lower 1");
    DeclareHistogram2D(DEBUGGING_OFFSET + 22, SA, SA, "QDC TOF - Upper 0");
    DeclareHistogram2D(DEBUGGING_OFFSET + 23, SA, SA, "QDC TOF - Upper 1");
    DeclareHistogram2D(DEBUGGING_OFFSET + 24, SA, SD, "QDC TOF - Upper ");
    DeclareHistogram2D(DEBUGGING_OFFSET + 25, SA, SD, "QDC TOF - Lower");
}

VandleOrnl2012Processor::VandleOrnl2012Processor() :
        EventProcessor(OFFSET, RANGE, "VandleOrnl2012Processor") {
    associatedTypes.insert("vandle");

    stringstream name;
    name << Globals::get()->GetOutputPath()
         << Globals::get()->GetOutputFileName();
    fileName_ = name.str();
    fileNames_.push_back(fileName_ + "-tof.dat");
    fileNames_.push_back(fileName_ + "-tof-02Plus.dat");
    fileNames_.push_back(fileName_ + "-tof-04Plus.dat");
}

bool VandleOrnl2012Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    double plotMult_ = 2;
    double plotOffset_ = 1000;

    BarMap vbars, betas;
    ///@TODO Update the BetaProcessor so that it actually generates this map.
    TimingMap starts;
    vector < ChanEvent * > geEvts;
    vector <vector<AddBackEvent>> geAddback;

    if (event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();
    if (event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((CloverProcessor *) DetectorDriver::get()->
                GetProcessor("CloverProcessor"))->GetGeEvents();
        geAddback = ((CloverProcessor *) DetectorDriver::get()->
                GetProcessor("CloverProcessor"))->GetAddbackEvents();
    }

    for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;
        if (!bar.GetHasEvent())
            continue;

        unsigned int barLoc = barId.first;
        TimingCalibration cal = bar.GetCalibration();

        bool isLower = barLoc > 6 && barLoc < 16;
        bool isUpper = barLoc > 29 && barLoc < 39;
        bool isCleared = isLower;
        int bananaNum = 1;

        for (TimingMap::iterator itStart = starts.begin();
             itStart != starts.end(); itStart++) {
            HighResTimingData start = (*itStart).second;
            if (!start.GetIsValid())
                continue;

            unsigned int startLoc = (*itStart).first.first;

            //!--------- CUT On Beta Energy ----------
            if (start.GetEnergy() < 1000)
                continue;

            double tofOffset = cal.GetTofOffset(startLoc);
            double tof = bar.GetCorTimeAve() -
                         start.GetWalkCorrectedTime() + tofOffset;
            double corTof =
                    ((VandleProcessor *) DetectorDriver::get()->
                            GetProcessor("VandleProcessor"))->
                            CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            bool inPeel = histo.BananaTest(bananaNum,
                                           corTof * plotMult_ + plotOffset_,
                                           bar.GetQdc());
            if (isLower) {
                if (startLoc == 0)
                    plot(DEBUGGING_OFFSET + 20,
                         tof * plotMult_ + plotOffset_, bar.GetQdc());
                else
                    plot(DEBUGGING_OFFSET + 21,
                         tof * plotMult_ + plotOffset_, bar.GetQdc());
            }

            if (isUpper) {
                if (startLoc == 0)
                    plot(DEBUGGING_OFFSET + 22,
                         tof * plotMult_ + plotOffset_, bar.GetQdc());
                else
                    plot(DEBUGGING_OFFSET + 23,
                         tof * plotMult_ + plotOffset_, bar.GetQdc());
            }

            //All of them are gated using a banana gate
            if (inPeel) {
                if (isCleared)
                    plot(DD_DEBUGGING1, corTof * plotMult_ + plotOffset_,
                         bar.GetQdc());

                if (isLower) {
                    ofstream data;
                    data.open(fileNames_[0].c_str(), ios::app);
                    data << corTof << endl;
                    data.close();

                    plot(DD_DEBUGGING2, corTof * plotMult_ + plotOffset_,
                         bar.GetQdc());
                }
                if (isUpper)
                    plot(DD_DEBUGGING3, corTof * plotMult_ + plotOffset_,
                         bar.GetQdc());

                if (barLoc == 12)
                    plot(DD_DEBUGGING6, corTof * plotMult_ + plotOffset_,
                         bar.GetQdc());
            }

            if (isLower)
                plot(DEBUGGING_OFFSET + 24,
                     corTof * plotMult_ + plotOffset_, bar.GetQdc());
            if (isUpper)
                plot(DEBUGGING_OFFSET + 25,
                     corTof * plotMult_ + plotOffset_, bar.GetQdc());

            double cycleTime = TreeCorrelator::get()->place(
                    "Cycle")->last().time;
            cycleTime *= (Globals::get()->GetClockInSeconds() * 1.e9);

            double decayTime = (bar.GetTimeAverage() - cycleTime) / 0.01;

            plot(DD_DEBUGGING10, corTof * plotMult_ + plotOffset_,
                 decayTime * 1.e-9);
            if (inPeel && isCleared)
                plot(DD_DEBUGGING12, corTof * plotMult_ + plotOffset_,
                     decayTime * 1.e-9);

            ofstream gammaGated;
            if (!geEvts.empty() && isCleared && inPeel) {
                for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
                     itGe != geEvts.end(); itGe++) {
                    double calEnergy = (*itGe)->GetCalibratedEnergy();
                    plot(DD_DEBUGGING11, calEnergy, decayTime * 1e-9);
                    if (calEnergy >= 595 && calEnergy <= 603) {
                        if (isLower) {
                            gammaGated.open(fileNames_[1].c_str(), ios::app);
                            gammaGated << corTof << endl;
                        }
                        plot(DD_DEBUGGING4, corTof * plotMult_ + plotOffset_,
                             bar.GetQdc());
                    }
                    if (calEnergy >= 692 && calEnergy <= 704) {
                        if (isLower) {
                            gammaGated.open(fileNames_[2].c_str(), ios::app);
                            gammaGated << corTof << endl;
                        }
                        plot(DD_DEBUGGING5, corTof * plotMult_ + plotOffset_,
                             bar.GetQdc());
                    } // if(calEnergy >= 692
                }// for(vector<ChanEvent *>::const_iterator
                gammaGated.close();
            }//geSummary_->GetMult() > 0
        }//loop over starts
    }//loop over bars
    EndProcess();
    return (true);
}
