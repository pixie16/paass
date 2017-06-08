///@file Anl1471Processor.cpp
///@brief A class to process data from ANL1471 experiment using VANDLE.
///@author S. Z. Taylor and S. V. Paulauskas
///@date July 14, 2015
#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>

#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DoubleBetaProcessor.hpp"
#include "DetectorDriver.hpp"
#include "GeProcessor.hpp"
#include "Anl1471Processor.hpp"
#include "VandleProcessor.hpp"

TFile *rootfile_;
TTree *roottree1_;
TTree *roottree2_;
TH2D *qdctof_;
TH1D *Vsize;
TH1D *Bsize;
TH1D *Gsize;
TH2D *BETA;
static double qdc_;

unsigned int barType;

struct VandleRoot {
    double tof;
    double qdc;
    double snrl;
    double snrr;
    double pos;
    double tdiff;
    double ben;
    double bqdcl;
    double bqdcr;
    double bsnrl;
    double bsnrr;
    double cyc;
    double bcyc;
    double HPGE;
    int vid;
    int vtype;
    int bid;
};

struct TapeInfo {
    bool move;
    bool beam;
};

struct GammaRoot {
    double gen;
    double gtime;
    double gcyc;//might be repetative with gtime?
    double gben;
    double gbtime;
    double gbcyc;
    int gid;
    int gbid;
};

TapeInfo tapeinfo;
VandleRoot vroot;
GammaRoot groot;

namespace dammIds {
    namespace experiment {
        const int DD_DEBUGGING0 = 0;
        const int DD_DEBUGGING1 = 1;
        const int DD_DEBUGGING2 = 2;
        const int DD_DEBUGGING3 = 3;
        const int DD_DEBUGGING4 = 4;
        const int DD_MedCTOFvQDC = 5;
        const int DD_MedVetoed = 6;
        const int DD_SmCTOFvQDC = 7;
        const int DD_SmVetoed = 8;
        const int DD_DEBUGGING9 = 9;
        const int D_tape = 10;
        const int D_beam = 11;
        const int DD_grow_decay = 12;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void Anl1471Processor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DEBUGGING0, SB, SD, "left-MaxValvsTDIFF");
    DeclareHistogram2D(DD_DEBUGGING1, SB, SD, "right-MaxValvsTDIFF");
    DeclareHistogram2D(DD_DEBUGGING2, SB, S6, "TDIFF-vandle");
    DeclareHistogram1D(DD_DEBUGGING3, S7, "Vandle Multiplicity");
    DeclareHistogram1D(DD_DEBUGGING4, S7, "Beta Multiplicity");
    DeclareHistogram2D(DD_MedCTOFvQDC, SC, SD, "ANL-medium-<E>-vs-CorTof");
    DeclareHistogram2D(DD_MedVetoed, SC, SD, "ANL-medium-vetoed");
    DeclareHistogram2D(DD_SmCTOFvQDC, SC, SD, "ANL-small-<E>-vs-CorTof");
    DeclareHistogram2D(DD_SmVetoed, SC, SD, "ANL-small-vetoed");
    DeclareHistogram2D(DD_DEBUGGING9, SD, S6, "BSNRLvsBQDCL");
    DeclareHistogram1D(D_tape, S1, "tape move");
    DeclareHistogram1D(D_beam, S1, "beam on/off");
    DeclareHistogram2D(DD_grow_decay, SC, SA, "Grow/Decay");
}//end declare plots

Anl1471Processor::Anl1471Processor() : EventProcessor(OFFSET, RANGE,
                                                      "Anl1471PRocessor") {
    associatedTypes.insert("vandle");
    associatedTypes.insert("beta");
    associatedTypes.insert("ge");

    stringstream name;
    name << Globals::get()->GetOutputPath()
         << Globals::get()->GetOutputFileName() << ".root";
    rootfile_ = new TFile(name.str().c_str(), "RECREATE");

    roottree1_ = new TTree("V", "");
    roottree2_ = new TTree("G", "");

    roottree1_->Branch("vandle", &vroot,
                       "tof/D:qdc/D:snrl/D:snrr/D:pos/D:tdiff/D:ben/D:bqdcl/D:bqdcr/D:bsnrl/D:bsnrr/D:cyc/D:bcyc/D:HPGE/D:vid/I:vtype/I:bid/I");
    roottree1_->Branch("tape", &tapeinfo, "move/b:beam/b");

    roottree2_->Branch("gamma", &groot,
                       "gen/D:gtime/D:gcyc/D:gben/D:gbtime/D:gbcyc/D:gid/I:gbid/I");
    roottree2_->Branch("tape", &tapeinfo, "move/b:beam/b");

    qdctof_ = new TH2D("qdctof", "", 1000, -100, 900, 16000, 0, 16000);
    Vsize = new TH1D("Vsize", "", 40, 0, 40);
    Bsize = new TH1D("Bsize", "", 40, 0, 40);
    Gsize = new TH1D("Gsize", "", 40, 0, 40);
    BETA = new TH2D("BETA", "", 8192, 0, 8192, 64, 0, 64);
}

Anl1471Processor::~Anl1471Processor() {
    rootfile_->Write();
    rootfile_->Close();
    delete (rootfile_);
}

bool Anl1471Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    double plotMult_ = 2;
    double plotOffset_ = 1000;

    BarMap vbars, betas;
    map<unsigned int, pair<double, double> > lrtBetas;

    BarMap betaStarts_;
    vector < ChanEvent * > geEvts;
    vector <vector<AddBackEvent>> geAddback;

    if (event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();


    static const vector<ChanEvent *> &doubleBetaStarts =
            event.GetSummary("beta:double:start")->GetList();
    BarBuilder startBars(doubleBetaStarts);
    startBars.BuildBars();
    betaStarts_ = startBars.GetBarMap();

    if (event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetGeEvents();
        geAddback = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetAddbackEvents();
    }

    Vsize->Fill(vbars.size());
    Bsize->Fill(betaStarts_.size());
    Gsize->Fill(geEvts.size());


    if (TreeCorrelator::get()->place("TapeMove")->status()) {
        tapeinfo.move = 1;
    } else {
        tapeinfo.move = 0;
    }
    if (TreeCorrelator::get()->place("Beam")->status()) {
        tapeinfo.beam = 1;
    } else {
        tapeinfo.beam = 0;
    }
    plot(D_tape, tapeinfo.move);
    plot(D_beam, tapeinfo.beam);
    plot(DD_DEBUGGING3, vbars.size());
    plot(DD_DEBUGGING4, betaStarts_.size());

    //Begin processing for VANDLE bars
    for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if (!bar.GetHasEvent())
            continue;

        if (bar.GetType() == "small")
            barType = 0;
        else if (bar.GetType() == "medium")
            barType = 1;

        unsigned int barLoc = barId.first;
        const TimingCalibration cal = bar.GetCalibration();


        for (BarMap::iterator itStart = betaStarts_.begin();
             itStart != betaStarts_.end(); itStart++) {
            BarDetector beta_start = (*itStart).second;
            unsigned int startLoc = (*itStart).first.first;
            if (!beta_start.GetHasEvent())
                continue;
            double tofOffset = cal.GetTofOffset(startLoc);
            double tof = bar.GetCorTimeAve() -
                         beta_start.GetCorTimeAve() + tofOffset;

            double corTof =
                    ((VandleProcessor *) DetectorDriver::get()->
                            GetProcessor("VandleProcessor"))->
                            CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            //tape move veto cut damm
            bool tapeMove = TreeCorrelator::get()->place("TapeMove")->status();
            if (tapeMove == 0) { //plot only if tape is NOT moving
                if (bar.GetType() == "medium")
                    plot(DD_MedCTOFvQDC, corTof * 2 + 1000, bar.GetQdc());

                if (bar.GetType() == "small")
                    plot(DD_SmCTOFvQDC, corTof * 2 + 1000, bar.GetQdc());
            }

            if (tapeMove == 1) { //plot only if tape is moving
                if (bar.GetType() == "medium")
                    plot(DD_MedVetoed, corTof * 2 + 1000, bar.GetQdc());

                if (bar.GetType() == "small")
                    plot(DD_SmVetoed, corTof * 2 + 1000, bar.GetQdc());
            }

            plot(DD_DEBUGGING9, beta_start.GetLeftSide().GetTraceQdc(),
                 beta_start.GetLeftSide().GetTrace().GetSignalToNoiseRatio());

            //adding HPGE energy info to vandle tree
            double HPGE_energy = -9999.0;
            if (geEvts.size() != 0) {
                for (vector<ChanEvent *>::const_iterator itHPGE = geEvts.begin();
                     itHPGE != geEvts.end(); itHPGE++) {
                    HPGE_energy = (*itHPGE)->GetCalibratedEnergy();
                }

            } else {
                HPGE_energy = -8888.0;
            }

            vroot.tof = corTof * 2 + 1000;//to make identicle to damm output
            vroot.qdc = bar.GetQdc();
            vroot.snrl = bar.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
            vroot.snrr = bar.GetRightSide().GetTrace().GetSignalToNoiseRatio();
            vroot.pos = bar.GetQdcPosition();
            vroot.tdiff = bar.GetTimeDifference();
            vroot.ben = beta_start.GetQdc();
            vroot.bqdcl = beta_start.GetLeftSide().GetTrace().GetQdc();
            vroot.bqdcr = beta_start.GetRightSide().GetTrace().GetQdc();
            vroot.bsnrl = beta_start.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
            vroot.bsnrr = beta_start.GetRightSide().GetTrace().GetSignalToNoiseRatio();
            vroot.cyc = 0;  /////////it.GetEventTime();
            vroot.bcyc = 0;  /////////itStart.GetEventTime()
            vroot.HPGE = HPGE_energy;
            vroot.vid = barLoc;
            vroot.vtype = barType;
            vroot.bid = startLoc;


            BETA->Fill(vroot.bqdcl, vroot.bsnrl);
            qdctof_->Fill(tof, bar.GetQdc());
            qdc_ = bar.GetQdc();
            tof = tof;
            roottree1_->Fill();
            // bar.GetLeftSide().ZeroRootStructure(leftVandle);
            // bar.GetRightSide().ZeroRootStructure(rightVandle);
            // beta_start.GetLeftSide().ZeroRootStructure(leftBeta);
            // beta_start.GetRightSide().ZeroRootStructure(rightBeta);
            qdc_ = tof = -9999;
            //VID = BID = SNRVL = SNRVR = -9999;
            //GamEn = SNRBL = SNRBR = vandle_ = beta_ = ge_ = -9999;

            plot(DD_DEBUGGING1, tof * plotMult_ + plotOffset_, bar.GetQdc());

        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
    //End processing for VANDLE bars

    //Stuff to fill HPGe branch
    if (geEvts.size() != 0) {
        for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
             itGe != geEvts.end(); itGe++) {
            double ge_energy, ge_time, gb_time_L, gb_time_R, gb_time, grow_decay_time, gb_en, gcyc_time;
            ge_energy = ge_time = gb_time_L = gb_time_R = gb_time = grow_decay_time = gb_en = gcyc_time = -9999.0;
            int ge_id = -9999;
            int gb_startLoc = -9999;
            BarDetector gb_start;
            ge_energy = (*itGe)->GetCalibratedEnergy();
            ge_id = (*itGe)->GetChanID().GetLocation();
            ge_time = (*itGe)->GetWalkCorrectedTime();
            ge_time *= (Globals::get()->GetClockInSeconds() * 1.e9);//in ns now

            if (TreeCorrelator::get()->place("Cycle")->status()) {
                gcyc_time = TreeCorrelator::get()->place("Cycle")->last().time;
                gcyc_time *= (Globals::get()->GetClockInSeconds() *
                              1.e9);//in ns now
                grow_decay_time =
                        (ge_time - gcyc_time) * 1e-9 * 1e2;//in seconds, then ms
                //cout << ge_energy << endl << grow_decay_time << endl << endl;
                plot(DD_grow_decay, ge_energy, grow_decay_time);
            }

            if (doubleBetaStarts.size() != 0) {
                for (BarMap::iterator itGB = betaStarts_.begin();
                     itGB != betaStarts_.end(); itGB++) {
                    gb_start = (*itGB).second;
                    gb_startLoc = (*itGB).first.first;
                    gb_en = gb_start.GetQdc();
                    gb_time_L = gb_start.GetLeftSide().GetHighResTimeInNs();
                    //GetCorrectedTime()??
                    gb_time_R = gb_start.GetRightSide().GetHighResTimeInNs();
                    //GetTimeAverage()??
                    gb_time = (gb_time_L + gb_time_R) / 2;
                    gb_time *= (Globals::get()->GetClockInSeconds() *
                                1.e9);//in ns now
                }
            } else {
                gb_startLoc = -9999;
                gb_en = -9999;
                gb_time = -9999;
            }

            groot.gen = ge_energy;
            groot.gtime = ge_time;
            groot.gcyc = ge_time - gcyc_time;
            groot.gben = gb_en;
            groot.gbtime = gb_time;
            groot.gbcyc = gb_time - gcyc_time;
            groot.gid = ge_id;
            groot.gbid = gb_startLoc;
            roottree2_->Fill();
        }
    }

    EndProcess();
    return (true);
}