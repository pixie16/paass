/** \file E14060Processor.cpp
 * \brief Experiment specific processor to handle the e14060 experiment at
 * the NSCL.
 *\author S. V. Paulauskas
 *\date September 15, 2016
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "DoubleBetaProcessor.hpp"
#include "DetectorDriver.hpp"
#include "GeProcessor.hpp"
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "E14060Processor.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "VandleProcessor.hpp"

#ifdef useroot
static double tof_;
static double qdc_;
#endif

namespace dammIds {
    namespace experiment {
        const int DD_DEBUGGING0 = 0; //!<QDC CTof- No Tape Move
        const int DD_DEBUGGING1 = 1; //!<QDC ToF Ungated
        const int DD_DEBUGGING2 = 2; //!<Cor ToF vs. Gamma E
        const int DD_DEBUGGING3 = 3; //!<Vandle Multiplicity
        const int DD_DEBUGGING4 = 4; //!<QDC vs Cor Tof Mult1
        const int DD_DEBUGGING5 = 5; //!<Mult2 Sym Plot Tof
        const int DD_TACS = 6; //!< Spectra from TACS
        const int DD_HAGRID = 7; //!< HAGRiD spectra
        const int DD_PROTONBETA2TDIFF_VS_BETA2EN = 13; //!< BetaProton Tdiff vs. Beta Energy
        const int D_ENERGY = 14; //!< Gamma singles ungated
        const int D_ENERGYBETA = 15; //!< Gamma singles beta gated
        const int DD_PROTONGAMMATDIFF_VS_GAMMAEN = 16; //!< GammaProton TDIFF vs. Gamma Energy
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void E14060Processor::DeclarePlots(void) {
    DeclareHistogram2D(DD_DEBUGGING0, SC, SD, "QDC CTof- No Tape Move");
    DeclareHistogram2D(DD_DEBUGGING1, SC, SD, "QDC ToF Ungated");
    DeclareHistogram2D(DD_DEBUGGING2, SC, SC, "Cor ToF vs. Gamma E");
    DeclareHistogram1D(DD_DEBUGGING3, S7, "Vandle Multiplicity");
    DeclareHistogram2D(DD_DEBUGGING4, SC, SC, "QDC vs Cor Tof Mult1");
    DeclareHistogram2D(DD_DEBUGGING5, SC, SC, "Mult2 Sym Plot Tof ");
    DeclareHistogram2D(DD_TACS, S3, SD, "Tacs");
    DeclareHistogram2D(DD_HAGRID, S3, SD, "HAGRiD");
    DeclareHistogram2D(DD_PROTONBETA2TDIFF_VS_BETA2EN, SD, SA,
                       "BetaProton Tdiff vs. Beta Energy");

    const int energyBins1 = SD;
    DeclareHistogram1D(D_ENERGY, energyBins1,
                       "Gamma singles ungated");
    DeclareHistogram1D(D_ENERGYBETA, energyBins1,
                       "Gamma singles beta gated");
    DeclareHistogram2D(DD_PROTONGAMMATDIFF_VS_GAMMAEN,
                       SD, SB, "GammaProton TDIFF vs. Gamma Energy");
}

E14060Processor::E14060Processor() : EventProcessor(OFFSET, RANGE,
                                                    "E14060PRocessor") {
    associatedTypes.insert("vandle");
    associatedTypes.insert("labr3");
    associatedTypes.insert("pspmt");
    associatedTypes.insert("ge");
    associatedTypes.insert("tac");

#ifdef useroot
    rootfile_ = new TFile("/tmp/test00.root","RECREATE");
    roottree_ = new TTree("vandle","");
    roottree_->Branch("tof",&tof_,"tof/D");
    roottree_->Branch("qdc",&qdc_,"qdc/D");
    qdctof_ = new TH2D("qdctof","",1000,-100,900,16000,0,16000);
    vsize_ = new TH1D("vsize","",40,0,40);
#endif
}

E14060Processor::~E14060Processor() {
#ifdef useroot
    rootfile_->Write();
    rootfile_->Close();
    delete(rootfile_);
#endif
}

///We do nothing here since we're completely dependent on the results of others
bool E14060Processor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);
    return (true);
}

bool E14060Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    double plotMult_ = 2;
    double plotOffset_ = 1000;

    BarMap vbars;
    vector < ChanEvent * > geEvts;
    vector <vector<AddBackEvent>> geAddback;

    if (event.GetSummary("vandle")->GetList().size() != 0)
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();
    if (event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetGeEvents();
        geAddback = ((GeProcessor *) DetectorDriver::get()->
                GetProcessor("GeProcessor"))->GetAddbackEvents();
    }

    static const vector<ChanEvent *> &hagridEvts =
            event.GetSummary("hagrid")->GetList();

    static const vector<ChanEvent *> &tacEvts =
            event.GetSummary("tac")->GetList();

#ifdef useroot
    vsize_->Fill(vbars.size());
#endif
    plot(DD_DEBUGGING3, vbars.size());

    //Begin processing for VANDLE bars
    for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if (!bar.GetHasEvent() || bar.GetType() == "small")
            continue;

        TimingCalibration cal = bar.GetCalibration();

    } //(BarMap::iterator itBar
    //End processing for VANDLE bars

    //-------------- LaBr3 Processing ---------------
    for (vector<ChanEvent *>::const_iterator it = hagridEvts.begin();
         it != hagridEvts.end(); it++)
        plot(DD_HAGRID, (*it)->GetEnergy(), (*it)->GetChanID().GetLocation());

    //-------------- TAC Processing ---------------
    for (vector<ChanEvent *>::const_iterator it = tacEvts.begin();
         it != tacEvts.end(); it++)
        plot(DD_TACS, (*it)->GetEnergy()), (*it)->GetChanID().GetLocation();

    EndProcess();
    return (true);
}
