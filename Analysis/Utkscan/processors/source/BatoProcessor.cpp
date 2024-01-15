/** @file BatoProcessor.cpp
 *  @brief Processor for Batos (Clover beta vetos)
 *  @authors T.T. King
 *  @date Oct 2020
*/

#include "BatoProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "Display.h"
#include "Exceptions.hpp"
#include "Messenger.hpp"
#include "RawEvent.hpp"
#include "pugixml.hpp"

using namespace std;
using namespace dammIds::bato;

namespace dammIds {
//! Namespace containing histogram definitions for the Bato.
namespace bato {
const unsigned int MAX_BATOS = 3;  //!< for *_DETX spectra

const int D_BATO_MULTI = 0;          //!<Multiplicity
const int DD_BATO_ENERGY = 1;  //!< Energy per board Bato
const int DD_BATO_QDC = 2;     //!< QDC per board Bato

}  // namespace bato
}  // namespace dammIds

BatoProcessor::BatoProcessor() : EventProcessor(OFFSET, RANGE, "BatoProcessor") {
    associatedTypes.insert("bato");
    batoevtcounter=0;
}

/** Declare plots including many for decay/implant/neutron gated analysis  */
void BatoProcessor::DeclarePlots(void) {
    //build the clovers from the channel list
    auto batoLocs_ = DetectorLibrary::get()->GetLocations("bato", "bato");
    BatoBuilder(batoLocs_);

    DeclareHistogram1D(D_BATO_MULTI, S5, "BATO Multiplicity");
    DeclareHistogram2D(DD_BATO_ENERGY, SE, S4, "BATO Energy vs DetNum");
    DeclareHistogram2D(DD_BATO_QDC, SE, S4, "BATO QDC vs DetNum");
}

bool BatoProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    using namespace dammIds::bato;
    //vector <ChanEvent*>
    static const auto &batoEvents = event.GetSummary("bato:bato", true)->GetList();


    plot(D_BATO_MULTI, batoEvents.size());

    for (auto it = batoEvents.begin(); it != batoEvents.end(); ++it) {
        int batoNum = (*it)->GetChanID().GetLocation();

      
        double pQDC = 0;
        vector<unsigned> onBoardQdc = {} ;
        if (!(*it)->GetQdc().empty()) {
            onBoardQdc = (*it)->GetQdc();
            pQDC= OnBoardQDC_to_WaveFormQDC(onBoardQdc);
        } 
        double Tick_ns = Globals::get()->GetClockInSeconds((*it)->GetChanID().GetModFreq()) * 1e9;
       
        plot(DD_BATO_ENERGY, (*it)->GetCalibratedEnergy(), batoNum);
        plot(DD_BATO_QDC, pQDC, batoNum);
// cout<<"bat0"<<endl;
        if (DetectorDriver::get()->GetSysRootOutput()) {
            //Fill root struct and push back on to vector
            BatoStruct.detNum = batoNum;
            BatoStruct.energy = (*it)->GetCalibratedEnergy();
            BatoStruct.time = (*it)->GetTimeSansCfd() * Tick_ns;
            BatoStruct.qdc = pQDC;
            BatoStruct.pQDCsums = vector<double>(onBoardQdc.begin(), onBoardQdc.end());
            pixie_tree_event_->bato_vec_.emplace_back(BatoStruct);
            BatoStruct = processor_struct::BATO_DEFAULT_STRUCT;  //reset to initalized values (see PaassRootStruct.hpp
        }
    }
    EndProcess();
    return true;
}

bool BatoProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    EndProcess();
    return true;
}

void BatoProcessor::BatoBuilder(const std::set<int> &batoLocs) {
    /*  BATO specific routine, determine the number of BATO detector
       channels and divide by num of chans per BATO to find the total number of BATOs
       Right now the BATOs are 2 chan per pad with 2 sipm per channel
    */
    auto BatoLocations = &batoLocs;
    // could set it now but we'll iterate through the locations to set this
    unsigned int BatoChans = 0;

    for (set<int>::const_iterator it = BatoLocations->begin(); it != BatoLocations->end(); it++) {
        leafToBato[*it] = int(BatoChans / chansPerBato);
        BatoChans++;
    }

    if (BatoChans % chansPerBato != 0) {
        stringstream ss;
        ss << " There does not appear to be the proper number of"
           << " channels per Bato.";
        throw GeneralException(ss.str());
    }
    if (BatoChans != 0) {
        numBatos = BatoChans / chansPerBato;
        Messenger m;
        m.start("Building Batos");

        stringstream ss;
        ss << "A total of " << BatoChans
           << " Bato channels were detected: ";
        int lastBato = numeric_limits<int>::min();
        for (map<int, int>::const_iterator it = leafToBato.begin();
             it != leafToBato.end(); it++) {
            if (it->second != lastBato) {
                m.detail(ss.str());
                ss.str("");
                lastBato = it->second;
                ss << "Bato " << lastBato << " : ";
            } else {
                ss << ", ";
            }
            ss << setw(2) << it->first;
        }
        m.detail(ss.str());

        if (numBatos > dammIds::bato::MAX_BATOS) {
            m.fail();
            stringstream ss;
            ss << "Number of detected Batos is greater than defined"
               << " MAX_BATOS = " << dammIds::bato::MAX_BATOS << "."
               << " See BatoProcessor.hpp for details.";
            throw GeneralException(ss.str());
        }
        m.done();
    }
}

double BatoProcessor::OnBoardQDC_to_WaveFormQDC(vector<unsigned> onBoardQdc) {
    double pQDC = 0, pQDC_baseline = 0, pQDC_waveform = 0;  //, pQDC_tail = 0;
    pQDC_baseline = onBoardQdc.at(0) + onBoardQdc.at(1);
    pQDC_waveform = onBoardQdc.at(3) + onBoardQdc.at(4) + onBoardQdc.at(5);
    //pQDC_tail = onBoardQdc.at(6) + onBoardQdc.at(7);
    pQDC = pQDC_waveform - pQDC_baseline;

    return pQDC;
}
