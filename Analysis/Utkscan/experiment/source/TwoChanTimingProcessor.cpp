/** \file TwoChanTimingProcessor.cpp
 * \brief Analyzes data from a simple Two channel Timing setup
 * \author D. P. Loureiro
 * \date June 1, 2017
 */
#include <fstream>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "HighResTimingData.hpp"
#include "TwoChanTimingProcessor.hpp"

#include <TFile.h>
#include <TTree.h>
#include <TH1I.h>
#include <TH2I.h>

static HighResTimingData::HrtRoot rstart1;
static HighResTimingData::HrtRoot rstop1;
static HighResTimingData::HrtRoot rstart2;
static HighResTimingData::HrtRoot rstop2;

std::ofstream trcfile;
TFile *rootfile;
TTree *tree;
TH1I *codes;
TH2I *traces;
TH2I *traces_stop;
TH2I *superpulse_start;
TH2I *superpulse_stop;
enum CODES {
    PROCESS_CALLED,
    WRONG_NUM
};

namespace dammIds {
    namespace experiment {
    }
}

using namespace std;
using namespace dammIds::experiment;

TwoChanTimingProcessor::TwoChanTimingProcessor() :
        EventProcessor(OFFSET, RANGE, "TwoChanTimingProcessor") {
    associatedTypes.insert("pulser");
    associatedTypes.insert("beta");

      std::cout<<"TwoChanTimingProcessor::TwoChanTimingProcessor()"<<std::endl;


    trcfile.open(Globals::get()->outputPath("trace.dat").c_str());

    stringstream rootname;

    rootname << Globals::get()->outputPath(Globals::get()->outputFile())
             << ".root";
    rootfile = new TFile(rootname.str().c_str(),"RECREATE");


    tree = new TTree("timing", "");
    tree->Branch("start1", &rstart1,"qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    tree->Branch("stop1", &rstop1, "qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    tree->Branch("start2", &rstart2,"qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    tree->Branch("stop2", &rstop2, "qdc/D:time:snr:wtime:phase:abase:sbase:id/b");
    //tree->Branch("StartTimeStamp",&StartTimeStamp,"Timestamp/D");
    //tree->Branch("StopTimeStamp",&StopTimeStamp,"Timestamp/D");
    tree->Branch("StartChiSq",&StartChiSq,"StartChi/D");
    tree->Branch("StopChiSq",&StopChiSq,"StopChi/D");
    codes = new TH1I("codes", "", 40, 0, 40);
    traces = new TH2I("traces","",1000,0,1000,20000,0,20000);
    traces_stop = new TH2I("traces_stop","",1000,0,1000,20000,0,20000);
    superpulse_start =  new TH2I("super_start","",1000,0,1000,4096,0,4095);
    superpulse_stop =  new TH2I("super_stop","",1000,0,1000,4096,0,4095);
    StartTimeStamp=-9999;
    StopTimeStamp=-9999;
;
}

TwoChanTimingProcessor::~TwoChanTimingProcessor() {
          std::cout<<"TwoChanTimingProcessor::~TwoChanTimingProcessor()"<<std::endl;

    codes->Write();
    rootfile->Write();
    rootfile->Close();
    trcfile.close();
}

bool TwoChanTimingProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;
      //std::cout<<"TwoChanTimingProcessor::Process()"<<std::endl;

    //Define a map to hold the information
    TimingMap pulserMap;

    //plot the number of times we called the function
    codes->Fill(PROCESS_CALLED);

    static const vector<ChanEvent *> &pulserEvents =
            event.GetSummary("pulser")->GetList();
            //event.GetSummary("beta")->GetList();

    //if(pulserEvents.size()==4)
      //std::cout<<"TwoChanTimingProcessor::Process()"<<pulserEvents.size()<<std::endl;

    for (vector<ChanEvent *>::const_iterator itPulser = pulserEvents.begin();
         itPulser != pulserEvents.end(); itPulser++) {
        int location = (*itPulser)->GetChanID().GetLocation();
        string subType = (*itPulser)->GetChanID().GetSubtype();
		//std::cout<<"TwoChanTimingProcessor::Process()"<<location<<" "<<subType<<std::endl;

		TimingDefs::TimingIdentifier key(location, subType);
		pulserMap.insert(make_pair(key, HighResTimingData(*(*itPulser))));
	    }

    //if (pulserMap.empty()) {
    if (pulserMap.empty() || pulserMap.size() % 2 != 0) {
    //if (pulserMap.empty() || pulserMap.size() != 3) {
        //If the map is empty or size isn't even we return and increment
        // error code
        codes->Fill(WRONG_NUM);
        EndProcess();
        return false;
    }

HighResTimingData start1 =   (*pulserMap.find(make_pair(0, "start1"))).second;
HighResTimingData stop1 =    (*pulserMap.find(make_pair(0, "stop1"))).second;
HighResTimingData start2 =   (*pulserMap.find(make_pair(0, "start2"))).second;
HighResTimingData stop2 =    (*pulserMap.find(make_pair(0, "stop2"))).second;

//cout<<start1.GetIsValid()<<endl;
    //std::cout<<"Start Phase is "<<start1.GetTrace().GetPhase()*Globals::get()->adcClockInSeconds()*1e9<<" ns"<<std::endl;
    //std::cout<<"Stop Phase is "<<stop1.GetTrace().GetPhase()*Globals::get()->adcClockInSeconds()*1e9<<" ns"<<std::endl;
    //std::cout<<"Start Maximum is "<<start1.GetTrace().GetMaxInfo().second<<" at bin "<<start1.GetTrace().GetMaxInfo().first<<std::endl;
    //std::cout<<"Stop Maximum is "<<stop1.GetTrace().GetMaxInfo().second<<" at bin "<<stop1.GetTrace().GetMaxInfo().first<<std::endl;
    //std::cout<<"Range is "<<Globals::get()->waveformRange("pulser:stop").first<<" and "<< Globals::get()->waveformRange("pulser:stop").second<<std::endl;

if(start1.GetIsValid()){
    Double_t Start_time = (start1.GetTimeSansCfd()*Globals::get()->filterClockInSeconds()+
			   start1.GetTrace().GetPhase()*Globals::get()->adcClockInSeconds())*1.e9;
   StartMaxValue = start1.GetMaximumValue();
   StartChiSq = start1.GetTrace().GetChiSquareDof();
}
if(stop1.GetIsValid()){   
 Double_t Stop_time = (stop1.GetTimeSansCfd()*Globals::get()->filterClockInSeconds()+
			   stop1.GetTrace().GetPhase()*Globals::get()->adcClockInSeconds())*1.e9;

    StopMaxValue = stop1.GetMaximumValue();
    StopChiSq = stop1.GetTrace().GetChiSquareDof();    
 
}
    //std::cout<<"Start Max is "<< StartMaxValue<<std::endl;
    //std::cout<<"Stop Max is "<< StopMaxValue<<std::endl;

    static int trcCounter = 0;
    int bin;
if (start1.GetIsValid() && stop1.GetIsValid()) {
    for(vector<unsigned int>::const_iterator it = start1.GetTrace().begin();
            it != start1.GetTrace().end(); it++) {
        bin = (int)(it-start1.GetTrace().begin());
        traces->Fill(bin, trcCounter, *it);
    	superpulse_start->Fill(bin,*it);
        //Only output the 500th trace to make sure that we are not at the
        // beginning of the file and we're a ways into the data.
        if(trcCounter == 500)
            trcfile << bin << " " << *it << " " << sqrt(*it) << endl;
    }
    for(vector<unsigned int>::const_iterator it = stop1.GetTrace().begin();
            it != stop1.GetTrace().end(); it++) {
        bin = (int)(it-stop1.GetTrace().begin());
        traces_stop->Fill(bin, trcCounter, *it);
    //	superpulse_stop->Fill(bin,*it);



    }
    trcCounter++;
    //tree->Fill();
}

    //We only plot and analyze the data if the data is validated
    if (start1.GetIsValid() && stop1.GetIsValid()) { //DPL: Should be &&!!
        start1.FillRootStructure(rstart1);
      	stop1.FillRootStructure(rstop1);
    }


//    if (start2.GetIsValid() && stop2.GetIsValid()) { //DPL: Should be &&!!
//        start2.FillRootStructure(rstart2);
//      	stop2.FillRootStructure(rstop2);
//    }

 //if (start1.GetIsValid() && stop1.GetIsValid()&&start2.GetIsValid() && stop2.GetIsValid()) 
 if (start1.GetIsValid() && stop1.GetIsValid()) 
   tree->Fill();


    if (start1.GetIsValid() && stop1.GetIsValid()) {
      stop1.ZeroRootStructure(rstop1);
      start1.ZeroRootStructure(rstart1);
    }

    //if (start2.GetIsValid() && stop2.GetIsValid()) {
    //  stop2.ZeroRootStructure(rstop2);
    //  start2.ZeroRootStructure(rstart2);
    //}

	//start2.FillRootStructure(rstart2);
        //stop2.FillRootStructure(rstop2);
        //stop2.ZeroRootStructure(rstop2);
        //start2.ZeroRootStructure(rstart2);
 
        StartTimeStamp=-9999;
        StopTimeStamp=-9999;
        StartMaxValue = -9999;
        StopMaxValue = -9999;
        StartChiSq = -9999;    
        StopChiSq = -9999;    

    EndProcess();
    //std::cout<<"********************************"<<std::endl;
    return true;
}
