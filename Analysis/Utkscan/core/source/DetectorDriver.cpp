/*! \file DetectorDriver.cpp
 *   \brief Main driver for event processing
 * \author S. N. Liddick, D. Miller, K. Miernik, S. V. Paulauskas
 * \date July 2, 2007
*/
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "DetectorDriverXmlParser.hpp"
#include "DetectorLibrary.hpp"
#include "Display.h"
#include "EventProcessor.hpp"
#include "Exceptions.hpp"
#include "HighResTimingData.hpp"
#include "RandomInterface.hpp"
#include "RawEvent.hpp"
#include "TraceAnalyzer.hpp"
#include "TreeCorrelator.hpp"

using namespace std;
using namespace dammIds::raw;

DetectorDriver *DetectorDriver::instance = NULL;

DetectorDriver *DetectorDriver::get() {
    if (!instance)
        instance = new DetectorDriver();
    return instance;
}

DetectorDriver::DetectorDriver() : histo(OFFSET, RANGE, "DetectorDriver") {
    eventNumber_ = 0;
    sysrootbool_ = false;
    fillLogic_  = false;
    tapeCycleNum_ = 0;
    lastCycleTime_ = 0;

    #ifdef USE_HRIBF
    // needed for scanor.f sanity checking
    //if dammPlots are off then add this to pass the check
    if(!Globals::get()->GetDammPlots()){
        histo.DeclareHistogram1D(1800,2,"PlaceHolder"); 
    }
    #endif

    try {
        DetectorDriverXmlParser parser;
        parser.ParseNode(this);
        sysrootbool_ = parser.GetRootOutOpt().first;
        rFileSizeGB_ = parser.GetRFileSize();
    } catch (GeneralException &e) {
        /// Any exception in registering plots in Processors
        /// and possible other exceptions in creating Processors
        /// will be intercepted here
        cout << "Exception caught at DetectorDriver::DetectorDriver" << endl;
        cout << "\t" << e.what() << endl;
        throw;
    } catch (GeneralWarning &w) {
        cout << "Warning found at DetectorDriver::DetectorDriver" << endl;
        cout << "\t" << w.what() << endl;
    }

    //adding root stuff

    if (sysrootbool_) {
        for (auto it = vecProcess.begin(); it != vecProcess.end(); it++){
            setProcess.emplace((*it)->GetName());
        }
        if (setProcess.empty()){
            throw GeneralException("Exception:DetectorDriver:: setProcess is empty and root requested. This will cause segfaults on root Fill()");

        }

        Long64_t rFileSizeB_ = rFileSizeGB_ * pow(1000,3);
        std::string name = Globals::get()->GetOutputPath() + Globals::get()->GetOutputFileName() + "_DD.root";
        PixieFile = new TFile(name.c_str(), "RECREATE");
        PTree = new TTree("PixTree", "Pixie Event Tree");
        PTree->SetMaxTreeSize(rFileSizeB_);

        // ROOTFILE system wide header
        //get the current systemTime and make it a string
        time_t now = time(nullptr);
        std::string date = ctime(&now);

        TNamed cfgTNamed("config", Globals::get()->GetConfigFileName());
        TNamed outfTNamed("outputFile", Globals::get()->GetOutputFileName());
        TNamed createTnamed("createTime", date);
        TNamed rootVersionTnamed("RootVersion", gROOT->GetVersion());
        TNamed rootSysTnamed("RootSys", gROOT->GetRootSys().Data());
        TNamed outRootTNamed("outputRootFile", name);

        cfgTNamed.Write();
        outfTNamed.Write();
        createTnamed.Write();
        rootVersionTnamed.Write();
        rootSysTnamed.Write();
        outRootTNamed.Write();

        // new Branch for PixTreeEvent
        PTree->Branch("PixTreeEvent",&pixie_tree_event_);

        // Loop over processor list and do root things, like setting headers
        // NO data is processed here. 
        for (auto itp = setProcess.begin(); itp !=setProcess.end();itp++) {
            GetProcessor((*itp))->SetPixTreeEventPtr(&pixie_tree_event_);

            if ((*itp) == "GammaScintProcessor") {
                //GammaScint Processor Header
                auto GSheader = ((GammaScintProcessor *) GetProcessor("GammaScintProcessor"))->GetTHeader();
                TNamed faciTnamed("facilityType", GSheader.find("FacilityType")->second);
                TNamed bunchTnamed("bunchingTime(sec)", GSheader.find("BunchingTime")->second);
                faciTnamed.Write();
                bunchTnamed.Write();
            } else if ((*itp) == "PspmtProcessor"){
                auto PSPMTheader = ((PspmtProcessor *) GetProcessor("PspmtProcessor"))->GetPSPMTHeader();
                TNamed vdType_("vdType",PSPMTheader.first);
                vdType_.Write();
                TNamed softThresh("softThresh",PSPMTheader.second);
                softThresh.Write();
            } else if ((*itp) == "LogicProcessor") {
                fillLogic_ = true;
            } else{
                continue;
            }
        }

        PTree->SetAutoFlush(3000);
        //ending root stuff
    }
}

DetectorDriver::~DetectorDriver() {
    for (vector<EventProcessor *>::iterator it = vecProcess.begin(); it != vecProcess.end(); it++)
        delete (*it);
    vecProcess.clear();

    for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++)
        delete (*it);
    vecAnalyzer.clear();
    instance = NULL;

    if (sysrootbool_) {
        PixieFile = PTree->GetCurrentFile();
        PixieFile->Write();
        PixieFile->Close();
        delete (PixieFile);
    }
}

void DetectorDriver::Init(RawEvent &rawev) {
    for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++) {
        (*it)->Init();
        (*it)->SetLevel(20);
    }

    for (vector<EventProcessor *>::iterator it = vecProcess.begin(); it != vecProcess.end(); it++)
        (*it)->Init(rawev);

    walk_ = DetectorLibrary::get()->GetWalkCorrections();
    cali_ = DetectorLibrary::get()->GetCalibrations();
}

void DetectorDriver::ProcessEvent(RawEvent &rawev) {
    // Segmentation violation issue workaround 
    // rawev.Size() can be zero. This happens when only one channel is fired in an event and that channel is defined as "ignore"
    if (rawev.Size() == 0) {
	return;
    }
    if (sysrootbool_) {
	pixie_tree_event_.Reset();
    }
    plot(dammIds::raw::D_NUMBER_OF_EVENTS, dammIds::GENERIC_CHANNEL);
    try {
        int innerEvtCounter=0;
        for (vector<ChanEvent *>::const_iterator it = rawev.GetEventList().begin(); it != rawev.GetEventList().end(); ++it) {
            PlotRaw((*it));
            ThreshAndCal((*it), rawev);
            PlotCal((*it));

            //internal TS for the FDSi experiment (Xu)
            if ((*it)->GetChanID().HasTag("its")) {
                pixie_tree_event_.internalTS = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
            }
            string place = (*it)->GetChanID().GetPlaceName();
            if (place == "__9999")
                continue;

            if ((*it)->IsSaturated() || (*it)->IsPileup())
                continue;

            double time = (*it)->GetTime();
            double energy = (*it)->GetCalibratedEnergy();
            int location = (*it)->GetChanID().GetLocation();

            EventData data(time, energy, location);
            TreeCorrelator::get()->place(place)->activate(data);
            if (innerEvtCounter == 0) {
                eventFirstTime_ = (*it)->GetTimeSansCfd(); //sets the time of the first det event in the pixie event
            }
            if ((*it)->GetChanID().HasTag("ets1")) {
                pixie_tree_event_.externalTS1 = (*it)->GetExternalTimeStamp();
            }
            if ((*it)->GetChanID().HasTag("ets2")){
                pixie_tree_event_.externalTS2 = (*it)->GetExternalTimeStamp();
            }
            if(pixie_tree_event_.externalTS2 != 0 && pixie_tree_event_.externalTS1 !=0 ) {
                plot(D_INTERNAL_TS_CHECK, ((pixie_tree_event_.externalTS1 - pixie_tree_event_.externalTS2) + 1000));
            }else {
                plot(D_INTERNAL_TS_CHECK,100);
            }
            innerEvtCounter++;
        }
        if ( eventNumber_ == 0){
            firstEventTime_ = rawev.GetEventList().front()->GetTimeSansCfd();
            firstEventTimeinNs_ = firstEventTime_ * Globals::get()->GetClockInSeconds(rawev.GetEventList().front()->GetChanID().GetModFreq()) * 1.e9;
        }
        //!First round is preprocessing, where process result must be guaranteed
        //!to not to be dependent on results of other Processors.
        for (vector<EventProcessor *>::iterator iProc = vecProcess.begin(); iProc != vecProcess.end(); iProc++)
            if ((*iProc)->HasEvent())
                (*iProc)->PreProcess(rawev);
        ///In the second round the Process is called, which may depend on other
        ///Processors.
        for (vector<EventProcessor *>::iterator iProc = vecProcess.begin(); iProc != vecProcess.end(); iProc++)
            if ((*iProc)->HasEvent())
                (*iProc)->Process(rawev);
        // Clear all places in correlator (if of resetable type)
        for (map<string, Place *>::iterator it = TreeCorrelator::get()->places_.begin();
             it != TreeCorrelator::get()->places_.end(); ++it)
            if ((*it).second->resetable())
                (*it).second->reset();
    } catch (GeneralException &e) {
        /// Any exception in activation of basic places, PreProcess and Process
        /// will be intercepted here
        cout << endl << Display::ErrorStr("Exception caught at DetectorDriver::ProcessEvent") << endl;
        throw;
    } catch (GeneralWarning &w) {
        cout << Display::WarningStr("Warning caught at DetectorDriver::ProcessEvent") << endl;
        cout << "\t" << Display::WarningStr(w.what()) << endl;
    }
    if (sysrootbool_) {
        if(fillLogic_) {
            FillLogicStruc();
        }
        pixie_tree_event_.eventNum = eventNumber_;
        pixie_tree_event_.fileName = Globals::get()->GetOutputFileName();
        PTree->Fill();
    }
    eventNumber_++;

}
/// Declare some of the raw and basic plots that are going to be used in the
/// analysis of the data. These include raw and calibrated energy spectra,
/// information about the run time, and count rates on the detectors. This
/// also calls the DeclarePlots method that is present in all of the
/// Analyzers and Processors.
void DetectorDriver::DeclarePlots() {
    try {
        DeclareHistogram1D(D_HIT_SPECTRUM, S7, "channel hit spectrum");
        DeclareHistogram2D(DD_RUNTIME_SEC, SE, S6, "run time - s");
        DeclareHistogram2D(DD_RUNTIME_MSEC, SE, S7, "run time - ms");
        DeclareHistogram1D(D_INTERNAL_TS_CHECK, SB, "Time Diff between ETS1 and ES2 +1000. 100=BAD");
        DeclareHistogram1D(D_NUMBER_OF_EVENTS, S4, "event counter");
        DeclareHistogram1D(D_HAS_TRACE, S8, "channels with traces");
        DeclareHistogram1D(D_HAS_TRACE_2, S8, "channels w/ traces: post WaveForm Analysis");
        DeclareHistogram1D(D_HAS_TRACE_3, S8, "channels w/ traces: post Fit Analysis");
        DeclareHistogram2D(DD_TRACE_MAX, SD, S8, "Max Value in Trace / 10 vs Chan Num");
        DeclareHistogram1D(D_SUBEVENT_GAP, SE, "Time Between Channels in 10 ns / bin");
        DeclareHistogram1D(D_EVENT_LENGTH, SE, "Event Length in ns");
        DeclareHistogram1D(D_EVENT_GAP, SE, "Time Between Events in ns");
        DeclareHistogram1D(D_EVENT_MULTIPLICITY, S7, "Number of Channels Event");
        DeclareHistogram1D(D_BUFFER_END_TIME, SE, "Buffer Length in ns");

        if (Globals::get()->HasRawHistogramsDefined()) {
            DetectorLibrary *modChan = DetectorLibrary::get();
            
            DetectorLibrary::size_type maxChan = modChan->size();

            for (DetectorLibrary::size_type i = 0; i < maxChan; i++) {
                if (!modChan->HasValue(i))
                    continue;

                stringstream idstr;

                const ChannelConfiguration &id = modChan->at(i);

                idstr << "M" << modChan->ModuleFromIndex(i)
                      << " C" << modChan->ChannelFromIndex(i)
                      << " - " << id.GetType()
                      << ":" << id.GetSubtype()
                      << " L" << id.GetLocation();
                DeclareHistogram1D(D_RAW_ENERGY + i, SE, ("RawE " + idstr.str()).c_str());
                DeclareHistogram1D(D_FILTER_ENERGY + i, SE, ("FilterE " + idstr.str()).c_str());
                DeclareHistogram1D(D_SCALAR + i, SE, ("Scalar " + idstr.str()).c_str());
                if (Globals::get()->GetPixieRevision() == "A")
                    DeclareHistogram1D(D_TIME + i, SE, ("Time " + idstr.str()).c_str());
                DeclareHistogram1D(D_CAL_ENERGY + i, SE, ("CalE " + idstr.str()).c_str());
            }
        }

        for (vector<TraceAnalyzer *>::const_iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++)
            (*it)->DeclarePlots();

        for (vector<EventProcessor *>::const_iterator it = vecProcess.begin(); it != vecProcess.end(); it++)
            (*it)->DeclarePlots();

    } catch (exception &e) {
        cout << Display::ErrorStr("Exception caught at DetectorDriver::DeclarePlots") << endl;
        throw;
    }
}

int DetectorDriver::ThreshAndCal(ChanEvent *chan, RawEvent &rawev) {
    ChannelConfiguration chanCfg = chan->GetChanID();
    int id = chan->GetID();
    string type = chanCfg.GetType();
    string subtype = chanCfg.GetSubtype();
    set<string> tags = chanCfg.GetTags();
    bool hasStartTag = chanCfg.HasTag("start");
    Trace &trace = chan->GetTrace();

    RandomInterface *randoms = RandomInterface::get();

    double energy = 0.0;

    if (type == "ignore" || type == "")
        return (0);

    if (!trace.empty()) {
        plot(D_HAS_TRACE, id);
        
        //!Setting these to false initally so that we can guarante that its false either if we are ignored or if it fails in the analyzers. 
        chan->GetTrace().SetHasValidWaveformAnalysis(false);
        chan->GetTrace().SetHasValidTimingAnalysis(false);

        for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++){
            if (!(*it)->IsIgnoredDetector(chanCfg)){
                (*it)->Analyze(trace, chanCfg);
            }
        }

        if(chan->GetTrace().HasValidWaveformAnalysis()){
            plot(D_HAS_TRACE_2,id);
        }
        if(chan->GetTrace().HasValidTimingAnalysis()){
            plot(D_HAS_TRACE_3,id);
        } else {
            trace.SetPhase(0.0); // if the timing analysis fails for any reason then set the phase to 0
        }
        //We are going to handle the filtered energies here.
        vector<double> filteredEnergies = trace.GetFilteredEnergies();
        if (filteredEnergies.empty()) {
            energy = chan->GetEnergy() + randoms->Generate();
        } else {
            energy = filteredEnergies.front();
            plot(D_FILTER_ENERGY + id, energy);
        }

        //Saves the time in nanoseconds
        chan->SetHighResTime((trace.GetPhase() * Globals::get()->GetAdcClockInSeconds() +
                chan->GetTimeSansCfd() * Globals::get()->GetFilterClockInSeconds()) * 1e9);

        //Plot max Value in trace post trace analysis
        plot(DD_TRACE_MAX,trace.GetMaxInfo().second / 10,id);
    } else {
        /// otherwise, use the Pixie on-board calculated energy and high res
        /// time is zero.
        energy = chan->GetEnergy() + randoms->Generate();
        chan->SetHighResTime(0.0);
    }

    /** Calibrate energy and apply the walk correction. */
    double time, walk_correction;
    if (chan->GetHighResTimeInNs() == 0.0) {
        time = chan->GetTime(); //time is in clock ticks
        walk_correction = walk_->GetCorrection(chanCfg, energy);
    } else {
        time = chan->GetHighResTimeInNs(); //time here is in ns
        walk_correction = walk_->GetCorrection(chanCfg, trace.GetQdc());
    }

    chan->SetCalibratedEnergy(cali_->GetCalEnergy(chanCfg, energy));
    chan->SetWalkCorrectedTime(time - walk_correction);

    //TODO Add group support for GetSummary() 
    rawev.GetSummary(type)->AddEvent(chan);
    DetectorSummary *summary;

    summary = rawev.GetSummary(type + ':' + subtype, false);
    if (summary != NULL)
        summary->AddEvent(chan);

    if (hasStartTag && type != "logic") {
        summary = rawev.GetSummary(type + ':' + subtype + ':' + "start", false);
        if (summary != NULL)
            summary->AddEvent(chan);
    }
    return (1);
}

int DetectorDriver::PlotRaw(const ChanEvent *chan) {
    plot(D_RAW_ENERGY + chan->GetID(), chan->GetEnergy());
    return (0);
}

int DetectorDriver::PlotCal(const ChanEvent *chan) {
    plot(D_CAL_ENERGY + chan->GetID(), chan->GetCalibratedEnergy());
    return (0);
}

EventProcessor *DetectorDriver::GetProcessor(const std::string &name) const {
    for (vector<EventProcessor *>::const_iterator it = vecProcess.begin(); it != vecProcess.end(); it++)
        if ((*it)->GetName() == name)
            return (*it);
    return (NULL);
}

std::set<std::string> DetectorDriver::GetProcessorList() {
   // std::set<std::string> Plist;

    return (setProcess);
}

void DetectorDriver::FillLogicStruc() { //This should be called away from the event loops. (near where it fills the filenames)
//TODO We need to make this sensative to running on something other than a 250MHz, also in the logic processor plotting its self
    double convertTimeNS = Globals::get()->GetClockInSeconds() * 1.0e9; // converstion factor from DSP TICKs to NS
   
    if(TreeCorrelator::get()->checkPlace("Beam")){
        LogStruc.beamStatus = TreeCorrelator::get()->place("Beam")->status();
        if (TreeCorrelator::get()->place("Beam")->status()){
            LogStruc.lastBeamOnTime = TreeCorrelator::get()->place("Beam")->last().time* convertTimeNS;
            LogStruc.lastBeamOffTime = TreeCorrelator::get()->place("Beam")->secondlast().time * convertTimeNS;
        } else {
            LogStruc.lastBeamOnTime = TreeCorrelator::get()->place("Beam")->secondlast().time* convertTimeNS;
            LogStruc.lastBeamOffTime = TreeCorrelator::get()->place("Beam")->last().time * convertTimeNS;
        }
    }

    if(TreeCorrelator::get()->checkPlace("TapeMove")){
        LogStruc.tapeMoving =  TreeCorrelator::get()->place("TapeMove")->status();
        if (TreeCorrelator::get()->place("TapeMove")->status()){
            LogStruc.lastTapeMoveStartTime = TreeCorrelator::get()->place("TapeMove")->last().time* convertTimeNS;
        } else {
            LogStruc.lastTapeMoveStartTime = TreeCorrelator::get()->place("TapeMove")->secondlast().time* convertTimeNS;
        }
    }

    if(TreeCorrelator::get()->checkPlace("Cycle")){
        LogStruc.tapeCycleStatus = TreeCorrelator::get()->place("Cycle")->status();
        if (TreeCorrelator::get()->place("Cycle")->status()){
            double currentTime_ = TreeCorrelator::get()->place("Cycle")->last().time* convertTimeNS;
            if (currentTime_ != lastCycleTime_){
                lastCycleTime_ = currentTime_;
                tapeCycleNum_++;
            }
            LogStruc.lastTapeCycleStartTime = currentTime_;
            LogStruc.cycleNum = tapeCycleNum_;
        } else {
            LogStruc.lastTapeCycleStartTime = TreeCorrelator::get()->place("Cycle")->secondlast().time* convertTimeNS;
            LogStruc.cycleNum = tapeCycleNum_;
        }
    }
    
    if(TreeCorrelator::get()->checkPlace("Protons") && TreeCorrelator::get()->place("Protons")->status()){
        LogStruc.lastProtonPulseTime = TreeCorrelator::get()->place("Protons")->last().time*convertTimeNS;
    }
    
    if (TreeCorrelator::get()->checkPlace("Supercycle") ){
        if (TreeCorrelator::get()->place("Supercycle")->status()){
            LogStruc.lastSuperCycleTime = TreeCorrelator::get()->place("Supercycle")->last().time* convertTimeNS;
        } else {
            LogStruc.lastSuperCycleTime = TreeCorrelator::get()->place("Supercycle")->secondlast().time* convertTimeNS;
        }
    }
    //fill the vector
    pixie_tree_event_.logic_vec_.emplace_back(LogStruc);
}
