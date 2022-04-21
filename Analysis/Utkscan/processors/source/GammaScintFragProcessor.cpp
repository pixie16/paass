/** \file GammaScintFragProcessor.cpp
 *\brief Processes information for Scintillation Type Gamma Detectors
 *
 *Processes information from scintillation type gamma-ray detectors. This code is intended to be a simpler version of the GammaScintFragProcessor, geared as a faster way to add these det types to the analysis root output. As such there is very little damm plotting; just the minimum needed to check that things are working right (total, BG total, DY total, DY BG total, multi, Long time scale drift tracking)
 *
 *
 *\author T. T. King
 *\date 22 Oct 2019
 */

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "BarDetector.hpp"
#include "DetectorDriver.hpp"
#include "DoubleBetaProcessor.hpp"
#include "GammaScintFragProcessor.hpp"
#include "RawEvent.hpp"

namespace dammIds {
namespace gscint {
const int LITTLEHAG_OFFSET = 0;  //!< offset for 2" hagrid
const int BIGHAG_OFFSET = 20;    //!< offset for 3" hagrid
const int NaIOFFSET = 40;        //!< offset for the large volume Nai

const int D_ENERGY = 0;            //!< Energy "Totals" (all det of a single kind in 1 plot)
const int D_BG_ENERGY = 1;         //!< Event Beta-Gated "Totals"
const int D_DY_ENERGY = 2;         //!< Energy from the "dynode" output. This will be used at RIKEN2018 to enable dual gain on HAGRID
const int D_DY_BG_ENERGY = 3;      //!< BG Energy from the "dynode" output. This will be used at RIKEN2018 to enable dual gain on HAGRID
const int DD_MULTI = 5;            //!< Multiplicy (normal, betaGated)
const int DD_LONGTIME_ENERGY = 6;  //!< Energy vs Time (Bunched) (Used for Tracking Drift over Long Times)

}  // namespace gscint
}  //namespace dammIds

using namespace std;
using namespace dammIds::gscint;

GammaScintFragProcessor::~GammaScintFragProcessor() = default;

void GammaScintFragProcessor::DeclarePlots() {
    for (auto it = typeList_.begin(); it != typeList_.end(); it++) {
        unsigned int offset = ReturnOffset((*it));
        if (offset == numeric_limits<unsigned int>::max()) {
            throw GeneralException("UNKNOWN Gamma Detector Type. Known Types are nai, smallhag, bighag");
        }

        const string typeName = (*it);
        stringstream ss;

        ss << typeName << "Energy (totals)";
        DeclareHistogram1D(D_ENERGY + offset, SD, ss.str().c_str());

        ss.str("");
        ss << typeName << " Beta-Gated:: Energy (totals)";
        DeclareHistogram1D(D_BG_ENERGY + offset, SE, ss.str().c_str());

        ss.str("");
        ss << typeName << "Dynode Energy (totals)";
        DeclareHistogram1D(D_DY_ENERGY + offset, SD, ss.str().c_str());

        ss.str("");
        ss << typeName << "Beta-Gated::Dynode Energy (totals)";
        DeclareHistogram1D(D_DY_BG_ENERGY + offset, SD, ss.str().c_str());

        ss.str("");
        ss << typeName << " Multiplicity (Normal,BetaGated)";
        DeclareHistogram2D(DD_MULTI + offset, S6, S3, ss.str().c_str());

        ss.str("");
        ss << typeName << " <E> vs T (Drift Tracker)";
        DeclareHistogram2D(DD_LONGTIME_ENERGY + offset, SE, SB, ss.str().c_str());
        // SB: this gives 2048 cycles for ISOL and a bit more than 16 hours (with 30 sec bunches) for fragmentation
    }
}

GammaScintFragProcessor::GammaScintFragProcessor(const std::map<std::string, std::string> &GSArgs,
                                                 const std::vector<std::string> &DetTypes)
    : EventProcessor(OFFSET, RANGE, "GammaScintFragProcessor") {
    Theader = GSArgs;
    associatedTypes.insert("gscint");

    ISOL_ = false;
    FacilType_ = GSArgs.find("FacilityType")->second;
    if (FacilType_ == "ISOL")
        ISOL_ = true;

    typeList_ = set<string>(DetTypes.begin(), DetTypes.end());

    if (ISOL_) {
        BunchingTimestr_ = "Tape_Cycle";
        bunchingTime_ = (double)0.0;
    } else {
        BunchingTimestr_ = GSArgs.find("BunchingTime")->second;
        bunchingTime_ = strtod(BunchingTimestr_.c_str(), nullptr);
    }

    localRoot_ = StringManipulation::StringToBool(GSArgs.find("GSroot")->second);
}

bool GammaScintFragProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);

    /** From the Detector Driver we get the Pixie Event number and the system-wide first event time
     * We get the event list for the "gscint" type.
     */
    firstGSEvent_ = false;
    if (evtNum_ == 0) {
        firstEventTime_ = DetectorDriver::get()->GetFirstEventTimeinNs();
        bunchLast_ = firstEventTime_;
        firstGSEvent_ = true;
    }
    evtNum_ = DetectorDriver::get()->GetEventNumber();
    GSEvents_ = event.GetSummary("gscint")->GetList();

    return (true);
}
bool GammaScintFragProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);

    hasLowResBeta_ = false;

    if (!event.GetSummary("beta")->GetList().empty()) {  //get pixie event level beta for setups with a "beta" type
        hasLowResBeta_ = true;
        //this works for beta:double and beta:single

    } else if (!event.GetSummary("pspmt:dynode_high")->GetList().empty()) {  //get pixie event level beta for setups with a "pspmt:dynode_high" type
        hasLowResBeta_ = true;
    }

    /** Which kind of bunching to use depends on the Facility. ISOL has access to the Tape Cycle and Time;
    * while Fragmentation only has the time bunching (which is why it is the default FacilityType)
    */
    if (ISOL_) {
        if (TreeCorrelator::get()->place("Cycle")->status()) {
            double currentTime_ = TreeCorrelator::get()->place("Cycle")->last().time;
            currentTime_ *= Globals::get()->GetClockInSeconds(GSEvents_.front()->GetChanID().GetModFreq()) * 1.e9;
            if (currentTime_ != bunchLast_) {
                double tdiff = (currentTime_ - bunchLast_) / 1.e6;
                if (bunchNum_ == 0) {
                    cout
                        << " #  There are some events at the beginning of the first segment missing from Histograms that use cycleNum."
                        << endl
                        << " #  This is a product of not starting the cycle After the LDF." << endl
                        << " #  This First TDIFF is most likely nonsense" << endl;
                }
                bunchLast_ = currentTime_;
                bunchNum_++;
                cout << endl
                     << "Cycle Change " << endl
                     << "Tdiff (Cycle start and Now) (ms)= " << tdiff << endl
                     << "Starting on Cycle #" << bunchNum_ << endl;
            }
        }
    } else {
        double currentTime_ = GSEvents_.front()->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(GSEvents_.front()->GetChanID().GetModFreq()) * 1.e9;
        double tdiff = (currentTime_ - bunchLast_);
        if (firstGSEvent_)
            cout << "First Bunch. Current Bunch Size is " << bunchingTime_ << " seconds." << endl;

        if (tdiff > (bunchingTime_ * 1.e9)) {
            bunchLast_ = currentTime_;
            bunchNum_++;
            cout << endl
                 << "Bunch Change" << endl
                 << "Now Starting Bunch # " << bunchNum_ << "." << endl;
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
    }  // end SubtypeList for loop

    //start actual event loop
    for (auto it = GSEvents_.begin(); it != GSEvents_.end(); it++) {
        double currentModFreq = (*it)->GetChanID().GetModFreq();
        string subType = (*it)->GetChanID().GetSubtype();
        unsigned int subTypeOffset = ReturnOffset(subType);
        double Genergy = (*it)->GetCalibratedEnergy();
        //double Gtime = (*it)->GetTimeSansCfd() * Globals::get()->GetClockInSeconds(currentModFreq) * 1.e9;
        //modify by Xu to use onboard cfd timing
        double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
        double Gtime = (*it)->GetTime() * internalTAC_Convert_Tick_adc;

        if (!((*it)->GetChanID().HasTag("dy"))) {
            plot(D_ENERGY + subTypeOffset, Genergy);
            plot(DD_LONGTIME_ENERGY + subTypeOffset, Genergy, bunchNum_);

            if (hasLowResBeta_) {
                plot(D_BG_ENERGY + subTypeOffset, Genergy);
            }
        } else {
            plot(D_DY_ENERGY + subTypeOffset, Genergy);
            if (hasLowResBeta_) {
                plot(D_DY_BG_ENERGY + subTypeOffset, Genergy);
            }
        }

        if (localRoot_) {
            if ((*it)->GetChanID().HasTag("dy")) {
                GSstruct.isDynodeOut = true;
            }
            GSstruct.group = (*it)->GetChanID().GetGroup();
            GSstruct.subtype = (*it)->GetChanID().GetSubtype();
            GSstruct.energy = Genergy;
            GSstruct.rawEnergy = (*it)->GetEnergy();
            GSstruct.time = Gtime;
            GSstruct.detNum = (*it)->GetChanID().GetLocation();

            if (!(*it)->GetTrace().empty() && (*it)->GetTrace().HasValidWaveformAnalysis()) {
                GSstruct.qdc = (*it)->GetTrace().GetQdc();
            }

            pixie_tree_event_->gammascint_vec_.emplace_back(GSstruct);
            GSstruct = processor_struct::GAMMASCINT_DEFAULT_STRUCT;  //reset structure
            //Dont fill because we want 1 pixie event per tree entry, so we add the current structure in the last spot
            //on a vector<> and then reset the structure. and we will at the end or Process()
        }  //end localRoot_
    }
    EndProcess();
    return (true);
}

unsigned int GammaScintFragProcessor::ReturnOffset(const std::string &subtype) {
    if (subtype == "nai")
        return (NaIOFFSET);
    if (subtype == "bighag")
        return (BIGHAG_OFFSET);
    if (subtype == "smallhag")
        return (LITTLEHAG_OFFSET);

    return (numeric_limits<unsigned int>::max());
}
