/** \file TACProcessor.cpp
 * \brief A Simple Processor for TAC signals in Pixie.
 * \author D. Perez-Loureiro
 * \date June 2, 2017
 */

#include <iostream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TACProcessor.hpp"
#include "HighResTimingData.hpp"

#include <TFile.h>
#include <TH1F.h>
#include <TH1I.h>
#include <TTree.h>

TFile *rootFile;
TH1F *hTAC;
TH1I *TACtrace;
TTree *thetree;

Int_t StartQDC=-999;
Int_t StopQDC= -999;
Double_t MaxTAC= -999;

namespace dammIds {
    namespace detTAC {
        const int D_ENERGY  = 0; //!< ID for the energy of the TAC detector
        const int DD_TAC_VS_PULSER = 1; //!< Energy TAC vs. Energy Pulser
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::detTAC;

TACProcessor::TACProcessor():
    EventProcessor(OFFSET, RANGE, "TACProcessor") {
    associatedTypes.insert("TAC");
    associatedTypes.insert("pulser");
    stringstream rootname;
    rootname << Globals::get()->outputPath(Globals::get()->outputFile())
             << ".root";
    rootFile = new TFile(rootname.str().c_str(),"RECREATE");

hTAC = new TH1F("hTAC","TAC Maxima Spectrum ",4096,0,4095);
TACtrace = new TH1I("trace","TAC Traces ",800,0,800);
thetree = new TTree("T","");
thetree->Branch("TAC_Traces",&TACtrace);
thetree->Branch("StartQDC",&StartQDC,"StQDC/I");
thetree->Branch("StopQDC",&StopQDC,"SpQDC/I");
thetree->Branch("MaxTAC",&MaxTAC,"Max/D");
}

TACProcessor::TACProcessor(const double &a):
    EventProcessor(OFFSET, RANGE, "TACProcessor") {
    associatedTypes.insert("TAC");
    associatedTypes.insert("pulser");
    a_ = a;
}

TACProcessor::~TACProcessor(){
hTAC->Write();
thetree->Write();
rootFile->Write();
rootFile->Close();
 delete (rootFile);
}
void TACProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_ENERGY, SA, "TAC Energy");
    DeclareHistogram2D(DD_TAC_VS_PULSER, SA, SA,
                       "TAC Energy vs. Pulser Energy");
}

bool TACProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    evts_ = event.GetSummary("TAC")->GetList();

    //std::cout<<"TACProcessor::PreProcess()->TAC Size is "<<evts_.size()<<std::endl;

    for(vector<ChanEvent*>::const_iterator it = evts_.begin();
        it != evts_.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
	//std::cout<<"Location is "<<location<<std::endl;
        //if(location == 0)
       	for(vector<unsigned int>::const_iterator itA = (*it)->GetTrace().begin();
            itA != (*it)->GetTrace().end(); itA++) {
	  int bin = (int)(itA-(*it)->GetTrace().begin());
	  TACtrace->SetBinContent(bin,*itA);
	  //Only output the 500th trace to make sure that we are not at the
	  // beginning of the file and we're a ways into the data.
	}
        Double_t Max_=(*it)->GetTrace().GetMaxInfo().second;
	Int_t MaxBin=(*it)->GetTrace().GetMaxInfo().first;
	std::cout<<"Maximum is "<<Max_<<" at "<<4*MaxBin<<" ns"<<std::endl;
        //thetree->Fill();
	//TACtrace->Reset();
	MaxTAC=Max_;
	hTAC->Fill(Max_);
}
    return(true);
}

bool TACProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

   //  if (evts_.size()==0) return(false);

   //  //Define a map to hold the information
     TimingMap pulserMap;


     static const vector<ChanEvent*> & pulserEvents =
         event.GetSummary("pulser")->GetList();

     //std::cout<<" TACProcessor::Process()->Pulser "<<pulserEvents.size()<<std::endl;
    for (vector<ChanEvent *>::const_iterator itPulser = pulserEvents.begin();
          itPulser != pulserEvents.end(); itPulser++) {
         int location = (*itPulser)->GetChanID().GetLocation();
         string subType = (*itPulser)->GetChanID().GetSubtype();



        TimingDefs::TimingIdentifier key(location, subType);
         pulserMap.insert(make_pair(key, HighResTimingData(*(*itPulser))));
     }

    if (pulserMap.empty() || pulserMap.size() % 2 != 0) {
    EndProcess();
	return(false);
}
 
    HighResTimingData start =
             (*pulserMap.find(make_pair(0, "start"))).second;
     HighResTimingData stop =
             (*pulserMap.find(make_pair(0, "stop"))).second;


     if (start.GetIsValid()&&stop.GetIsValid()&&TACtrace->GetEntries()>0) {
     StartQDC=start.GetTraceQdc();
     StopQDC=stop.GetTraceQdc() ;   
     thetree->Fill();
    }
     TACtrace->Reset();
     StartQDC=-999;
     StopQDC= -999;
     EndProcess();
	return(true);
}
