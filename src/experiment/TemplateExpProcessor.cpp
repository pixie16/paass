/** \file TemplateExpProcessor.cpp
 * \brief Example class for experiment specific setups
 *\author S. V. Paulauskas
 *\date May 20, 2016
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "GeProcessor.hpp"
#include "GetArguments.hpp"
#include "RawEvent.hpp"
#include "TemplateProcessor.hpp"
#include "TemplateExpProcessor.hpp"
#include "TimingMapBuilder.hpp"

#ifdef useroot
static double tof_;
static double tEnergy;
#endif

namespace dammIds {
    namespace experiment {
        const int D_TSIZE  = 0;
        const int D_GEENERGY  = 1;
        const int DD_TENVSGEN  = 2;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void TemplateExpProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_TSIZE, S3, "Num Template Evts");
    DeclareHistogram1D(D_GEENERGY, SA, "Gamma Energy with Cut");
    DeclareHistogram2D(DD_TENVSGEN, SA, SA, "Template En vs. Ge En");
}

TemplateExpProcessor::TemplateExpProcessor() :
    EventProcessor(OFFSET, RANGE, "TemplateExpProcessor") {
    gCutoff_ = 0.; ///Set the gamma cuttoff energy to a default of 0.
    SetAssociatedTypes();
    ObtainHisName();
    SetupAsciiOutput();
    SetupRootOutput();
}

TemplateExpProcessor::TemplateExpProcessor(const double &gcut) :
    EventProcessor(OFFSET, RANGE, "TemplateExpProcessor") {
    gCutoff_ = gcut;
    SetAssociatedTypes();
    ObtainHisName();
    SetupAsciiOutput();
    SetupRootOutput();
}

///Destructor to close output files and clean up pointers
TemplateExpProcessor::~TemplateExpProcessor() {
    poutstream_->close();
    delete(poutstream_);
#ifdef useroot
    prootfile_->Write();
    prootfile_->Close();
    delete(prootfile_);
#endif
}

///Associates this Experiment Processor with template and ge detector types
void TemplateExpProcessor::SetAssociatedTypes(void) {
    associatedTypes.insert("template");
    associatedTypes.insert("ge");
}

///Sets up the name of the output ascii data file
void TemplateExpProcessor::SetupAsciiOutput(void) {
    stringstream name;
    name << fileName_ << ".dat";
    poutstream_ = new ofstream(name.str().c_str());
}

#ifdef useroot
///Sets up ROOT output file, tree, branches, histograms.
void TemplateExpProcessor::SetupRootOutput(void) {
    stringstream rootname;
    rootname << fileName_ << ".root";
    prootfile_ = new TFile(rootname.str().c_str(),"RECREATE");
    proottree_ = new TTree("data","");
    proottree_->Branch("tof",&tof_,"tof/D");
    proottree_->Branch("ten",&tEnergy,"ten/D");
    ptvsge_ = new TH2D("tvsge","",1000,-100,900,16000,0,16000);
    ptsize_ = new TH1D("tsize","",40,0,40);
}
#endif

///Obtains the name of the histogram file passed via command line
void TemplateExpProcessor::ObtainHisName(void) {
    char hisFileName[32];
    GetArgument(1, hisFileName, 32);
    string temp = hisFileName;
    fileName_ = temp.substr(0, temp.find_first_of(" "));
}

///We do nothing here since we're completely dependent on the resutls of others
bool TemplateExpProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return(false);
    return(true);
}

///Main processing of data of interest
bool TemplateExpProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);

    ///Vectors to hold the information we will get from the various processors
    vector<ChanEvent*> geEvts, tEvts;
    vector<vector<AddBackEvent> > geAddback;

    ///Obtain the list of template events that were created
    ///in TemplateProcessor::PreProecess
    if(event.GetSummary("template")->GetList().size() != 0)
        tEvts = ((TemplateProcessor*)DetectorDriver::get()->
            GetProcessor("TemplateProcessor"))->GetTemplateEvents();

    ///Obtain the list of Ge events and addback events that were created
    ///in GeProcessor::PreProecess
    if(event.GetSummary("ge")->GetList().size() != 0) {
        geEvts = ((GeProcessor*)DetectorDriver::get()->
            GetProcessor("GeProcessor"))->GetGeEvents();
        geAddback = ((GeProcessor*)DetectorDriver::get()->
            GetProcessor("GeProcessor"))->GetAddbackEvents();
    }

    ///Plot the size of the template events vector in two ways
    plot(D_TSIZE, tEvts.size());
#ifdef useroot
    ptsize_->Fill(tEvts.size());
#endif

    ///Obtain some useful logic statuses
    bool isTapeMoving = TreeCorrelator::get()->place("TapeMove")->status();
    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();
    double clockInSeconds = Globals::get()->clockInSeconds();

    ///Begin loop over template events
    for(vector<ChanEvent*>::iterator tit = tEvts.begin();
        tit != tEvts.end(); ++tit) {
        ///Begin loop over ge events
        for (vector<ChanEvent*>::iterator it1 = geEvts.begin();
            it1 != geEvts.end(); ++it1) {
            ChanEvent *chan = *it1;

            double gEnergy = chan->GetCalEnergy();
            double gTime   = chan->GetCorrectedTime();

            ///Plot the Template Energy vs. Ge Energy
            plot(DD_TENVSGEN, gEnergy, (*tit)->GetEnergy());

            ///Output template and ge energy to text file
            *poutstream_ << (*tit)->GetEnergy() << " " << gEnergy << endl;
            ///Fill ROOT histograms and tree with the information
            #ifdef useroot
                ptvsge_->Fill((*tit)->GetEnergy(), gEnergy);
                tEnergy = (*tit)->GetEnergy();
                tof_ = (*tit)->GetTime() - gTime;
                proottree_->Fill();
                tEnergy = tof_ = -9999;
            #endif
            ///Plot the Ge energy with a cut
            if(gEnergy > gCutoff_)
                plot(D_GEENERGY, gEnergy);
        }
    }
    EndProcess();
    return(true);
}
