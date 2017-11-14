/** \file Ornl2016Processor.cpp
 * \brief A class to process data from the Ornl 2016 OLTF experiment using
 * VANDLE. Using Root and Damm for histogram analysis. 
 * Moved to PAASS Oct 2016
 * Writen originally by S. V. Paulauskas on February 10, 2016
 * Significant edits were made in the Summer of 2017
 *
 *\author Thomas King
 *\date February 10, 2016
 *
 *
 *
*/
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <vector>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "StringManipulationFunctions.hpp"

#include "DoubleBetaProcessor.hpp"
#include "GeProcessor.hpp"
#include "Ornl2016Processor.hpp"
#include "VandleProcessor.hpp"

static unsigned int evtNum = 0;

namespace dammIds {
    namespace experiment {
        const int D_VANDLEMULT = 0;
        const int DD_CTOFNOTAPE = 1;
        const int DD_QDCTOFNOGATE = 2;
        const int DD_CORTOFVSEGAM = 3;
        const int DD_QDCVSCORTOFMULT1 = 4;
        const int DD_MULT2SYM = 5;
        const int DD_LIGLEN = 6;
        const int DD_LIGLTOF = 7;

        const int D_LABR3SUM = 8;
        const int D_LABR3BETA = 9;
        const int D_HPGESUM = 10;
        const int D_HPGESUMBETA = 11;
        const int D_NAISUM = 12;
        const int D_NAIBETA = 13;

        const int DD_TOFVSNAI = 14;
        const int DD_TOFVSHAGRID = 15;
        const int DD_TOFVSGE = 16;

        const int DD_DETcheckNai= 17;
        const int DD_DETcheckHag= 18;
        const int DD_DETcheckGe= 19;

        const int D_BETASCALARRATE = 29; //6079 in his
        const int D_BETAENERGY = 30;
        const int DD_QDCVTOF = 31;

        const int D_DBGge = 35 ;//super beta gated specta;
        const int D_DBGnai = 36 ;//super beta gated specta;
        const int D_DBGlabr = 37 ;//super beta gated specta;
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

    std::string DBGeName = "Super-BGated HPGe Energy ( Window="+SupBetaWin.second+" ns)";
    std::string DBNaiName = "Super-BGated NaI Energy (Window= "+SupBetaWin.second+" ns)";
    std::string DBLaBrName = "Super-BGated LaBr Energy ( Window="+SupBetaWin.second+" ns)";

    DeclareHistogram1D(D_DBGge, SD, DBGeName.c_str());
    DeclareHistogram1D(D_DBGnai, SD, DBNaiName.c_str());
    DeclareHistogram1D(D_DBGlabr, SD, DBLaBrName.c_str());


    DeclareHistogram2D(DD_DETcheckGe, SD, S5, "Detector Ignore Check (Ge)");
    DeclareHistogram2D(DD_DETcheckHag, SD, S5, "Detector Ignore Check (Hag)");
    DeclareHistogram2D(DD_DETcheckNai, SD, S5, "Detector Ignore Check (Nai)");


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


/*void Ornl2016Processor::rootGstrutInit(RAY &strutName) { //Zeros the entire aux  structure

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
*/
void Ornl2016Processor::rootGstrutInit2(PROSS &strutName) { //Zeros the entire processed structure
    strutName.AbE = -999;
    strutName.AbEvtNum = -9999;
    strutName.Multi = 0;
    //strutName.SymX = -999;
    //strutName.SymY = -999;
}

void Ornl2016Processor::rootAuxRoot(string &type) { //Zeros the entire processed structure
    if (type == "LaBr"){
        aux_LaBrEn=0;
        aux_LaBrNum=0;
        aux_LaBrTime=0;
        aux_LaBrHasLRBeta=false;
        aux_LaBrTdiff = 0;
        aux_LaBrMulti=0;
        aux_BetaEn=0;
        aux_BetaTime=0;
        aux_BetaMulti=0;

    } else if (type == "NaI"){
        aux_NaIEn=0;
        aux_NaINum=0;
        aux_NaITime=0;
        aux_NaIHasLRBeta=false;
        aux_NaITdiff = 0;
        aux_NaIMulti=0;
        aux_BetaEn=0;
        aux_BetaTime=0;
        aux_BetaMulti=0;

    } else if (type == "Ge"){
        aux_GeEn=0;
        aux_GeNum=0;
        aux_GeTime=0;
        aux_GeHasLRBeta=false;
        aux_GeTdiff = 0;
        aux_GeMulti=0;
        aux_BetaEn=0;
        aux_BetaTime=0;
        aux_BetaMulti=0;

    }

}

Ornl2016Processor::Ornl2016Processor() : EventProcessor(
        OFFSET, RANGE, "Ornl2016Processor") {

    debugging = to_bool(Globals::get()->GetOrnl2016Arguments().find("Debugging")->second);
    Vout = Globals::get()->GetOrnl2016Arguments().find("MkVandle")->second;
    if (Vout == "damm"){
        Pvandle = true;
        VoutDamm = true;
    }else if (Vout =="root") {
        Pvandle = true;
        VoutRoot = true;
    }else if (Vout == "both"){
        Pvandle = true;
        VoutRoot = true;
        VoutDamm = true;
    }else{
        Pvandle=VoutDamm=VoutRoot=false;
    }

    /*
     *  } else if (name == "WaveformAnalyzer") {
        std::vector <std::string> tokens =
            StringManipulation::TokenizeString(analyzer.attribute("ignored").as_string(""), ",");
        vecAnalyzer.emplace_back()(new WaveformAnalyzer(std::set<std::string>(tokens.begin(), tokens.end())));
    } else {
    */

    // parsing ignore list for miss behaving gamma ray dectors
    std::vector <std::string> HagIgnored = StringManipulation::TokenizeString(Globals::get()->GetOrnl2016Arguments().find("HagIgnore")->second, ",");
    std::vector <std::string> NaiIgnored = StringManipulation::TokenizeString(Globals::get()->GetOrnl2016Arguments().find("NaiIgnore")->second, ",");
    std::vector <std::string> GeIgnored =  StringManipulation::TokenizeString(Globals::get()->GetOrnl2016Arguments().find("GeIgnore")->second, ",");

    hagIgnore = std::set<std::string>(HagIgnored.begin(),HagIgnored.end());
    naiIgnore = std::set<std::string>(NaiIgnored.begin(),NaiIgnored.end());
    geIgnore = std::set<std::string>(GeIgnored.begin(),GeIgnored.end());


    SupBetaWin.second=Globals::get()->GetOrnl2016Arguments().find("SupBetaWin")->second.c_str();
    SupBetaWin.first= atof(SupBetaWin.second.c_str());

    associatedTypes.insert("ge");
    associatedTypes.insert("nai");
    associatedTypes.insert("labr3");
    associatedTypes.insert("beta");

    if (Pvandle) {
        associatedTypes.insert("vandle");
    }

    LgammaThreshold_ = atof(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_L")->second.c_str());
    LsubEventWindow_ = atof(Globals::get()->GetOrnl2016Arguments().find("sub_event_L")->second.c_str());
    NgammaThreshold_ = atof(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_N")->second.c_str());
    NsubEventWindow_ = atof(Globals::get()->GetOrnl2016Arguments().find("sub_event_N")->second.c_str());
    GgammaThreshold_ = atof(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_G")->second.c_str());
    GsubEventWindow_ = atof(Globals::get()->GetOrnl2016Arguments().find("sub_event_G")->second.c_str());

    // initalize addback vectors
    LaddBack_.emplace_back(ScintAddBack(0, 0, 0));
    NaddBack_.emplace_back(ScintAddBack(0, 0, 0));
    GaddBack_.emplace_back(ScintAddBack(0, 0, 0));

    // ROOT file Naming
    string hisPath = Globals::get()->GetOutputPath();
    string hisfilename = hisPath + Globals::get()->GetOutputFileName()+"-ONRL2016";
    string rootname = hisfilename + "-gammaSing.root";
    string rootname2 = hisfilename + "-gammaAddBk.root";
    string rootname3 = hisfilename + "-histo.root";
    string rootname4 = hisfilename + "-vandleDebug.root";
    string rootname5 = hisfilename + "-vandle.root";

    // Start Primary Root File
    rootFName_ = new TFile(rootname.c_str(), "RECREATE");
    TauxL = new TTree("TauxL", "LaBr Tree for ORNL2016");
    TauxN = new TTree("TauxN", "NaI Tree for ORNL2016");
    TauxG = new TTree("TauxG", "HPGe Tree for ORNL2016");

    //Taux Stuff
    //singBranch = Taux->Branch("sing", &sing,"LaBr[16]/D:NaI[10]/D:Ge[4]/D:beta/D:eventNum/D:cycle/i:gMulti/i:nMulti/i:hMulti/i:bMulti/i");
    //rootGstrutInit(sing);

    TauxL->Branch("aux_LaBrEn",&aux_LaBrEn);
    TauxL->Branch("aux_LaBrNum",&aux_LaBrNum);
    TauxL->Branch("aux_LaBrTime",&aux_LaBrTime);
    TauxL->Branch("aux_LaBrTdiff",&aux_LaBrTdiff);
    TauxL->Branch("aux_lMulti",&aux_LaBrMulti);
    TauxL->Branch("aux_LaBrHasLRBeta",&aux_LaBrHasLRBeta);
    TauxL->Branch("aux_BetaEn",&aux_BetaEn);
    TauxL->Branch("aux_BetaTime",&aux_BetaTime);
    TauxL->Branch("aux_BetaMulti",&aux_BetaMulti);
    TauxL->Branch("aux_cycle",&aux_cycle);
    TauxL->Branch("aux_cycleSTime",&aux_cycleSTime);
    TauxL->Branch("aux_eventNum",&aux_eventNum);


    TauxN->Branch("aux_NaIEn",&aux_NaIEn);
    TauxN->Branch("aux_NaINum",&aux_NaINum);
    TauxN->Branch("aux_NaITime",&aux_NaITime);
    TauxN->Branch("aux_NaITdiff",&aux_NaITdiff);
    TauxN->Branch("aux_nMulti",&aux_NaIMulti);
    TauxN->Branch("aux_NaIHasLRBeta",&aux_NaIHasLRBeta);
    TauxN->Branch("aux_BetaEn",&aux_BetaEn);
    TauxN->Branch("aux_BetaTime",&aux_BetaTime);
    TauxN->Branch("aux_BetaMulti",&aux_BetaMulti);
    TauxN->Branch("aux_cycle",&aux_cycle);
    TauxN->Branch("aux_cycleSTime",&aux_cycleSTime);
    TauxN->Branch("aux_eventNum",&aux_eventNum);

    TauxG->Branch("aux_GeEn",&aux_GeEn);
    TauxG->Branch("aux_GeNum",&aux_GeNum);
    TauxG->Branch("aux_GeTime",&aux_GeTime);
    TauxG->Branch("aux_GeTdiff",&aux_GeTdiff);
    TauxG->Branch("aux_gMulti",&aux_GeMulti);
    TauxG->Branch("aux_GeHasLRBeta",&aux_GeHasLRBeta);
    TauxG->Branch("aux_BetaEn",&aux_BetaEn);
    TauxG->Branch("aux_BetaTime",&aux_BetaTime);
    TauxG->Branch("aux_BetaMulti",&aux_BetaMulti);
    TauxG->Branch("aux_cycle",&aux_cycle);
    TauxG->Branch("aux_cycleSTime",&aux_cycleSTime);
    TauxG->Branch("aux_eventNum",&aux_eventNum);

    TauxL->SetAutoFlush(3000);
    TauxN->SetAutoFlush(3000);
    TauxG->SetAutoFlush(3000);


    //Start second Root File
    rootFName2_ = new TFile(rootname2.c_str(), "RECREATE");
    Tadd = new TTree("Tadd", "Tree for Addbacks @ ORNL2016");
    gProcBranch = Tadd->Branch("Gpro", &Gpro, "AbE/D:AbEvtNum/D:Multi/D");
    lProcBranch = Tadd->Branch("Lpro", &Lpro, "AbE/D:AbEvtNum/D:Multi/D");
    nProcBranch = Tadd->Branch("Npro", &Npro, "AbE/D:AbEvtNum/D:Multi/D");

    rootGstrutInit2(Gpro);
    rootGstrutInit2(Lpro);
    rootGstrutInit2(Npro);
    Tadd->SetAutoFlush(3000);
    // End second Root File

    if (Pvandle) {
        // Start Third  RootFile
        rootFName3_ = new TFile(rootname3.c_str(), "RECREATE");
        qdcVtof_ = new TH2D("qdcVtof", "", 1000, -100, 900, 32000, -16000, 16000);
        tofVGe_ = new TH2D("tofVGe", "", 1500, -100, 1400, 16000, 0, 16000);
        tofVLabr_ = new TH2D("tofVLaBr", "", 1500, -100, 1400, 16000, 0, 16000);
        tofVNai_ = new TH2D("tofVNaI", "", 1500, -100, 1400, 16000, 0, 16000);

        // End (Histo) RootFile
    }

    // Start fourth debugging RootFille
    if (debugging) {
        rootFName4_ = new TFile(rootname4.c_str(), "RECREATE");
        Wave = new TTree("Wave", "Tree for Waveform Analyzer Debugging");

        Wave->Branch("evtNumber", &evtNumber);
        Wave->Branch("output_name", &output_name);
        Wave->Branch("vandle_subtype", &vandle_subtype);
        Wave->Branch("vandle_BarQDC", &vandle_BarQDC);
        Wave->Branch("vandle_lQDC", &vandle_lQDC);
        Wave->Branch("vandle_rQDC", &vandle_rQDC);
        Wave->Branch("vandle_QDCPos", &vandle_QDCPos);
        Wave->Branch("vandle_TOF", &vandle_TOF);
        Wave->Branch("vandle_lSnR", &vandle_lSnR);
        Wave->Branch("vandle_rSnR", &vandle_rSnR);
        Wave->Branch("vandle_lAmp", &vandle_lAmp);
        Wave->Branch("vandle_rAmp", &vandle_rAmp);
        Wave->Branch("vandle_lMaxAmpPos", &vandle_lMaxAmpPos);
        Wave->Branch("vandle_rMaxAmpPos", &vandle_rMaxAmpPos);
        Wave->Branch("vandle_lAveBaseline", &vandle_lAveBaseline);
        Wave->Branch("vandle_rAveBaseline", &vandle_rAveBaseline);
        Wave->Branch("vandle_barNum", &vandle_barNum);
        Wave->Branch("vandle_TAvg", &vandle_TAvg);
        Wave->Branch("vandle_Corrected_TAvg", &vandle_Corrected_TAvg);
        Wave->Branch("vandle_TDiff", &vandle_TDiff);
        Wave->Branch("vandle_Corrected_TDiff", &vandle_Corrected_TDiff);
        Wave->Branch("vandle_ltrace", &vandle_ltrace);
        Wave->Branch("vandle_rtrace", &vandle_rtrace);
        Wave->Branch("beta_BarQDC", &beta_BarQDC);
        Wave->Branch("beta_lQDC", &beta_lQDC);
        Wave->Branch("beta_rQDC", &beta_rQDC);
        Wave->Branch("beta_lSnR", &beta_lSnR);
        Wave->Branch("beta_rSnR", &beta_rSnR);
        Wave->Branch("beta_lAmp", &beta_lAmp);
        Wave->Branch("beta_rAmp", &beta_rAmp);
        Wave->Branch("beta_lMaxAmpPos", &beta_lMaxAmpPos);
        Wave->Branch("beta_rMaxAmpPos", &beta_rMaxAmpPos);
        Wave->Branch("beta_lAveBaseline", &vandle_lAveBaseline);
        Wave->Branch("beta_rAveBaseline", &vandle_rAveBaseline);
        Wave->Branch("beta_barNum", &beta_barNum);
        Wave->Branch("beta_TAvg", &beta_TAvg);
        Wave->Branch("beta_Corrected_TAvg", &beta_Corrected_TAvg);
        Wave->Branch("beta_TDiff", &beta_TDiff);
        Wave->Branch("beta_Corrected_TDiff", &beta_Corrected_TDiff);
        Wave->Branch("beta_ltrace", &beta_ltrace);
        Wave->Branch("beta_rtrace", &beta_rtrace);


        Wave->SetAutoFlush(3000);
        //End debugging RootFille
    }

    // Tvan Stuff
    if (Pvandle && VoutRoot) {
        rootFName5_ = new TFile(rootname5.c_str(), "RECREATE");
        Tvan = new TTree("Tvan", "Tree for Vandle Stuff (coincident gammas as well) @ ORNL2016");

        Tvan->Branch("evtNumber", &evtNumber);
        Tvan->Branch("output_name", &output_name);

        Tvan->Branch("vandle_BarQDC", &vandle_BarQDC);
        Tvan->Branch("vandle_barNum", &vandle_barNum);
        Tvan->Branch("vandle_TOF", &vandle_TOF);
        Tvan->Branch("vandle_TAvg", &vandle_TAvg);
        Tvan->Branch("vandle_Corrected_TAvg", &vandle_Corrected_TAvg);
        Tvan->Branch("vandle_TDiff", &vandle_TDiff);

        Tvan->Branch("vandle_QDCPos", &vandle_QDCPos);
        Tvan->Branch("vandle_lSnR", &vandle_lSnR);
        Tvan->Branch("vandle_rSnR", &vandle_rSnR);
        Tvan->Branch("vandle_lAmp", &vandle_lAmp);
        Tvan->Branch("vandle_rAmp", &vandle_rAmp);
        Tvan->Branch("vandle_lMaxAmpPos", &vandle_lMaxAmpPos);
        Tvan->Branch("vandle_rMaxAmpPos", &vandle_rMaxAmpPos);
        Tvan->Branch("vandle_lAveBaseline", &vandle_lAveBaseline);
        Tvan->Branch("vandle_rAveBaseline", &vandle_rAveBaseline);
        Tvan->Branch("vandle_ltrace", &vandle_ltrace);
        Tvan->Branch("vandle_rtrace", &vandle_rtrace);

        Tvan->Branch("vandle_ge", &vandle_ge);
        Tvan->Branch("vandle_labr3", &vandle_labr3);
        Tvan->Branch("vandle_nai", &vandle_nai);

        Tvan->SetAutoFlush(3000);

    }

}


Ornl2016Processor::~Ornl2016Processor() {

    //sing
    rootFName_->Write();
    rootFName_->Close();
    delete (rootFName_);

    //addback
    rootFName2_->Write();
    rootFName2_->Close();
    delete (rootFName2_);

    //Wave Debugging
    if (debugging) {
        rootFName4_->Write();
        rootFName4_->Close();
        delete (rootFName4_);
    }

    if (Pvandle) {
        //histo
        rootFName3_->Write();
        rootFName3_->Close();
        delete (rootFName3_);

        if (VoutRoot){
            //Vandle + coincidence
            rootFName5_->Write();
            rootFName5_->Close();
            delete (rootFName5_);
        }
    }

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
    double plotOffset_ = 200;
    double plotMult_ = 2;

    map<unsigned int, pair<double, double> > lrtBetas;
    BarMap betas, vbars;

    hasLRbeta = false;
    hasLRbeta = TreeCorrelator::get()->place("Beta")->status(); 
    //might need a static initialize to false + reset at the end

    if (Pvandle || debugging){
        if( event.GetSummary("vandle")->GetList().size() != 0) {
        vbars = ((VandleProcessor *) DetectorDriver::get()->GetProcessor("VandleProcessor"))->GetBars();
        }
    }
    if (event.GetSummary("beta:double")->GetList().size() != 0) {
        betas = ((DoubleBetaProcessor *) DetectorDriver::get()->GetProcessor("DoubleBetaProcessor"))->GetBars();
        lrtBetas = ((DoubleBetaProcessor *) DetectorDriver::get()->GetProcessor("DoubleBetaProcessor"))->GetLowResBars();
    }

    static const vector<ChanEvent *> &labr3Evts = event.GetSummary("labr3")->GetList();
    static const vector<ChanEvent *> &naiEvts = event.GetSummary("nai")->GetList();
    static const vector<ChanEvent *> &geEvts = event.GetSummary("ge")->GetList();



    /// PLOT ANALYSIS HISTOGRAMS-------------------------------------------------------------------------------------------------------------------------------------

    // initalize the root structures
    //rootGstrutInit(sing);
    rootGstrutInit2(Gpro);
    rootGstrutInit2(Lpro);
    rootGstrutInit2(Npro);

    std::string beta = "beta";
    rootAuxRoot(beta);
    std::string Ge = "Ge";
    rootAuxRoot(Ge);
    std::string NaI = "NaI";
    rootAuxRoot(NaI);
    std::string LaBr = "LaBr";
    rootAuxRoot(LaBr);

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
            aux_cycleSTime = cycleLast;
            cycleLast = cycleTime;
            cycleNum++;
            cout << "Cycle Change " << endl << "Tdiff (Cycle start and Now) (ms)= " << tdiff << endl
                 << "Starting on Cycle #" << cycleNum << endl;
        }
    }
    aux_cycle = cycleNum;

    //set multiplicys for sing branch based on the size of the detector maps for the event. limitation: sub event is smaller than full event this will end up being too large
    aux_GeMulti = geEvts.size();
    aux_NaIMulti = naiEvts.size();
    aux_LaBrMulti = labr3Evts.size();
    aux_BetaMulti = lrtBetas.size();
    aux_eventNum = evtNum;

    BetaList.clear();

    for (map<unsigned int, pair<double, double> >::iterator bIt = lrtBetas.begin(); bIt != lrtBetas.end(); bIt++) {
        aux_BetaTime = bIt->second.first;
        plot(D_BETASCALARRATE, cycleNum);//PLOTTING BETA SCALAR SUM per CYCLE (LIKE 759 but per cycle vs per second
        plot(D_BETAENERGY, bIt->second.second);
        aux_BetaEn = bIt->second.second;
        BetaList.emplace_back(make_pair(aux_BetaTime,aux_BetaEn));
    }

    //NaI ONLY
    // ----------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itNai = naiEvts.begin(); itNai != naiEvts.end(); itNai++) {
        int naiNum = (*itNai)->GetChannelNumber();
        aux_NaINum = naiNum;
        aux_NaIEn = (*itNai)->GetCalibratedEnergy();
        aux_NaITime = (*itNai)->GetTime();
        //sing.NaI[naiNum] = (*itNai)->GetCalibratedEnergy();
        plot(D_NAISUM, (*itNai)->GetCalibratedEnergy()); //plot totals


        //Beta Gate and addback
        aux_NaIHasLRBeta=hasLRbeta;
        if (hasLRbeta) {  //Beta Gate
            plot(D_NAIBETA, (*itNai)->GetCalibratedEnergy()); //plot beta-gated totals

            double TDiff;
            for (vector<pair<double, double>>::const_iterator itBetaList = BetaList.begin();
                 itBetaList !=BetaList.end(); itBetaList++) {

                TDiff = aux_NaITime - (*itBetaList).first;

                if (TDiff < 0) {
                    continue;
                } else if (TDiff >= 0) {
                    aux_BetaEn = (*itBetaList).second;
                    aux_BetaTime = (*itBetaList).first;
                    aux_NaITdiff = TDiff;

                    /*cout<<fixed<<"BetaList.size() = "<<BetaList.size()<<endl;
                    cout <<fixed<< "NaI Time = "<<(*itNai)->GetTime() << endl;
                    cout <<fixed<< "BetaList Time = "<<(*itBetaList).first<<endl;
                    cout <<fixed<< "aux_BetaTime = "<<aux_BetaTime<<endl;
                    cout <<fixed<< "aux_GeTDiff = "<<aux_NaITdiff<<endl;
                    cout <<fixed<< "TDiff = "<<TDiff<<endl;
                    cout <<fixed<< "Diff = "<<(*itNai)->GetTime() - (*itBetaList).first<<endl;
                    cout<<fixed<<"BetaMulti = "<<aux_BetaMulti<<endl;
                    cout<<fixed<<"Evt Number = "<<evtNum<<endl;
*/
                    if (TDiff<=SupBetaWin.first)
                        plot(D_DBGnai,(*itNai)->GetCalibratedEnergy());

                    break;
                }
            }
            //begin addback calulations for NaI
            double energy = (*itNai)->GetCalibratedEnergy();
            double time = (*itNai)->GetTimeSansCfd();

            if (energy < NgammaThreshold_) {
                continue;
            }//end energy comp if statment
            double t1 = Globals::get()->GetClockInSeconds();
            double dtime = abs(time - NrefTime) * t1;

            if (dtime >NsubEventWindow_) {
                //if event time is outside sub event window start new addback after filling tree
                Npro.AbEvtNum = evtNum;
                Npro.AbE = NaddBack_.back().energy;
                Npro.Multi = NaddBack_.back().multiplicity;
                Tadd->Fill();
                NaddBack_.emplace_back(ScintAddBack());
            }//end subEvent IF
            NaddBack_.back().energy += energy; // if still inside sub window: incrament
            NaddBack_.back().time = time;
            NaddBack_.back().multiplicity += 1;
            NrefTime = time;

        }//end beta gate
        TauxN->Fill();
    } //NaI loop End

    //HPGe ONLY---------------------------------------------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
         itGe != geEvts.end(); itGe++) {
        int geNum = (*itGe)->GetChanID().GetLocation();
        aux_GeEn = (*itGe)->GetCalibratedEnergy();
        aux_GeNum = geNum;
        aux_GeTime = (*itGe)->GetTime();

        plot(D_HPGESUM, (*itGe)->GetCalibratedEnergy()); //plot non-gated totals

        aux_GeHasLRBeta=hasLRbeta;

        if (hasLRbeta) { //beta-gated Processing to cut LaBr contamination out
            plot(D_HPGESUMBETA,(*itGe)->GetCalibratedEnergy());

            double TDiff;
            for (vector<pair<double, double>>::const_iterator itBetaList = BetaList.begin();
                 itBetaList !=BetaList.end(); itBetaList++) {

                TDiff = aux_GeTime - (*itBetaList).first;

                if (TDiff < 0) {
                     continue;
                } else if (TDiff >= 0) {
                    aux_BetaEn = (*itBetaList).second;
                    aux_BetaTime = (*itBetaList).first;
                    aux_GeTdiff = TDiff;
/*
                    cout<<fixed<<"BetaList.size() = "<<BetaList.size()<<endl;
                    cout <<fixed<< "HPGe Time = "<<(*itGe)->GetTime() << endl;
                    cout <<fixed<< "BetaList Time = "<<(*itBetaList).first<<endl;
                    cout <<fixed<< "aux_BetaTime = "<<aux_BetaTime<<endl;
                    cout <<fixed<< "aux_GeTDiff = "<<aux_GeTdiff<<endl;
                    cout <<fixed<< "TDiff = "<<TDiff<<endl;
                    cout <<fixed<< "Diff = "<<(*itGe)->GetTime() - (*itBetaList).first<<endl;
                    cout<<fixed<<"BetaMulti = "<<aux_BetaMulti<<endl;
                    cout<<fixed<<"Evt Number = "<<evtNum<<endl;
*/
                    if (TDiff<=SupBetaWin.first)
                        plot(D_DBGge,(*itGe)->GetCalibratedEnergy());

                    break;
                }
            }

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
                Tadd->Fill();
                GaddBack_.emplace_back(ScintAddBack());
            } //end subEvent IF

            GaddBack_.back().energy += energy;
            GaddBack_.back().time = time;
            GaddBack_.back().multiplicity += 1;
            GrefTime = time;


        } //end BetaGate
        TauxG->Fill();
    } //GE loop end

    //HAGRiD ONLY-------------------------------------------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itLabr = labr3Evts.begin();
         itLabr != labr3Evts.end(); itLabr++) {
        int labrNum = (*itLabr)->GetChanID().GetLocation();
        plot(D_LABR3SUM, (*itLabr)->GetCalibratedEnergy()); //plot non-gated totals
        //sing.LaBr[labrNum] = (*itLabr)->GetCalibratedEnergy();

        aux_LaBrNum = labrNum;
        aux_LaBrEn = (*itLabr)->GetCalibratedEnergy();
        aux_LaBrTime = (*itLabr)->GetTimeSansCfd();

        aux_LaBrHasLRBeta=hasLRbeta;

        if (hasLRbeta) {

            plot(D_LABR3BETA, (*itLabr)->GetCalibratedEnergy()); //plot beta-gated totals

            double TDiff;
            for (vector<pair<double, double>>::const_iterator itBetaList = BetaList.begin();
                 itBetaList !=BetaList.end(); itBetaList++) {

                TDiff = aux_LaBrTime - (*itBetaList).first;

                if (TDiff < 0) {
                    continue;
                } else if (TDiff >= 0) {
                    aux_BetaEn = (*itBetaList).second;
                    aux_BetaTime = (*itBetaList).first;
                    aux_LaBrTdiff = TDiff;
/*
                                       cout<<fixed<<"BetaList.size() = "<<BetaList.size()<<endl;
                                       cout <<fixed<< "LaBr Time = "<<(*itLabr)->GetTime() << endl;
                                       cout <<fixed<< "BetaList Time = "<<(*itBetaList).first<<endl;
                                       cout <<fixed<< "aux_BetaTime = "<<aux_BetaTime<<endl;
                                       cout <<fixed<< "aux_LaBrTDiff = "<<aux_LaBrTdiff<<endl;
                                       cout <<fixed<< "TDiff = "<<TDiff<<endl;
                                       cout <<fixed<< "Diff = "<<(*itLabr)->GetTime() - (*itBetaList).first<<endl;
                                       cout<<fixed<<"BetaMulti = "<<aux_BetaMulti<<endl;
                                       cout<<fixed<<"Evt Number = "<<evtNum<<endl;
*/
                    if (TDiff<=SupBetaWin.first)
                        plot(D_DBGlabr,(*itLabr)->GetCalibratedEnergy());

                    break;
                }
            }

           //Addback Calc


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
                Tadd->Fill();
                LaddBack_.emplace_back(ScintAddBack());
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
        TauxL->Fill();
        }//end beta gate


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

             double tof = bar.GetTimeAverage() - start.GetTimeAverage() + cal.GetTofOffset(startLoc);

             /*double corTof = ((VandleProcessor *) DetectorDriver::get()->GetProcessor("VandleProcessor"))->
                     CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());
*/
             plot(DD_QDCVTOF, (tof * 2) + plotOffset_, bar.GetQdc());

             qdcVtof_->Fill(tof,bar.GetQdc());
            if (VoutRoot || debugging) {
                vandle_subtype = bar.GetType();
                vandle_lSnR = bar.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
                vandle_rSnR = bar.GetRightSide().GetTrace().GetSignalToNoiseRatio();
                vandle_lAmp = bar.GetLeftSide().GetMaximumValue();
                vandle_rAmp = bar.GetRightSide().GetMaximumValue();
                vandle_lMaxAmpPos = bar.GetLeftSide().GetMaximumPosition();
                vandle_rMaxAmpPos = bar.GetRightSide().GetMaximumPosition();
                vandle_lAveBaseline = bar.GetLeftSide().GetAveBaseline();
                vandle_rAveBaseline = bar.GetRightSide().GetAveBaseline();
                vandle_BarQDC = bar.GetQdc();
                vandle_QDCPos = bar.GetQdcPosition();
                vandle_lQDC = bar.GetLeftSide().GetTraceQdc();
                vandle_rQDC = bar.GetRightSide().GetTraceQdc();
                vandle_TOF = tof;
                vandle_barNum = barLoc;
                vandle_TAvg = bar.GetTimeAverage();
                vandle_Corrected_TAvg = bar.GetCorTimeAve();
                vandle_TDiff = bar.GetTimeDifference();
                vandle_Corrected_TDiff = bar.GetCorTimeDiff();
                vandle_ltrace = bar.GetLeftSide().GetTrace();
                vandle_rtrace = bar.GetRightSide().GetTrace();

                beta_lSnR = start.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
                beta_rSnR = start.GetRightSide().GetTrace().GetSignalToNoiseRatio();
                beta_lAmp = start.GetLeftSide().GetMaximumValue();
                beta_rAmp = start.GetRightSide().GetMaximumValue();
                beta_lMaxAmpPos = start.GetLeftSide().GetMaximumPosition();
                beta_rMaxAmpPos = start.GetRightSide().GetMaximumPosition();
                beta_lAveBaseline = start.GetLeftSide().GetAveBaseline();
                beta_rAveBaseline = start.GetRightSide().GetAveBaseline();
                beta_BarQDC = start.GetQdc();
                beta_lQDC = start.GetLeftSide().GetTraceQdc();
                beta_rQDC = start.GetRightSide().GetTraceQdc();
                beta_barNum = startLoc;
                beta_TAvg = start.GetTimeAverage();
                beta_Corrected_TAvg = start.GetCorTimeAve();
                beta_TDiff = start.GetTimeDifference();
                beta_Corrected_TDiff = start.GetCorTimeDiff();
                beta_ltrace = start.GetLeftSide().GetTrace();
                beta_rtrace = start.GetRightSide().GetTrace();
            }

             //TracePloting commented
/*          int itTVl=0;
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
*/


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

                 if (hagIgnore.find(to_string(labrNum)) != hagIgnore.end())
                     continue;
                 if(VoutRoot) {
                     vandle_labr3.emplace_back(make_pair(labrNum,labrEn));
                 }


                 plot(DD_TOFVSHAGRID, labrEn, tof * plotMult_ + 200);
                 plot(DD_DETcheckHag,labrEn,labrNum);
                 tofVLabr_->Fill(labrEn, tof);

             };

             //Nai loop for mVan
             int naiNum;
             double naiEn;
             for (vector<ChanEvent *>::const_iterator itNai = naiEvts.begin();
                  itNai != naiEvts.end(); itNai++) {
                 naiNum = (*itNai)->GetChanID().GetLocation();
                 naiEn = (*itNai)->GetCalibratedEnergy();
                 if (naiIgnore.find(to_string(naiNum)) != naiIgnore.end())
                     continue;

                 if(VoutRoot) {
                     vandle_nai.emplace_back(make_pair(naiNum,naiEn));
                 }


                 plot(DD_TOFVSNAI, naiEn, tof * plotMult_ + 200);
                 plot(DD_DETcheckNai,naiEn,naiNum);
                 tofVNai_->Fill(naiEn,tof);


             };

             //ge loop for mVan
             int geNum;
             double geEn;
             for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
                  itGe != geEvts.end(); itGe++) {
                 geNum = (*itGe)->GetChanID().GetLocation();
                 geEn = (*itGe)->GetCalibratedEnergy();
                 if (geIgnore.find(to_string(geNum)) != geIgnore.end())
                     continue;

                 if(VoutRoot) {
                     vandle_ge.emplace_back(make_pair(geNum,geEn));
                 }


                 plot(DD_TOFVSGE, geEn, tof * plotMult_ + 200);
                 plot(DD_DETcheckGe,geEn,geNum);
                  tofVGe_->Fill(geEn,tof);


             };
         };

         if (debugging) {
             Wave->Fill();
         }

         if(Pvandle && VoutRoot) {
             Tvan->Fill();
             vandle_ge.clear();
             vandle_labr3.clear();
             vandle_nai.clear();
         }

     };//End VANDLE

    evtNumber = evtNum;
    aux_eventNum = evtNum;


    evtNum++;
    EndProcess();
    return (true);
}
