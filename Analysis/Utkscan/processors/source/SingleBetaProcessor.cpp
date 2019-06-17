/** \file SingleBetaProcessor.cpp
 * \brief New Processor to handle a generic single ended "beta" detector
 * \author T. T. King
 * \date June 17, 2019
 */

#include "SingleBetaProcessor.hpp"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "TreeCorrelator.hpp"

namespace dammIds {
namespace singlebeta {
const int DD_LOWEN = 0;        //!< ID for Low Res detector energy
const int DD_HIGHEN = 1;       //!< ID for Low Res detector energy
const int DD_QDC = 2;          //!< ID for the QDC of the HRT single beta detectors
const int DD_BETAMAXXVAL = 3;  //!< Max Value in Trace for appropriate detectors
}  // namespace singlebeta
}  // namespace dammIds

using namespace std;
using namespace dammIds::singlebeta;

SingleBetaProcessor::SingleBetaProcessor() : EventProcessor(OFFSET, RANGE, "SingleBetaProcessor") {
    associatedTypes.insert("beta");
}

void SingleBetaProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_LOWEN, SD, S3, "Pixie Energy vs. LRT Location");
    DeclareHistogram2D(DD_HIGHEN, SD, S3, "Pixie Energy vs. HRT Locations without Traces");
    DeclareHistogram2D(DD_QDC, SB, S3, "QDC vs. HRT Location");
    DeclareHistogram2D(DD_BETAMAXXVAL, SD, S3, "Max Value in the Trace for HRT Beta");
}

bool SingleBetaProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);

    static const vector<ChanEvent *> &events = event.GetSummary("beta:single")->GetList();

    //! standard timing reslution and offset for damm plotting; commented out to prevent compiler warings, but left for future use.
    // double resolution = 2;
    // double offset = 1500;

    for ( auto itE = events.begin(); itE != events.end(); itE++) {
        if (DetectorDriver::get()->GetSysRootOutput()) {
            SBstruc.detNum = (*itE)->GetChanID().GetLocation();
            SBstruc.energy = (*itE)->GetCalibratedEnergy();
            SBstruc.rawEnergy = (*itE)->GetEnergy();
        }
        double hrtTick2Ns = Globals::get()->GetAdcClockInSeconds((*itE)->GetChanID().GetModFreq()) * 1e9;

        double evtTick2Ns = Globals::get()->GetClockInSeconds((*itE)->GetChanID().GetModFreq()) * 1e9;

        if ((*itE)->GetChanID().HasTag("start") && (*itE)->GetTrace().HasValidFitAnalysis()) {
            plot(DD_QDC, (*itE)->GetTrace().GetQdc(), (*itE)->GetChanID().GetLocation());
            plot(DD_BETAMAXXVAL, (*itE)->GetTrace().GetMaxInfo().second, (*itE)->GetChanID().GetLocation());

            if (DetectorDriver::get()->GetSysRootOutput()) {
                SBstruc.isHighResBeta = true;
                SBstruc.time = (*itE)->GetHighResTimeInNs();
                SBstruc.qdc = (*itE)->GetTrace().GetQdc();
                SBstruc.tMaxVal = (*itE)->GetTrace().GetMaxInfo().second;
                SBstruc.hasTraceFit = true;
            }
        } else if ((*itE)->GetChanID().HasTag("start") && !(*itE)->GetTrace().HasValidFitAnalysis()) {  //! Start Beta, without a valid fit for the trace (I.e no trace, ignored or Onboard CFD)
            plot(DD_HIGHEN, (*itE)->GetCalibratedEnergy(), (*itE)->GetChanID().GetLocation());

            if (DetectorDriver::get()->GetSysRootOutput()) {
                SBstruc.isHighResBeta = true;
                SBstruc.time = (*itE)->GetTime() * hrtTick2Ns;
                if ((*itE)->GetTrace().HasValidWaveformAnalysis()){
                    SBstruc.qdc = (*itE)->GetTrace().GetQdc();
                }
            }
        } else {  //! Low Res Betas. I.E. beta in singles

            plot(DD_LOWEN, (*itE)->GetCalibratedEnergy(), (*itE)->GetChanID().GetLocation());
            if (DetectorDriver::get()->GetSysRootOutput()) {
                SBstruc.isLowResBeta = true;
                SBstruc.time = (*itE)->GetTimeSansCfd() * evtTick2Ns;
            }
        }
        if(DetectorDriver::get()->GetSysRootOutput()){
            pixie_tree_event_->singlebeta_vec_.emplace_back(SBstruc);
            SBstruc = processor_struct::SINGLEBETA_DEFAULT_STRUCT;
        }
    }

    return (true);
}

bool SingleBetaProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    EndProcess();
    return (true);
}
