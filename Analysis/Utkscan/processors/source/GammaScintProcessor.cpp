/** \file GammaScintProcessor.cpp
 *\brief Processes information for Scintillation Type Gamma Detectors
 *
 *Processes information from scintillation type gamma-ray detectors. This code should function similarly to the clover processor.
 *
 *
 *\author T. T. King
 *\date 21 Dec 2017
 */

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "BarDetector.hpp"
#include "DetectorDriver.hpp"
#include "DoubleBetaProcessor.hpp"
#include "GammaScintProcessor.hpp"
#include "RawEvent.hpp"



namespace dammIds {
    namespace gscint{
        const int LITTLEHAG_OFFSET = 0; //!< offset for 2" hagrid
        const int BIGHAG_OFFSET = 20; //!< offset for 3" hagrid
        const int NaIOFFSET = 40; //!< offset for the Nai:beasts
        const int ADDBACKOFFSET = 100; //!< offset for addback Spectra
        const int GAMMAGAMMAOFFSET = 150;//!< offset for the beta-gamma->gamma plots

        const int D_ENERGY = 0; //!< Energy "Totals" (all det of a single kind in 1 plot)
        const int D_BGENERGY = 1; //!< Event Beta-Gated "Totals"
        const int D_DYENERGY =2 ; //!< Energy from the "dynode" output. This will be used at RIKEN2018 to enable dual gain on HAGRID
        const int D_MBGENERGY = 3; //!< Medium Resolution Beta-Gated "Totals"
        const int D_CYCLEENERGY = 4; //!< Totals anti-gated on TapeMove
        const int DD_MULTI = 5; //!< Multiplicy (normal, betaGated)
        const int DD_LONGTIME_ENERGY = 6 ; //!< Energy vs Time (Bunched) (Used for Tracking Drift over Long Times)
        const int DD_TIME_ENERGY = 7; //!< Energy vs Short Time (only uses the first Scale)
        const int DD_BGTIME_ENERGY = 8; //!< Beta Gated Energy vs Time (Used for prompt vs delayed gammas)

    }
}//namespace dammIds

using namespace std;
using namespace dammIds::gscint;

GammaScintProcessor::~GammaScintProcessor()=default;

void GammaScintProcessor::DeclarePlots() {
    for(auto it= typeList_.begin(); it != typeList_.end(); it++) {
        unsigned int offset = ReturnOffset((*it));
        if (offset == numeric_limits<unsigned int>::max()) {
            throw GeneralException("UNKNOWN Gamma Detector Type. Known Types are nai, smallhag, bighag");
        }

        const string typeName = (*it);
        stringstream ss;

        ss << typeName << "Energy (totals)";
        DeclareHistogram1D(D_ENERGY + offset, SD, ss.str().c_str());

        ss.str("");
        ss << typeName << " Beta-Gated Energy";
        DeclareHistogram1D(D_BGENERGY + offset, SE, ss.str().c_str());

        ss.str("");
        ss << typeName << "Energy from Dynode (totals)";
        DeclareHistogram1D(D_DYENERGY + offset, SD, ss.str().c_str());

        ss.str("");
        ss<< typeName << "Addback Energy";
        DeclareHistogram1D(D_ENERGY + offset + ADDBACKOFFSET, SD, ss.str().c_str());

        ss.str("");
        ss<< typeName << "BG Addback Energy";
        DeclareHistogram1D(D_BGENERGY + offset + ADDBACKOFFSET, SD, ss.str().c_str());

        ss.str("");
        ss << typeName << " Med Res Beta-Gated Energy (" << MRBetaWindow_.second << "ns)";
        DeclareHistogram1D(D_MBGENERGY + offset, SE, ss.str().c_str());

        if (ISOL_){
            ss.str("");
            ss << typeName << " Energy (Anti-Tape Move)";
            DeclareHistogram1D(D_CYCLEENERGY + offset, SE, ss.str().c_str());
        }

        ss.str("");
        ss<<typeName << " Multiplicity (Normal,BetaGated)";
        DeclareHistogram2D(DD_MULTI + offset,S6,S3,ss.str().c_str());


        ss.str("");
        ss<<typeName << " <E> vs T (Drift Tracker) " << FacilType_;
        DeclareHistogram2D(DD_LONGTIME_ENERGY + offset,SE,SB,ss.str().c_str());
        // SB: this gives 2048 cycles for ISOL and a bit more than 16 hours (with 30 sec bunches) for fragmentation

        if (EvsT_ && Globals::get()->GetDammPlots()) {
            //this bool is here because these are LARGE (with binDepth = 2) his, they add ~1GB to the .his file size, for the SINGLE DEFAULT
            //time scale. default parse is True
            ss.str("");
            ss << typeName << " <E> vs Short T";
            DeclareHistogramTimeY(DD_TIME_ENERGY + offset, SD, SD, ss.str().c_str(), binDepth, timeScales_.front(), "s");

            ss.str("");
            ss << typeName << " BG <E> vs Short Time";
            DeclareHistogramTimeY(DD_BGTIME_ENERGY + offset, SD, SD, ss.str().c_str(), binDepth, timeScales_, "s");

        }
        if (BetaGammGamm) {
            Messenger m;
            m.detail("Loading Beta Gamma-gamma Plots from GammaScintProcessor", 1);
            /**@todo add beta gamma gamma plots
             */
        }
    }
}


GammaScintProcessor::GammaScintProcessor(const std::map<std::string,std::string> &GSArgs,
                                         const std::vector<std::string> &DetTypes,
                                         const std::vector<std::string> &TimeScales)
        :EventProcessor(OFFSET,RANGE, "GammaScintProcessor"){

    Theader = GSArgs;
    associatedTypes.insert("gscint");

    ISOL_=false;
    FacilType_ = GSArgs.find("FacilityType")->second;
    if (FacilType_ =="ISOL")
        ISOL_ = true;

    typeList_= set<string>(DetTypes.begin(),DetTypes.end());
    EvsT_ = StringManipulation::StringToBool(GSArgs.find("EnergyVsTime")->second);


    if (ISOL_){
        BunchingTimestr_ = "Tape_Cycle";
        bunchingTime_ = (double) 0.0;
    }else {
        BunchingTimestr_ = GSArgs.find("BunchingTime")->second;
        bunchingTime_ = strtod(BunchingTimestr_.c_str(), nullptr);
    }

    SysRoot_ =  StringManipulation::StringToBool(GSArgs.find("GSroot")->second);

    if (!TimeScales.empty()) {
        if (TimeScales.size() > MAXTIMEPLOTS)
            throw GeneralException("Requested more Time Scale plots than allowed. "
                                           "Check GammaScintProcessor.hpp for details");
        timeScales_.emplace_back(10e-3);
        //Robert and Miguel agreed that 10ms should A) be default, and B) should always be present if we ask for these.
        for (auto it = TimeScales.begin(); it != TimeScales.end(); it++) {
            stringstream ss;
            ss << (*it) << "e-3";
            timeScales_.emplace_back(strtof(ss.str().c_str(), nullptr));
        }
    }else {
        timeScales_.emplace_back(10e-3);
    }
    MRBetaWindow_.second = GSArgs.find("MRBWin")->second;
    MRBetaWindow_.first=strtod(MRBetaWindow_.second.c_str(), nullptr)*Globals::get()->GetClockInSeconds() *1.e9;

    //Loads addback thresholds and Sub Event Windows (parsed in DetectorDriverXmlParser, with defaults)
    // initializing  Addback Parameter maps
    // map structure <subtype , <parameter , value > >
    // Parameters are "thresh" , "subEvtWin", "refTime".
    // ref times need to be in ns because the Gtime is in ns

    double NgammaThreshold_ =  strtod(GSArgs.find("NaI_Thresh")->second.c_str(), nullptr);
    double NsubEventWin_ =  strtod(GSArgs.find("NaI_SubWin")->second.c_str(), nullptr)/(Globals::get()->GetClockInSeconds());
    double LHgammaThreshold_ =  strtod(GSArgs.find("LH_Thresh")->second.c_str(), nullptr);
    double LHsubEventWin_ =  strtod(GSArgs.find("LH_SubWin")->second.c_str(), nullptr)/(Globals::get()->GetClockInSeconds());
    double BHgammaThreshold_ =  strtod(GSArgs.find("BH_Thresh")->second.c_str(), nullptr);
    double BHsubEventWin_ =  strtod(GSArgs.find("BH_SubWin")->second.c_str(), nullptr)/(Globals::get()->GetClockInSeconds());

    std::map <std::string,double > paraData ;
    paraData.insert(make_pair("thresh",NgammaThreshold_));
    paraData.insert(make_pair("subEvtWin",NsubEventWin_));
    paraData.insert(make_pair("refTime",-2.0*NsubEventWin_));
    ParameterMap.emplace("nai",paraData);
    paraData.clear();

    paraData.insert(make_pair("thresh",LHgammaThreshold_));
    paraData.insert(make_pair("subEvtWin",LHsubEventWin_));
    paraData.insert(make_pair("refTime",-2.0*LHsubEventWin_));
    ParameterMap.emplace("smallhag",paraData);
    paraData.clear();

    paraData.insert(make_pair("thresh",BHgammaThreshold_));
    paraData.insert(make_pair("subEvtWin",BHsubEventWin_));
    paraData.insert(make_pair("refTime",-2.0*BHsubEventWin_));
    ParameterMap.emplace("bighag",paraData);
    paraData.clear();

    // initalize addback event structs
    LHaddBack_ = new GSAddback(0.,0.,0.,0);
    NaddBack_ = new GSAddback(0.,0.,0.,0);
    BHaddBack_ = new GSAddback(0.,0.,0.,0);
    FAILEDaddback_ = new GSAddback(numeric_limits<double>::max(),numeric_limits<double>::max(),
                                   numeric_limits<double>::max(),numeric_limits<unsigned>::max());


}

bool GammaScintProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);

    /** From the Detector Driver we get the Pixie Event number and the system-wide first event time
     * We get the event list for the "gscint" type.
     */
    firstGSEvent_ = false;
    if (evtNum_ == 0){
        firstEventTime_ = DetectorDriver::get()->GetFirstEventTimeinNs();
        bunchLast_ = firstEventTime_;
        firstGSEvent_= true;
    }
    evtNum_ = DetectorDriver::get()->GetEventNumber();
    GSEvents_ = event.GetSummary("gscint")->GetList();

    return (true);
}


bool GammaScintProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);

    hasLowResBeta_ = false;
    hasMedResBeta_ = false;


    if (!event.GetSummary("beta")->GetList().empty()) { //get pixie event level beta for setups with a "beta" type
        hasLowResBeta_ = TreeCorrelator::get()->place("Beta")->status();
        /**@todo Get beta list for beta:single (leribbs style) setups, but since we don't run much like this anymore its probably
        * not super urgent */
        if (!event.GetSummary("beta:double")->GetList().empty()) {
            //also get the Low Res Bars for medium res beta gating.
            lrtBetas = ((DoubleBetaProcessor *) DetectorDriver::get()->GetProcessor("DoubleBetaProcessor"))->GetLowResBars();
            //Loop
            BetaList.clear();
            for (auto bIt = lrtBetas.begin(); bIt != lrtBetas.end(); bIt++) {
                double betaTime = bIt->second.first;
                double betaEn = bIt->second.second;
                BetaList.emplace_back(make_pair(betaTime, betaEn));
            }
        }
    } else {
        ///@todo Get fragmentation style (pspsmt) "beta" for medium resolution beta gating
    }


    /** Which kind of bunching to use depends on the Facility. ISOL has access to the Tape Cycle and Time;
    * while Fragmentation only has the time bunching (which is why it is the default FacilityType)
    */
    if (ISOL_) {
        if (TreeCorrelator::get()->place("Cycle")->status()) {
            double currentTime_ = TreeCorrelator::get()->place("Cycle")->last().time;
            currentTime_ *= Globals::get()->GetClockInSeconds() * 1.e9;
            if (currentTime_ != bunchLast_) {
                double tdiff = (currentTime_ - bunchLast_) / 1.e6;
                if (bunchNum_ == 0) {
                    cout
                            << " #  There are some events at the beginning of the first segment missing from Histograms that use cycleNum."
                            << endl << " #  This is a product of not starting the cycle After the LDF." << endl
                            << " #  This First TDIFF is most likely nonsense" << endl;
                }
                bunchLast_ = currentTime_;
                bunchNum_++;
                cout << endl << "Cycle Change " << endl << "Tdiff (Cycle start and Now) (ms)= " << tdiff << endl
                     << "Starting on Cycle #" << bunchNum_ << endl;
            }
        }
    } else {
        double currentTime_ = GSEvents_.back()->GetTimeSansCfd() * Globals::get()->GetClockInSeconds() * 1.e9;
        double tdiff = (currentTime_ - bunchLast_);
        //cout <<"bunchLast_ = "<<bunchLast_<<endl<<"Tdiff from First to current = " << currentTime_ - firstEventTime_<<endl;
        if (firstGSEvent_)
            cout << "First Bunch. Current Bunch Size is " << bunchingTime_ << " seconds." << endl;

        if (tdiff > (bunchingTime_ * 1.e9)) { //not working right now
            bunchLast_ = currentTime_;
            bunchNum_++;
            cout << endl << "Bunch Change" << endl << "Now Starting Bunch # " << bunchNum_ << "." << endl;
        };

    }

    //Plotting Multiplicities (BetaGated might be useful for Hagrids (LaBr) due to the internal activity
    //also adding the multiplicities to root
    for (auto subTypeList_ = typeList_.begin(); subTypeList_ != typeList_.end(); subTypeList_++) {
        string TYPE = "gscint:";
        const vector<ChanEvent *> &subEvent = event.GetSummary(TYPE + (*subTypeList_))->GetList();
        unsigned int offset = ReturnOffset((*subTypeList_));
        plot(DD_MULTI + offset, subEvent.size(), 0);
        if (hasLowResBeta_)
            plot(DD_MULTI + offset, subEvent.size(), 1);
    } // end SubtypeList for loop

    //start actual event loop
    for (auto it = GSEvents_.begin(); it != GSEvents_.end(); it++) {
        hasMedResBeta_=false;
        string subType = (*it)->GetChanID().GetSubtype();
        unsigned int subTypeOffset = ReturnOffset(subType);
        double Genergy = (*it)->GetCalibratedEnergy();
        double Gtime = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds() * 1.e9;
        double decayTime = (Gtime - bunchLast_) / 1.0e9;

        if (hasLowResBeta_) {
            //loop the the "beta" events to find the current gamma event's closest beta event
            //For the ORNL2016 data the beta multiplicy is mostly 1 so this is here for completeness and
            //the possiblity of a future need.

            for (auto itB = BetaList.begin(); itB != BetaList.end(); itB++) {
                double BetaTime =(*itB).first * Globals::get()->GetClockInSeconds() * 1.e9;
                double MRBtDiff = Gtime - BetaTime ;
                if (MRBtDiff < 0)
                    continue;
                else if (MRBtDiff >= 0)   {
                    hasMedResBeta_ = true;
                    break;
                }
            }// end BetaList Loop
        }//end if medium resolution beta gating loop.

        //DAMM PLOTS: Only plot things like drift BG etc if not a dynode signal.
        if (!((*it)->GetChanID().HasTag("dy"))) {
            plot(D_ENERGY + subTypeOffset, Genergy);
            plot(DD_LONGTIME_ENERGY + subTypeOffset, Genergy, bunchNum_);
            plot(DD_TIME_ENERGY + subTypeOffset, Genergy, decayTime / timeScales_.front());
            if (hasLowResBeta_) {
                plot(D_BGENERGY + subTypeOffset, Genergy);
                timePloty(DD_BGTIME_ENERGY + subTypeOffset, Genergy, decayTime, timeScales_);
                if (hasMedResBeta_)
                    plot(D_MBGENERGY + subTypeOffset, Genergy);
            }
        } else {
            plot(D_DYENERGY + subTypeOffset, Genergy);
        }

        if (SysRoot_) {
            if ((*it)->GetChanID().HasTag("dy"))
                GSstruct.isDynodeOut = true;
            GSstruct.group = (*it)->GetChanID().GetGroup();
            GSstruct.subtype = (*it)->GetChanID().GetSubtype() ;

            GSstruct.energy = Genergy;
            GSstruct.rawEnergy = (*it)->GetEnergy();
            GSstruct.time = Gtime;
            GSstruct.detNum = (*it)->GetChanID().GetLocation();

            if (!(*it)->GetTrace().empty() && (*it)->GetTrace().HasValidWaveformAnalysis()) {
                GSstruct.qdc = (*it)->GetTrace().GetQdc();
            }
            pixie_tree_event_->gamma_scint_vec_.emplace_back(GSstruct);
            GSstruct = processor_struct::GAMMASCINT_DEFAULT_STRUCT; //reset structure
            //Dont fill because we want 1 pixie event per tree entry, so we add the current structure in the last spot
            //on a vector<> and then reset the structure. and we will at the end or Process()
        } //end sysroot_

       //Starting Rough (TypeWide) Addback
    // (TYPEWIDE ADDBACK MUST BE THE LAST THING IN THE FOR LOOP due to the energy check below)

    if (Genergy < (GetAddbackPara(subType, "thresh")) || ((*it)->GetChanID().HasTag("dy")) ) { ;
        continue;
    }
    //double abTdiff = abs(Gtime - (GetAddbackPara(subType, "refTime")));
    double abTdiff = abs((*it)->GetTimeSansCfd()-GetAddbackPara(subType,"refTime"));

    if (abTdiff > (GetAddbackPara(subType, "subEvtWin"))) {
        //if we are outside of the sub event window for a given subtype
        // then fill the tree then zero the correct struc.

        plot(D_ENERGY + subTypeOffset + ADDBACKOFFSET, GetAddbackStruct(subType)->energy);
        if (hasLowResBeta_) {
            plot(D_BGENERGY + subTypeOffset + ADDBACKOFFSET, GetAddbackStruct(subType)->energy);
        }
        //reset the correct addback struct to 0s
        (*GetAddbackStruct(subType)) = GSAddback(0.,0.,0.,0);


    }
    if (GetAddbackStruct(subType)->multiplicity == 0) {
        //only count as "beta gated" if the first addback event has a beta. (a catch incase the addback event
            //crosses pixie events, which it shouldn't because process is closed at an event boundary )
        GetAddbackStruct(subType)->ftime = (*it)->GetTimeSansCfd();
    }
        //Gets last entry in the addback vector for the correct subtype, and increments it with the current values
        GetAddbackStruct(subType)->energy += (*it)->GetCalibratedEnergy();
        GetAddbackStruct(subType)->time = (*it)->GetTimeSansCfd();
        GetAddbackStruct(subType)->multiplicity += 1;

        SetAddbackRefTime(subType, (*it)->GetTimeSansCfd());
    } //End GSEvents for loop

    //now that we have processed every det event in the Pixie Event list. We wait for the DD to ask for the vector
    //the vector is cleared at the beginning of Process() so we dont need to do it here.

    EndProcess();
    return (true);
}


unsigned int GammaScintProcessor::ReturnOffset(const std::string &subtype) {
    if (subtype == "nai")
        return (NaIOFFSET);
    if (subtype == "bighag")
        return (BIGHAG_OFFSET);
    if (subtype == "smallhag")
        return (LITTLEHAG_OFFSET);

    return (numeric_limits<unsigned int>::max());
}

void GammaScintProcessor::SetAddbackRefTime(const std::string &subtype, const double &newRefTime) {
    auto ito = ParameterMap.find(subtype);
    if (ito == ParameterMap.end()) {
        stringstream ss;
        ss<<"Error in looking up Addback Parameters for "<<subtype <<" (Unknown Type)";
        throw GeneralException(ss.str());
    }else{
        auto iti = ito->second.find("refTime");
        if (iti == ParameterMap.find(subtype)->second.end()) {
            stringstream ss;
            ss << "Error in looking up Addback Reference Time for " << subtype << " (Unknown Parameter)";
            throw GeneralException(ss.str());
        }
        iti->second = newRefTime;
    }
}

double GammaScintProcessor::GetAddbackPara(const std::string &subtype, const std::string &option) {
    auto ito = ParameterMap.find(subtype);
    if (ito == ParameterMap.end()) {
        stringstream ss;
        ss<<"Error in looking up Addback Parameters for "<<subtype <<" (Unknown Type)";
        throw GeneralException(ss.str());
    }else {
        auto iti = ito->second.find(option);
        if (iti == ito->second.end()) {
            stringstream ss;
            ss << "Error in looking up Addback Parameter for " << subtype << " (Unknown Parameter)";
            throw GeneralException(ss.str());
        }
        return iti->second;
    }
}

GSAddback* GammaScintProcessor::GetAddbackStruct(const std::string &subtype) {
    if (subtype == "nai")
        return (NaddBack_);
    else if (subtype == "bighag")
        return (BHaddBack_);
    else if (subtype == "smallhag")
        return (LHaddBack_);
    else
        return (FAILEDaddback_);
}

/**
 * Plot to multiple in "cycle" spectrums with different time scales (based on the size() of the timescale vector)
 * (like grow and decay curves)
 */
void GammaScintProcessor::timePloty(int dammId, double x, double y,
                                    const vector<float> &timeScale) {
    for (unsigned int i = 0; i < timeScale.size(); i++) {
        plot(dammId + i, x, y / timeScale[i]);
    }
}

void GammaScintProcessor::DeclareHistogramTimeY(int dammId, int xsize, int ysize,const char *title, int halfWordsPerChan,
                                            const vector<float > &timeScale, const char *units) {
    stringstream fullTitle;
    for (unsigned int i = 0; i < timeScale.size(); i++) {
        fullTitle << title << " (" << timeScale[i] << " " << units << "/bin)";
        histo.DeclareHistogram2D(dammId + i, xsize, ysize, fullTitle.str().c_str(),halfWordsPerChan, 1, 1);
        fullTitle.str("");
    }
}
void GammaScintProcessor::DeclareHistogramTimeY(int dammId, int xsize, int ysize, const char *title, int halfWordsPerChan,
                                                const float  &timeScale, const char *units) {
    stringstream fullTitle;
    fullTitle << title << " (" << timeScale << " " << units << "/bin)";
    histo.DeclareHistogram2D(dammId, xsize, ysize, fullTitle.str().c_str(), halfWordsPerChan, 1, 1);
    fullTitle.str("");
}



