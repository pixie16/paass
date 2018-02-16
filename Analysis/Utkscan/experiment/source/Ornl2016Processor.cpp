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

    // parsing ignore list for miss behaving gamma ray dectors
    std::vector <std::string> HagIgnored = StringManipulation::TokenizeString(Globals::get()->GetOrnl2016Arguments().find("HagIgnore")->second, ",");
    std::vector <std::string> NaiIgnored = StringManipulation::TokenizeString(Globals::get()->GetOrnl2016Arguments().find("NaiIgnore")->second, ",");
    std::vector <std::string> GeIgnored =  StringManipulation::TokenizeString(Globals::get()->GetOrnl2016Arguments().find("GeIgnore")->second, ",");

    hagIgnore = std::set<std::string>(HagIgnored.begin(),HagIgnored.end());
    naiIgnore = std::set<std::string>(NaiIgnored.begin(),NaiIgnored.end());
    geIgnore = std::set<std::string>(GeIgnored.begin(),GeIgnored.end());

    SupBetaWin.second=Globals::get()->GetOrnl2016Arguments().find("SupBetaWin")->second;
    SupBetaWin.first= strtod(SupBetaWin.second.c_str(), nullptr);

    associatedTypes.insert("ge");
    associatedTypes.insert("nai");
    associatedTypes.insert("labr3");
    associatedTypes.insert("beta");

    LgammaThreshold_ = strtod(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_L")->second.c_str(), nullptr);
    LsubEventWindow_ = strtod(Globals::get()->GetOrnl2016Arguments().find("sub_event_L")->second.c_str(), nullptr);
    NgammaThreshold_ = strtod(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_N")->second.c_str(), nullptr);
    NsubEventWindow_ = strtod(Globals::get()->GetOrnl2016Arguments().find("sub_event_N")->second.c_str(), nullptr);
    GgammaThreshold_ = strtod(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_G")->second.c_str(), nullptr);
    GsubEventWindow_ = strtod(Globals::get()->GetOrnl2016Arguments().find("sub_event_G")->second.c_str(), nullptr);

    // initalize addback vectors
    LaddBack_.emplace_back(ScintAddBack(0, 0, 0));
    NaddBack_.emplace_back(ScintAddBack(0, 0, 0));
    GaddBack_.emplace_back(ScintAddBack(0, 0, 0));

    // ROOT file Naming
    string hisPath = Globals::get()->GetOutputPath();
    string hisfilename = hisPath + Globals::get()->GetOutputFileName()+"-ONRL2016";
    string rootname = hisfilename + "-gammaSing.root";

    // Start Primary Root File
    rootFName_ = new TFile(rootname.c_str(), "RECREATE");
    LaBr_tree = new TTree("LaBr_tree", "LaBr Tree for ORNL2016");
    NaI_tree = new TTree("NaI_tree", "NaI Tree for ORNL2016");
    HPGe_tree = new TTree("HPGe_tree", "HPGe Tree for ORNL2016");

    LaBr_tree->Branch("aux_LaBrEn",&aux_LaBrEn);
    LaBr_tree->Branch("aux_LaBrNum",&aux_LaBrNum);
    LaBr_tree->Branch("aux_LaBrTime",&aux_LaBrTime);
    LaBr_tree->Branch("aux_LaBrTdiff",&aux_LaBrTdiff);
    LaBr_tree->Branch("aux_lMulti",&aux_LaBrMulti);
    LaBr_tree->Branch("aux_LaBrHasLRBeta",&aux_LaBrHasLRBeta);
    LaBr_tree->Branch("aux_BetaEn",&aux_BetaEn);
    LaBr_tree->Branch("aux_BetaTime",&aux_BetaTime);
    LaBr_tree->Branch("aux_BetaMulti",&aux_BetaMulti);
    LaBr_tree->Branch("aux_cycle",&aux_cycle);
    LaBr_tree->Branch("aux_cycleSTime",&aux_cycleSTime);
    LaBr_tree->Branch("aux_eventNum",&aux_eventNum);

    NaI_tree->Branch("aux_NaIEn",&aux_NaIEn);
    NaI_tree->Branch("aux_NaINum",&aux_NaINum);
    NaI_tree->Branch("aux_NaITime",&aux_NaITime);
    NaI_tree->Branch("aux_NaITdiff",&aux_NaITdiff);
    NaI_tree->Branch("aux_nMulti",&aux_NaIMulti);
    NaI_tree->Branch("aux_NaIHasLRBeta",&aux_NaIHasLRBeta);
    NaI_tree->Branch("aux_BetaEn",&aux_BetaEn);
    NaI_tree->Branch("aux_BetaTime",&aux_BetaTime);
    NaI_tree->Branch("aux_BetaMulti",&aux_BetaMulti);
    NaI_tree->Branch("aux_cycle",&aux_cycle);
    NaI_tree->Branch("aux_cycleSTime",&aux_cycleSTime);
    NaI_tree->Branch("aux_eventNum",&aux_eventNum);

    HPGe_tree->Branch("aux_GeEn",&aux_GeEn);
    HPGe_tree->Branch("aux_GeNum",&aux_GeNum);
    HPGe_tree->Branch("aux_GeTime",&aux_GeTime);
    HPGe_tree->Branch("aux_GeTdiff",&aux_GeTdiff);
    HPGe_tree->Branch("aux_gMulti",&aux_GeMulti);
    HPGe_tree->Branch("aux_GeHasLRBeta",&aux_GeHasLRBeta);
    HPGe_tree->Branch("aux_BetaEn",&aux_BetaEn);
    HPGe_tree->Branch("aux_BetaTime",&aux_BetaTime);
    HPGe_tree->Branch("aux_BetaMulti",&aux_BetaMulti);
    HPGe_tree->Branch("aux_cycle",&aux_cycle);
    HPGe_tree->Branch("aux_cycleSTime",&aux_cycleSTime);
    HPGe_tree->Branch("aux_eventNum",&aux_eventNum);

    LaBr_tree->SetAutoFlush(3000);
    NaI_tree->SetAutoFlush(3000);
    HPGe_tree->SetAutoFlush(3000);


    addBack_tree = new TTree("addBack_tree", "Tree for Addbacks @ ORNL2016");
    gProcBranch = addBack_tree->Branch("Gpro", &Gpro, "AbE/D:AbEvtNum/D:Multi/D");
    lProcBranch = addBack_tree->Branch("Lpro", &Lpro, "AbE/D:AbEvtNum/D:Multi/D");
    nProcBranch = addBack_tree->Branch("Npro", &Npro, "AbE/D:AbEvtNum/D:Multi/D");

    rootGstrutInit2(Gpro);
    rootGstrutInit2(Lpro);
    rootGstrutInit2(Npro);
    addBack_tree->SetAutoFlush(3000);
    // End second Root File

}


Ornl2016Processor::~Ornl2016Processor() {

    rootFName_->Write("",TObject::kOverwrite);
    rootFName_->Close();
    delete (rootFName_);

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

    map<unsigned int, pair<double, double> > lrtBetas;
    BarMap betas, vbars;

    hasLRbeta = false;
    hasLRbeta = TreeCorrelator::get()->place("Beta")->status();
    //might need a static initialize to false + reset at the end

    if (! event.GetSummary("beta:double")->GetList().empty()) {
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

    //NaI ONLY-----------------------------------------------------------------
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

                    // cout<<fixed<<"BetaList.size() = "<<BetaList.size()<<endl;
                    // cout <<fixed<< "NaI Time = "<<(*itNai)->GetTime() << endl;
                    // cout <<fixed<< "BetaList Time = "<<(*itBetaList).first<<endl;
                    // cout <<fixed<< "aux_BetaTime = "<<aux_BetaTime<<endl;
                    // cout <<fixed<< "aux_GeTDiff = "<<aux_NaITdiff<<endl;
                    // cout <<fixed<< "TDiff = "<<TDiff<<endl;
                    // cout <<fixed<< "Diff = "<<(*itNai)->GetTime() - (*itBetaList).first<<endl;
                    // cout<<fixed<<"BetaMulti = "<<aux_BetaMulti<<endl;
                    // cout<<fixed<<"Evt Number = "<<evtNum<<endl;

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
                addBack_tree->Fill();
                NaddBack_.emplace_back(ScintAddBack());
            }//end subEvent IF
            NaddBack_.back().energy += energy; // if still inside sub window: incrament
            NaddBack_.back().time = time;
            NaddBack_.back().multiplicity += 1;
            NrefTime = time;

        }//end beta gate
        NaI_tree->Fill();
    } //NaI loop End

    //HPGe ONLY----------------------------------------------------------------
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

                    // cout<<fixed<<"BetaList.size() = "<<BetaList.size()<<endl;
                    // cout <<fixed<< "HPGe Time = "<<(*itGe)->GetTime() << endl;
                    // cout <<fixed<< "BetaList Time = "<<(*itBetaList).first<<endl;
                    // cout <<fixed<< "aux_BetaTime = "<<aux_BetaTime<<endl;
                    // cout <<fixed<< "aux_GeTDiff = "<<aux_GeTdiff<<endl;
                    // cout <<fixed<< "TDiff = "<<TDiff<<endl;
                    // cout <<fixed<< "Diff = "<<(*itGe)->GetTime() - (*itBetaList).first<<endl;
                    // cout<<fixed<<"BetaMulti = "<<aux_BetaMulti<<endl;
                    // cout<<fixed<<"Evt Number = "<<evtNum<<endl;

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
                addBack_tree->Fill();
                GaddBack_.emplace_back(ScintAddBack());
            } //end subEvent IF

            GaddBack_.back().energy += energy;
            GaddBack_.back().time = time;
            GaddBack_.back().multiplicity += 1;
            GrefTime = time;


        } //end BetaGate
        HPGe_tree->Fill();
    } //GE loop end

    //HAGRiD ONLY--------------------------------------------------------------
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

                   // cout<<fixed<<"BetaList.size() = "<<BetaList.size()<<endl;
                   // cout <<fixed<< "LaBr Time = "<<(*itLabr)->GetTime() << endl;
                   // cout <<fixed<< "BetaList Time = "<<(*itBetaList).first<<endl;
                   // cout <<fixed<< "aux_BetaTime = "<<aux_BetaTime<<endl;
                   // cout <<fixed<< "aux_LaBrTDiff = "<<aux_LaBrTdiff<<endl;
                   // cout <<fixed<< "TDiff = "<<TDiff<<endl;
                   // cout <<fixed<< "Diff = "<<(*itLabr)->GetTime() - (*itBetaList).first<<endl;
                   // cout<<fixed<<"BetaMulti = "<<aux_BetaMulti<<endl;
                   // cout<<fixed<<"Evt Number = "<<evtNum<<endl;

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
                addBack_tree->Fill();
                LaddBack_.emplace_back(ScintAddBack());
            }// end if for new entry in vector

            LaddBack_.back().energy += energy;
            LaddBack_.back().time = time;
            LaddBack_.back().multiplicity += 1;
            LrefTime = time;

        LaBr_tree->Fill();
        }//end beta gate


    } //Hagrid loop end

    aux_eventNum = evtNum;


    evtNum++;
    EndProcess();
    return (true);
}
