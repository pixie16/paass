/** \file ArrayProcessor.cpp
 * \brief A Simple Processor for an Array of Sensl J series SiPMs signals in Pixie. * \author D. Perez-Loureiro
 * \date June 21, 2017
 */

#include <iostream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "ArrayJProcessor.hpp"

#include <TFile.h>
#include <TH1F.h>
#include <TH1I.h>
#include <TTree.h>

TFile *rootOutFile;
TH1F *hArrayJ;
TH1I *ArrayJtrace;
TTree *ttree;
Double_t qdcValue;
namespace dammIds {
    namespace detArrayJ {
        const int D_ENERGY  = 0; //!< ID for the energy of the ArrayJ detector
        const int DD_ArrayJ_VS_PULSER = 1; //!< Energy ArrayJ vs. Energy Pulser
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::detArrayJ;

ArrayJProcessor::ArrayJProcessor():
    EventProcessor(OFFSET, RANGE, "ArrayJProcessor") {
    associatedTypes.insert("ArrayJ");
rootOutFile = new TFile("ArrayJProcessorOut.root","RECREATE");
hArrayJ = new TH1F("hArrayJ","ArrayJ QDC Spectrum ",8192,0,8191);
ArrayJtrace = new TH1I("trace","ArrayJ Traces ",800,0,800);
ttree = new TTree("T","");
ttree->Branch("ArrayJ_Traces",&ArrayJtrace);
 ttree->Branch("QDC",&qdcValue,"QDC/D");
}

ArrayJProcessor::ArrayJProcessor(const double &a):
    EventProcessor(OFFSET, RANGE, "ArrayJProcessor") {
    associatedTypes.insert("ArrayJ");
    a_ = a;
}

ArrayJProcessor::~ArrayJProcessor(){
hArrayJ->Write();
ttree->Write();
rootOutFile->Write();
rootOutFile->Close();
//delete rootFile;
}
void ArrayJProcessor::DeclarePlots(void) {
  DeclareHistogram1D(D_ENERGY, SA, "ArrayJ Energy");
  DeclareHistogram2D(DD_ArrayJ_VS_PULSER, SA, SA,
                       "ArrayJ Energy vs. Pulser Energy");
}

bool ArrayJProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return(false);

    evts_ = event.GetSummary("ArrayJ")->GetList();

    //std::cout<<"ArrayJProcessor::PreProcess()->ArrayJ Size is "<<evts_.size()<<std::endl;

    for(vector<ChanEvent*>::const_iterator it = evts_.begin();
        it != evts_.end(); it++) {
        unsigned int location = (*it)->GetChanID().GetLocation();
	//std::cout<<"Location is "<<location<<std::endl;
        //if(location == 0)
       	for(vector<unsigned int>::const_iterator itA = (*it)->GetTrace().begin();
            itA != (*it)->GetTrace().end(); itA++) {
	  int bin = (int)(itA-(*it)->GetTrace().begin());
	  ArrayJtrace->SetBinContent(bin,*itA);
	  //Only output the 500th trace to make sure that we are not at the
	  // beginning of the file and we're a ways into the data.
	}
         qdcValue=(*it)->GetTrace().GetQdc();
	Int_t Max=(*it)->GetTrace().GetMaxInfo().second;
	//std::cout<<"Maximum is "<<Max<<" at "<<4*MaxBin<<" ns"<<std::endl;
        ttree->Fill();
	ArrayJtrace->Reset();
	hArrayJ->Fill( qdcValue);
}
    return(true);
}

bool ArrayJProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    
    return(true);
}
