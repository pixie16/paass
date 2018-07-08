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
    sysrootbool_ =false;

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
            GeneralException("Exception:DetectorDriver:: setProcess is empty and root requested. this will cause segfaults on root fill()");
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
        TNamed RootversionTnamed("RootVersion", gROOT->GetRootSys().Data());
        TNamed outRootTNamed("outputRootFile", name);

        cfgTNamed.Write();
        outfTNamed.Write();
        createTnamed.Write();
        RootversionTnamed.Write();
        outRootTNamed.Write();

        for (auto itp = setProcess.begin(); itp !=setProcess.end();itp++) {

            if ((*itp) == "GammaScintProcessor") {
                PTree->Branch("GSsing", &GSsing);

                //GammaScint Processor Header
                auto GSheader = ((GammaScintProcessor *) GetProcessor("GammaScintProcessor"))->GetTHeader();
                TNamed faciTnamed("facilityType", GSheader.find("FacilityType")->second);
                TNamed bunchTnamed("bunchingTime(sec)", GSheader.find("BunchingTime")->second);
                //including a reminder for which str gscint subtypes map to which num subtype
                //general order is decreasing mass/weight (-1 is the default/Unknown match)
                std::stringstream type2NumType;
                type2NumType << "nai=0 " << " bighag=1 " << " smallhag=2 ";
                TNamed typesTNamed("GS_type->NumType", type2NumType.str().c_str());
                faciTnamed.Write();
                bunchTnamed.Write();
                typesTNamed.Write();

            } else if ((*itp) == "VandleProcessor") {
                PTree->Branch("Vandles", &Vandles);
            } else if ((*itp) == "CloverProcessor") {
                PTree->Branch("Clover",&Csing);
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

    if (sysrootbool_) {
        for (auto itp = setProcess.begin(); itp !=setProcess.end();itp++) {
            if ((*itp) == "GammaScintProcessor") {
                GSsing.clear();
            } else if ((*itp)  == "VandleProcessor") {
                Vandles.clear();
            } else if ((*itp) == "CloverProcessor") {
                Csing.clear();
            } else{
                continue;
            }
        }
    }
    plot(dammIds::raw::D_NUMBER_OF_EVENTS, dammIds::GENERIC_CHANNEL);
    try {
        int innerEvtCounter=0;
        for (vector<ChanEvent *>::const_iterator it = rawev.GetEventList().begin(); it != rawev.GetEventList().end(); ++it) {
            PlotRaw((*it));
            ThreshAndCal((*it), rawev);
            PlotCal((*it));

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
            if (innerEvtCounter == 0){
                eventFirstTime_= (*it)->GetTimeSansCfd(); //sets the time of the first det event in the pixie event
            }
            innerEvtCounter++;
        }
        if ( eventNumber_ == 0){
            firstEventTime_ = rawev.GetEventList().front()->GetTimeSansCfd();
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
    eventNumber_++;
    if (sysrootbool_) {
        for (auto itp = setProcess.begin(); itp !=setProcess.end();itp++) {
            if ((*itp) == "GammaScintProcessor") {
                GSsing = ((GammaScintProcessor *) get()->GetProcessor("GammaScintProcessor"))->GetGSVector();
            } else if ((*itp) == "VandleProcessor") {
                Vandles = ((VandleProcessor *) get()->GetProcessor("VandleProcessor"))->GetVanVector();
            } else if ((*itp) == "CloverProcessor"){
                Csing = ((CloverProcessor * ) get()->GetProcessor("CloverProcessor"))->GetCloverVec();
            } else{
                continue;
            }

        }
        PTree->Fill();
    }
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

        if (Globals::get()->HasRawHistogramsDefined()) {
            DetectorLibrary *modChan = DetectorLibrary::get();
            DeclareHistogram1D(D_NUMBER_OF_EVENTS, S4, "event counter");
            DeclareHistogram1D(D_HAS_TRACE, S8, "channels with traces");
            DeclareHistogram1D(D_SUBEVENT_GAP, SE, "Time Between Channels in 10 ns / bin");
            DeclareHistogram1D(D_EVENT_LENGTH, SE, "Event Length in ns");
            DeclareHistogram1D(D_EVENT_GAP, SE, "Time Between Events in ns");
            DeclareHistogram1D(D_EVENT_MULTIPLICITY, S7, "Number of Channels Event");
            DeclareHistogram1D(D_BUFFER_END_TIME, SE, "Buffer Length in ns");
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

        for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++)
            (*it)->Analyze(trace, chanCfg);

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