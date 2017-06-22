/** \file Ornl2016Processor.cpp
 * \brief A class to process data from the Ornl 2016 OLTF experiment using
 * VANDLE. Using Root and Damm for histogram analysis. 
 * Moved to PAASS Oct 2016
 *
 *
 *\author S. V. Paulauskas
 *\date February 10, 2016
 *
 *\Edits by Thomas King 
 *\Starting June 2016
 *
 *
*/
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"

#include "DoubleBetaProcessor.hpp"
#include "GeProcessor.hpp"
#include "Ornl2016Processor.hpp"
#include "VandleProcessor.hpp"

static unsigned int evtNum = 0;

namespace dammIds {
    namespace experiment {
        const unsigned int Ornl2016_OFFSET = 0;
        const int D_VANDLEMULT = 0 + Ornl2016_OFFSET;

        const int DD_CTOFNOTAPE = 1 + Ornl2016_OFFSET;
        const int DD_QDCTOFNOGATE = 2 + Ornl2016_OFFSET;
        const int DD_CORTOFVSEGAM = 3 + Ornl2016_OFFSET;
        const int DD_QDCVSCORTOFMULT1 = 4 + Ornl2016_OFFSET;
        const int DD_MULT2SYM = 5 + Ornl2016_OFFSET;
        const int DD_LIGLEN = 6 + Ornl2016_OFFSET;
        const int DD_LIGLTOF = 7 + Ornl2016_OFFSET;

        const int D_LABR3SUM = 8 + Ornl2016_OFFSET;
        const int D_LABR3BETA = 9 + Ornl2016_OFFSET;

        const int D_HPGESUM = 10 + Ornl2016_OFFSET;
        const int D_HPGESUMBETA = 11 + Ornl2016_OFFSET;

        const int D_NAISUM = 12 + Ornl2016_OFFSET;
        const int D_NAIBETA = 13 + Ornl2016_OFFSET;

        const int DD_TOFVSNAI = 14 + Ornl2016_OFFSET;
        const int DD_TOFVSHAGRID = 15 + Ornl2016_OFFSET;
        const int DD_TOFVSGE = 16 + Ornl2016_OFFSET;

        //seeds for Damm cycle his
        const int D_BETASCALARRATE = 29 + Ornl2016_OFFSET; //6079 in his
        const int D_BETAENERGY = 30 + Ornl2016_OFFSET;
        const int D_IGEBETA = 31 + Ornl2016_OFFSET;
        const int D_INAIBETA = 35 + Ornl2016_OFFSET;
        const int D_IHAGBETA = 45 + Ornl2016_OFFSET;
        const int DD_QDCVTOF = 48 + Ornl2016_OFFSET;
        const int DD_TRACES = 49 + Ornl2016_OFFSET;
        const int DD_FLASHTRACES = 50 + Ornl2016_OFFSET;
        const int D_BADLOCATION = 51 + Ornl2016_OFFSET;
        const int D_BADCYCLE = 52 + Ornl2016_OFFSET;


        const int DD_QDCVSTOFNOF = 53 + Ornl2016_OFFSET;
        const int DD_SIGNOIS = 54 + Ornl2016_OFFSET;
        const int DD_GSIGNOIS = 55 + Ornl2016_OFFSET;
        const int DD_ETRIGVSQDC =56 + Ornl2016_OFFSET;
        const int DD_GETRIGVSQDC = 57+Ornl2016_OFFSET;

        const int D_GCYCLE = 58 +Ornl2016_OFFSET;
        const int D_STARTLOC = 59+Ornl2016_OFFSET;

        const int DD_BADCYCLELOC = 60+Ornl2016_OFFSET;
        const int DD_GOODCYCLELOC =61+Ornl2016_OFFSET;
        const int DD_QDCVTOFNOMOD2 = 62 +Ornl2016_OFFSET;
        const int D_MOD2CHECK = 63 + Ornl2016_OFFSET;



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

    DeclareHistogram1D(D_HPGESUM, SC, "HPGe Clover summed");
    DeclareHistogram1D(D_HPGESUMBETA, SC, "HPGe Clover summed - BETA GATED");

    DeclareHistogram2D(DD_TOFVSNAI, SC, SB, "ToF vs. NaI");
    DeclareHistogram2D(DD_TOFVSHAGRID, SC, SB, "ToF vs. HAGRiD");
    DeclareHistogram2D(DD_TOFVSGE, SC, SB, "ToF vs. Ge");

    DeclareHistogram1D(D_BETASCALARRATE, SB, "Beta scalar per cycle");
    DeclareHistogram1D(D_BETAENERGY, SD, "Beta Energy");

    DeclareHistogram2D(DD_QDCVTOF, SC, SD, "exp processor made qdc vs tof");
    DeclareHistogram2D(DD_TRACES, S8, SE, "traces");
    DeclareHistogram2D(DD_FLASHTRACES, S8, SE, "traces gated +/- 15 of 2nd flash center");
    DeclareHistogram1D(D_BADLOCATION, S6, "'Bad' trace vs bar location");
    DeclareHistogram1D(D_BADCYCLE, SC, "# of 'Bad' traces per cycle");
    DeclareHistogram2D(DD_QDCVSTOFNOF, SC, SD, "QDC vs TOF after separation ");
    DeclareHistogram2D(DD_SIGNOIS, SD, SC, "signal to noise vs QDC of bad traces");
    DeclareHistogram2D(DD_GSIGNOIS, SD, SC, "signal to noise vs QDC of good traces");
    DeclareHistogram2D(DD_ETRIGVSQDC, SD,SC,"energy of trigger vs qdc for BAD");
    DeclareHistogram2D(DD_GETRIGVSQDC, SD,SC,"energy of trigger vs qdc for Good");
    DeclareHistogram1D(D_GCYCLE,SC,"#of traces in gamma flash");
    DeclareHistogram1D(D_STARTLOC,SB,"Detector Referenced as Start for vandle (First bunch is bad second is good)");
    DeclareHistogram2D(DD_BADCYCLELOC,SC,S6," # of bad traces per cycle in each bar");
    DeclareHistogram2D(DD_GOODCYCLELOC,SC,S6," # of good traces per cycle in each bar");
    DeclareHistogram2D(DD_QDCVTOFNOMOD2,SC,SD,"QDC vs TOF without module 2 (second master)");
    DeclareHistogram1D(D_MOD2CHECK,S6,"# of traces in bars (without mod2)");



    // //Declaring beta gated
    // for (unsigned int i=0; i < 4; i++){
    //   stringstream ss;
    //   ss<< "HPGe " << i <<" - beta gated " ;
    //   DeclareHistogram1D(D_IGEBETA + i,SC,ss.str().c_str());
    // }

    // //Declaring NaI beta gated
    // for (unsigned int i=0; i < 10; i++){
    //   stringstream ss;
    //   ss<< "NaI " << i << " - beta gated ";
    //   DeclareHistogram1D(D_INAIBETA + i ,SC,ss.str().c_str());
    // }

    // //Declaring Hagrid beta gated
    // for (unsigned int i = 0; i < 16; i++){
    //   stringstream ss;
    //   ss<< "LaBr " << i << " - beta gated ";
    //   DeclareHistogram1D(D_IHAGBETA + i ,SC,ss.str().c_str());

    // }

}


void Ornl2016Processor::rootGstrutInit(RAY &strutName) { //Zeros the entire aux  structure

    fill(strutName.LaBr, strutName.LaBr + 16, 0);
    fill(strutName.NaI, strutName.NaI + 10, 0);
    fill(strutName.Ge, strutName.Ge + 4, 0);
    strutName.beta = -9999;
    strutName.cycle = -9999;
    strutName.eventNum = -9999;
    strutName.gMulti = -9999;
    strutName.nMulti = -9999;
    strutName.lMulti = -9999;
    strutName.bMulti = -9999;

}

void Ornl2016Processor::rootGstrutInit2(PROSS &strutName) { //Zeros the entire processed structure
    strutName.AbE = -999;
    strutName.AbEvtNum = -9999;
    strutName.Multi = 0;
    //strutName.SymX = -999;
    //strutName.SymY = -999;
}

void Ornl2016Processor::rootNstrutInit(NBAR &strutName) { //Zeros the entire VANDLE structure
    fill(strutName.LaBr, strutName.LaBr + 16, 0);
    fill(strutName.NaI, strutName.NaI + 10, 0);
    fill(strutName.Ge, strutName.Ge + 4, 0);
    strutName.tof = -999;
    strutName.qdc = -999;
    strutName.betaEn = -999;
    strutName.snrl = -999;
    strutName.snrr = -999;
    strutName.Qpos = -999;
    strutName.tDiff = -999;
    strutName.barid = -999;
}

/*
void Ornl2016Processor::rootBWaveInit(BWave &strutName) { //Zeros the WaveForm Structures
    fill(strutName.Ltrace, strutName.Ltrace + 131, 0);
    fill(strutName.Rtrace, strutName.Rtrace + 131, 0);
    strutName.Lbaseline = -999;
    strutName.Rbaseline = -999;
    strutName.LmaxLoc = -999;
    strutName.RmaxLoc = -999;
    strutName.Lamp = -999;
    strutName.Ramp = -999;
    strutName.BarQdc = -999;
    strutName.Rsnr = -999;
    strutName.Lsnr = -999;
    strutName.Lqdc =-999;
    strutName.Rqdc = -999;
    strutName.Tdiff = -99999;
    strutName.Rphase = -999999;
    strutName.Lphase = -999999;
    }
*/

void Ornl2016Processor::rootVWaveInit(VWave &strutName) { //Zeros the WaveForm Structures
    fill(strutName.Ltrace, strutName.Ltrace + 131, 0);
    fill(strutName.Rtrace, strutName.Rtrace + 131, 0);
    strutName.Lbaseline = -999;
    strutName.Rbaseline = -999;
    strutName.LmaxLoc = -999;
    strutName.RmaxLoc = -999;
    strutName.Lamp = -999;
    strutName.Ramp = -999;
    strutName.BarQdc = -999;
    strutName.TOF = -999;
    strutName.Lsnr = -999;
    strutName.Rsnr = -999;
    strutName.VbarNum = -1;
}


Ornl2016Processor::Ornl2016Processor(double gamma_threshold_L, double sub_event_L, double gamma_threshold_N,
                                     double sub_event_N, double gamma_threshold_G, double sub_event_G) : EventProcessor(
        OFFSET, RANGE, "Ornl2016Processor") {

    associatedTypes.insert("vandle");
    associatedTypes.insert("ge");
    associatedTypes.insert("nai");
    associatedTypes.insert("labr3");
    associatedTypes.insert("beta");

    LgammaThreshold_ = gamma_threshold_L;
    LsubEventWindow_ = sub_event_L;
    NgammaThreshold_ = gamma_threshold_N;
    NsubEventWindow_ = sub_event_N;
    GgammaThreshold_ = gamma_threshold_G;
    GsubEventWindow_ = sub_event_G;

    // initalize addback vectors
    LaddBack_.push_back(ScintAddBack(0, 0, 0));
    NaddBack_.push_back(ScintAddBack(0, 0, 0));
    GaddBack_.push_back(ScintAddBack(0, 0, 0));

    // ROOT file Naming
    string hisPath = Globals::get()->GetOutputPath();
    string hisfilename = hisPath + Globals::get()->GetOutputFileName();
    string rootname = hisfilename + ".root";
    string rootname2 = hisfilename +"-hiso.root";
    string rootname3 = hisfilename + "-Owave.root";

    // Start Primary Root File
    rootFName_ = new TFile(rootname.c_str(), "RECREATE");
    Taux = new TTree("Taux", "Tree for Gamma-ray stuff @ ORNL2016");
    Tvan = new TTree("Tvan","Tree for Vandle Stuff (coincident gammas as well) @ ORNL2016");

    //Taux Stuff
    singBranch = Taux->Branch("sing", &sing,
                              "LaBr[16]/D:NaI[10]/D:Ge[4]/D:beta/D:eventNum/D:cycle/i:gMulti/i:nMulti/i:hMulti/i:bMulti/i");
    gProcBranch = Taux->Branch("Gpro", &Gpro, "AbE/D:AbEvtNum/D:Multi/D");
    lProcBranch = Taux->Branch("Lpro", &Lpro, "AbE/D:AbEvtNum/D:Multi/D");
    nProcBranch = Taux->Branch("Npro", &Npro, "AbE/D:AbEvtNum/D:Multi/D");

    Taux->SetAutoFlush(3000);
    rootGstrutInit(sing);
    rootGstrutInit2(Gpro);
    rootGstrutInit2(Lpro);
    rootGstrutInit2(Npro);

    // Tvan Stuff
    mVanBranch = Tvan->Branch("mVan", &mVan,
                              "LaBr[16]/D:NaI[10]/D:Ge[4]/D:tof/D:qdc/D:betaEn/D:snrl/D:snrr/D:Qpos/D:tDiff/D:barid/i");
    Tvan->SetAutoFlush(3000);
    rootNstrutInit(mVan);

    // End Primary Root File

    // Start Secondary (Histo) RootFile
    rootFName2_= new TFile(rootname2.c_str(),"RECREATE");
    qdcVtof_ = new TH2D("qdcVtof","",1000,-100,900,32000,-16000,16000);
    tofVGe_ = new  TH2D("tofVGe","",1500,-100,1400,16000,0,16000);
    tofVLabr_ = new  TH2D("tofVLaBr","",1500,-100,1400,16000,0,16000);
    tofVNai_ = new  TH2D("tofVNaI","",1500,-100,1400,16000,0,16000);

    // End Secondary (Histo) RootFile



    // Start Tertiary (Waveform) RootFille
    rootFName3_ = new TFile(rootname3.c_str(),"RECREATE");
    Wave = new TTree("Wave","Tree for Waveform Analyzer Debugging");
    VwaveBranch = Wave->Branch("Vwave",&Vwave,"Ltrace[131]/D:Rtrace[131]:Lbaseline:Rbaseline:LmaxLoc:RmaxLoc:Lamp:Ramp:BarQdc:TOF:Lsnr:Rsnr:VbarNum/I");
    BwaveBranch = Wave->Branch("Bwave",&Bwave,"Ltrace[131]/D:Rtrace[131]:Lbaseline:Rbaseline:LmaxLoc:RmaxLoc:Lamp:Ramp:BarQdc:Lsnr:Rsnr:Lqdc:Rqdc:Tdiff:Lphase:Rphase");

 //   rootBWaveInit(Bwave);
    rootVWaveInit(Vwave);

    Wave->SetAutoFlush(3000);
    // Start Tertiary (Waveform) RootFille
}


Ornl2016Processor::~Ornl2016Processor() {

    rootFName_->Write();
    rootFName_->Close();

    rootFName2_->Write();
    rootFName2_->Close();

//    rootFName3_->Write();
//    rootFName3_->Close();

    delete (rootFName_);
    delete (rootFName2_);
   // delete (rootFName3_);


}


bool Ornl2016Processor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);


    EndProcess();
    return (true);
}

bool Ornl2016Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    double plotOffset_ = 1000;
    double plotMult_ = 2;

    BarMap vbars, betas;
    bool hasBeta = false;
    map<unsigned int, pair<double, double> > lrtBetas;
    hasBeta = TreeCorrelator::get()->place(
            "Beta")->status(); //might need a static initialize to false + reset at the end

    /*    if (event.GetSummary("vandle")->GetList().size() != 0) {
        vbars = ((VandleProcessor *) DetectorDriver::get()->
                GetProcessor("VandleProcessor"))->GetBars();
    }
    */    
    if (event.GetSummary("beta:double")->GetList().size() != 0) {
        betas = ((DoubleBetaProcessor *) DetectorDriver::get()->
                GetProcessor("DoubleBetaProcessor"))->GetBars();

        if (event.GetSummary("beta:double")->GetList().size() != 0) {
            lrtBetas = ((DoubleBetaProcessor *) DetectorDriver::get()->
                    GetProcessor("DoubleBetaProcessor"))->GetLowResBars();
        }
    }
    static const vector<ChanEvent *> &labr3Evts =
            event.GetSummary("labr3")->GetList();
    static const vector<ChanEvent *> &naiEvts =
            event.GetSummary("nai")->GetList();
    static const vector<ChanEvent *> &geEvts =
            event.GetSummary("ge")->GetList();


    /// PLOT ANALYSIS HISTOGRAMS-------------------------------------------------------------------------------------------------------------------------------------

    rootGstrutInit(sing); // initalize the root structures
    rootGstrutInit2(Gpro);
    rootGstrutInit2(Lpro);
    rootGstrutInit2(Npro);
    rootNstrutInit(mVan);

    //Setting vars for addback
    double LrefTime = -2.0 * LsubEventWindow_;
    double NrefTime = -2.0 * NsubEventWindow_;
    double GrefTime = -2.0 * GsubEventWindow_;




    //Cycle timing
    static double cycleLast = 2;
    static int cycleNum = 0;

    if (TreeCorrelator::get()->place("Cycle")->status()) {
        double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
        cycleTime *= Globals::get()->GetClockInSeconds() * 1.e9;
        if (cycleTime != cycleLast) {
            double tdiff = (cycleTime - cycleLast) / 1e6; //Outputs cycle length in msecs.
            if (cycleNum == 0) {
                cout
                        << " #  There are some events at the beginning of the first segment missing from Histograms that use cycleNum."
                        << endl << " #  This is a product of not starting the cycle After the LDF." << endl
                        << " #  This First TDIFF is most likely nonsense" << endl;
            }
            cycleLast = cycleTime;
            cycleNum++;
            cout << "Cycle Change " << endl << "Tdiff (Cycle start and Now) (ms)= " << tdiff << endl
                 << "Starting on Cycle #" << cycleNum << endl;
        }
    }
    sing.cycle = cycleNum;

    //set multiplicys for sing branch based on the size of the detector maps for the event. limitation: sub event is smaller than full event this will end up being too large
    sing.gMulti = geEvts.size();
    sing.nMulti = naiEvts.size();
    sing.lMulti = labr3Evts.size();
    sing.bMulti = lrtBetas.size();


    for (map<unsigned int, pair<double, double> >::iterator bIt = lrtBetas.begin(); bIt != lrtBetas.end(); bIt++) {

    plot(D_BETASCALARRATE, cycleNum);//PLOTTING BETA SCALAR SUM per CYCLE (LIKE 759 but per cycle vs per second
    plot(D_BETAENERGY, bIt->second.second);
    sing.beta = bIt->second.second;
    }


    //NaI ONLY----------------------------------------------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itNai = naiEvts.begin();
         itNai != naiEvts.end(); itNai++) {
        int naiNum = (*itNai)->GetChannelNumber();
        sing.NaI[naiNum] = (*itNai)->GetCalibratedEnergy();
        plot(D_NAISUM, (*itNai)->GetCalibratedEnergy()); //plot totals

        if (hasBeta) {  //Beta Gate
            plot(D_NAIBETA, (*itNai)->GetCalibratedEnergy()); //plot beta-gated totals

            //begin addback calulations for NaI
            double energy = (*itNai)->GetCalibratedEnergy();
            double time = (*itNai)->GetTime();

            if (energy < NgammaThreshold_) {
                continue;
            }//end energy comp if statment
            double t1 = Globals::get()->GetClockInSeconds();
            double dtime = abs(time - NrefTime) * t1;

            if (dtime >
                NsubEventWindow_) { //if event time is outside sub event window start new addback after filling tree
                Npro.AbEvtNum = evtNum;
                Npro.AbE = NaddBack_.back().energy;
                Npro.Multi = NaddBack_.back().multiplicity;
                Taux->Fill();
                NaddBack_.push_back(ScintAddBack());
            }//end subEvent IF
            NaddBack_.back().energy += energy; // if still inside sub window: incrament
            NaddBack_.back().time = time;
            NaddBack_.back().multiplicity += 1;
            NrefTime = time;

        }//end beta gate
    } //NaI loop End

    //HPGe ONLY---------------------------------------------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
         itGe != geEvts.end(); itGe++) {
        int geNum = (*itGe)->GetChanID().GetLocation();
        sing.Ge[geNum] = (*itGe)->GetCalibratedEnergy();
        plot(D_HPGESUM, (*itGe)->GetCalibratedEnergy()); //plot non-gated totals

        if (hasBeta) { //beta-gated Processing to cut LaBr contamination out
            plot(D_HPGESUMBETA, (*itGe)->GetCalibratedEnergy()); //plot non-gated totals
            //begin addback calulations for clover
            double energy = (*itGe)->GetCalibratedEnergy();
            double time = (*itGe)->GetTime();
            if (energy < GgammaThreshold_) {
                continue;
            }//end energy comp if statment
            double t1 = Globals::get()->GetClockInSeconds();
            double dtime = abs(time - GrefTime) * t1;

            if (dtime >
                GsubEventWindow_) { //if event time is outside sub event window start new addback after filling tree
                Gpro.AbEvtNum = evtNum;
                Gpro.Multi = GaddBack_.back().multiplicity;
                Gpro.AbE = GaddBack_.back().energy;
                Taux->Fill();
                GaddBack_.push_back(ScintAddBack());
            } //end subEvent IF

            GaddBack_.back().energy += energy;
            GaddBack_.back().time = time;
            GaddBack_.back().multiplicity += 1;
            GrefTime = time;


        } //end BetaGate
    } //GE loop end

    //HAGRiD ONLY-------------------------------------------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itLabr = labr3Evts.begin();
         itLabr != labr3Evts.end(); itLabr++) {
        int labrNum = (*itLabr)->GetChanID().GetLocation();
        plot(D_LABR3SUM, (*itLabr)->GetCalibratedEnergy()); //plot non-gated totals

        if (hasBeta) {

            plot(D_LABR3BETA, (*itLabr)->GetCalibratedEnergy()); //plot beta-gated totals
            //begin addback calculations for LaBr | Beta Gated to Remove La Contamination

            double energy = (*itLabr)->GetCalibratedEnergy();
            double time = (*itLabr)->GetTime();

            if (energy < LgammaThreshold_) {
                continue;
            }//end energy comp if statment

            double t1 = Globals::get()->GetClockInSeconds();
            double dtime = abs(time - LrefTime) * t1;

            if (dtime >
                LsubEventWindow_) { //if event time is outside sub event window start new addback after filling tree

                Lpro.AbEvtNum = evtNum;
                Lpro.AbE = LaddBack_.back().energy;
                Lpro.Multi = LaddBack_.back().multiplicity;
                Taux->Fill();
                LaddBack_.push_back(ScintAddBack());
            }// end if for new entry in vector

            LaddBack_.back().energy += energy;
            LaddBack_.back().time = time;
            LaddBack_.back().multiplicity += 1;
            LrefTime = time;


            // Left in for referance but not usefull without more than 1 detc per type (like 2 clovers)
//                for (vector<ChanEvent *>::const_iterator itLabr2 = itLabr + 1;
//                     itLabr2 != labr3Evts.end(); itLabr2++) {
//                    double energy2 = (*itLabr2)->GetCalEnergy();
//                    int labrNum2 = (*itLabr2)->GetChanID().GetLocation();
//                    //double time2=(*itGe2)->GetCorrectedTime();
//                    if (energy2 < LgammaThreshold_) {
//                        continue;
//                    }//end energy comp if statement
//                    if (labrNum2 != labrNum) {
//                        Lpro.SymX = energy;
//                        Lpro.SymY = energy2;
//                        Taux->Fill();
//                    }
//
//
//                } //end symplot inner loop

        }//end beta gate

        sing.LaBr[labrNum] = (*itLabr)->GetCalibratedEnergy();
    } //Hagrid loop end

     //Begin VANDLE
     for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
         TimingDefs::TimingIdentifier barId = (*it).first;
         BarDetector bar = (*it).second;


         if (!bar.GetHasEvent() || bar.GetType() == "small")
             continue;

         int barLoc = barId.first;
         TimingCalibration cal = bar.GetCalibration();

         for (BarMap::iterator itStart = betas.begin();
              itStart != betas.end(); itStart++) {
             unsigned int startLoc = (*itStart).first.first;
             BarDetector start = (*itStart).second;
             if (!start.GetHasEvent())
                 continue;






             double tof = bar.GetTimeAverage() -
                          start.GetTimeAverage() + cal.GetTofOffset(startLoc);

             double corTof = ((VandleProcessor *) DetectorDriver::get()->
                     GetProcessor("VandleProcessor"))->
                     CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

             mVan.qdc = bar.GetQdc();
             mVan.Qpos = bar.GetQdcPosition();
             mVan.tDiff = bar.GetTimeDifference();
             mVan.tof = tof;
             mVan.cortof = corTof;
             mVan.barid = barLoc;
             mVan.snrl = bar.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
             mVan.snrr = bar.GetRightSide().GetTrace().GetSignalToNoiseRatio();
             mVan.betaEn = start.GetQdc();
             plot(DD_QDCVTOF, (tof * 2) + plotOffset_, bar.GetQdc());

             qdcVtof_->Fill(tof,bar.GetQdc());

             Vwave.VbarNum=barLoc;
             Vwave.TOF=tof;
             Vwave.BarQdc=bar.GetQdc();
             Vwave.Lbaseline=bar.GetLeftSide().GetAveBaseline();
             Vwave.Rbaseline=bar.GetRightSide().GetAveBaseline();
             Vwave.RmaxLoc=bar.GetRightSide().GetMaximumPosition();
             Vwave.LmaxLoc=bar.GetLeftSide().GetMaximumPosition();
             Vwave.Ramp=bar.GetRightSide().GetMaximumValue();
             Vwave.Lamp=bar.GetLeftSide().GetMaximumValue();
             Vwave.Lsnr=bar.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
             Vwave.Rsnr=bar.GetRightSide().GetTrace().GetSignalToNoiseRatio();
             //bar.GetLeftSide().()


 /*
             Bwave.Lbaseline=start.GetLeftSide().GetAveBaseline();
             Bwave.Rbaseline=start.GetRightSide().GetAveBaseline();
             Bwave.BarQdc= start.GetQdc();
             Bwave.LmaxLoc=start.GetLeftSide().GetMaximumPosition();
             Bwave.RmaxLoc=start.GetRightSide().GetMaximumPosition();
             Bwave.Lamp=start.GetLeftSide().GetMaximumValue();
             Bwave.Ramp=start.GetRightSide().GetMaximumValue();
             Bwave.Lsnr=start.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
             Bwave.Rsnr=start.GetRightSide().GetTrace().GetSignalToNoiseRatio();
             Bwave.Lqdc=start.GetLeftSide().GetTraceQdc();
             Bwave.Rqdc=start.GetRightSide().GetTraceQdc();
             Bwave.Lphase = start.GetLeftSide().GetP;
             Bwave.Rphase = start.GetRightSide()

 */


             int itTVl=0;
             for (vector<unsigned int>::const_iterator itTL = bar.GetLeftSide().GetTrace().begin();
                  itTL != bar.GetLeftSide().GetTrace().end(); itTL++) {
                 Vwave.Ltrace[itTVl]=(*itTL);
                 itTVl++;
             };

             int itTVr=0;
             for (vector<unsigned int>::const_iterator itTR = bar.GetRightSide().GetTrace().begin();
                  itTR != bar.GetRightSide().GetTrace().end(); itTR++) {
                 Vwave.Rtrace[itTVr]=(*itTR);
                 itTVr++;
             };

             int itTBl=0;
             for (vector<unsigned int>::const_iterator itTL = start.GetLeftSide().GetTrace().begin();
                  itTL != start.GetLeftSide().GetTrace().end(); itTL++) {
                 Bwave.Ltrace[itTBl]=(*itTL);
                 itTBl++;
             };

             int itTr=0;
             for (vector<unsigned int>::const_iterator itTR = start.GetRightSide().GetTrace().begin();
                  itTR != start.GetRightSide().GetTrace().end(); itTR++) {
                 Bwave.Rtrace[itTr]=(*itTR);
                 itTr++;
             };

             Wave->Fill();

             //this is ghost flash troubleshooting code
 /*

             if (barLoc <8 || barLoc > 15){
                 plot(DD_QDCVTOFNOMOD2,(tof * 2) + plotOffset_, bar.GetQdc());
                 plot(D_MOD2CHECK,barLoc);
             };


             static int trcCounter = 0;
             static int ftrcCounter = 0;
             double dammBin = (tof * 2) + 1000;
             static int badTrcEvtCounter = 0;
             if (dammBin >= 1048 && dammBin <= 1078) {
                 for (vector<unsigned int>::const_iterator itTL = bar.GetLeftSide().GetTrace()->begin();
                      itTL != bar.GetLeftSide().GetTrace()->end(); itTL++) {
                     plot(DD_FLASHTRACES, itTL - bar.GetLeftSide().GetTrace()->begin(), ftrcCounter, (*itTL));
                 }
                 for (vector<unsigned int>::const_iterator itTR = bar.GetRightSide().GetTrace()->begin();
                      itTR != bar.GetRightSide().GetTrace()->end(); itTR++) {
                     plot(DD_FLASHTRACES, itTR - bar.GetRightSide().GetTrace()->begin(),
                          ftrcCounter + 1, (*itTR));
                 }
                 ftrcCounter += 3;

                 plot(D_BADLOCATION, barLoc);
                 plot(D_STARTLOC,startLoc);
                 if (TreeCorrelator::get()->place("Cycle")->status()) {
                     double inCycleTime = bar.GetTimeAverage();
                     double cycleTimeLast = TreeCorrelator::get()->place("Cycle")->last().time;
                     cycleTimeLast *= (Globals::get()->clockInSeconds() * 1.e9);
                     double currenttime = (inCycleTime) - (cycleTimeLast);


                     //cout << endl << "timeavg=" << inCycleTime << endl << "currenttime=" << currenttime << endl
                     //     << "last cycle start=" << cycleTimeLast << endl;

                     plot(DD_SIGNOIS, bar.GetLeftSide().GetSignalToNoiseRatio(),bar.GetQdc() );
                     plot(DD_ETRIGVSQDC,start.GetQdc(),bar.GetQdc());
                     plot(D_BADCYCLE, cycleNum);
                     plot(DD_BADCYCLELOC,cycleNum,barLoc);
                 }
             }
             else if (dammBin >=995 && dammBin<=1015){
                 plot(D_GCYCLE,cycleNum);
             }
             else {

                 plot(D_STARTLOC,startLoc+1000);

                 for (vector<unsigned int>::const_iterator itTL = bar.GetLeftSide().GetTrace()->begin();
                      itTL != bar.GetLeftSide().GetTrace()->end(); itTL++) {
                     plot(DD_TRACES, itTL - bar.GetLeftSide().GetTrace()->begin(),
                          trcCounter, (*itTL));
                 }

                 for (vector<unsigned int>::const_iterator itTR = bar.GetRightSide().GetTrace()->begin();
                      itTR != bar.GetRightSide().GetTrace()->end(); itTR++) {
                     plot(DD_TRACES, itTR - bar.GetRightSide().GetTrace()->begin(),
                          trcCounter + 1, (*itTR));
                 }

                 trcCounter += 3;
                 plot(DD_QDCVSTOFNOF, (tof * 2) + 1000, bar.GetQdc());
                 plot(DD_GSIGNOIS, bar.GetLeftSide().GetSignalToNoiseRatio(),bar.GetQdc() );
                 plot(DD_GETRIGVSQDC,start.GetQdc(),bar.GetQdc());
             }


             */

             //tof vs gammas in damm for testing against root when its working right
             //Gamma Loops for VANDLE
             //labr loop for mVan
             int labrNum;
             double labrEn;
             for (vector<ChanEvent *>::const_iterator itlabr3 = labr3Evts.begin();
                  itlabr3 != labr3Evts.end(); itlabr3++) {
                 labrNum = (*itlabr3)->GetChanID().GetLocation();
                 labrEn = (*itlabr3)->GetCalibratedEnergy();
                 plot(DD_TOFVSHAGRID, labrEn, tof * plotMult_ + 200);

                 tofVLabr_->Fill(labrEn,tof);

                 mVan.LaBr[labrNum] = labrEn;
             };

             //Nai loop for mVan
             int naiNum;
             double naiEn;
             for (vector<ChanEvent *>::const_iterator itNai = naiEvts.begin();
                  itNai != naiEvts.end(); itNai++) {
                 naiNum = (*itNai)->GetChanID().GetLocation();
                 naiEn = (*itNai)->GetCalibratedEnergy();
                 plot(DD_TOFVSNAI, naiEn, tof * plotMult_ + 200);

                 tofVNai_->Fill(naiEn,tof);

                 mVan.NaI[naiNum] = naiEn;
             };

             //ge loop for mVan
             int geNum;
             double geEn;
             for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
                  itGe != geEvts.end(); itGe++) {
                 geNum = (*itGe)->GetChanID().GetLocation();
                 geEn = (*itGe)->GetCalibratedEnergy();
                 plot(DD_TOFVSGE, geEn, tof * plotMult_ + 200);

                 tofVGe_->Fill(geEn,tof);

                 mVan.Ge[geNum] = geEn;
             };
         };

         Wave->Fill();
     };//End VANDLE

    sing.eventNum = evtNum;

    Taux->Fill();
    Tvan->Fill();


    evtNum++;
    EndProcess();
    return (true);
}
