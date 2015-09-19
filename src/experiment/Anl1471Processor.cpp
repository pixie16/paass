/** \file Anl1471Processor.cpp
 *\brief Processes information for VANDLE
 *
 * Processes VANDLE data for the ANL experiment number 1471
 *
 *\author S. V. Paulauskas
 *\date September 19, 2015
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "Anl1471Processor.hpp"

namespace dammIds {
    namespace vandle {
        const unsigned int DEBUGGING_OFFSET = 70;
        const int DD_DEBUGGING0  = 0+DEBUGGING_OFFSET;
        const int DD_DEBUGGING1  = 1+DEBUGGING_OFFSET;
        const int DD_DEBUGGING2  = 2+DEBUGGING_OFFSET;
        const int DD_DEBUGGING3  = 3+DEBUGGING_OFFSET;
        const int DD_DEBUGGING4  = 4+DEBUGGING_OFFSET;
        const int DD_DEBUGGING5  = 5+DEBUGGING_OFFSET;
        const int DD_DEBUGGING6  = 6+DEBUGGING_OFFSET;
        const int DD_DEBUGGING7  = 7+DEBUGGING_OFFSET;
        const int DD_DEBUGGING8  = 8+DEBUGGING_OFFSET;
        const int DD_DEBUGGING9  = 9+DEBUGGING_OFFSET;
        const int DD_DEBUGGING10  = 10+DEBUGGING_OFFSET;
        const int DD_DEBUGGING11  = 11+DEBUGGING_OFFSET;
        const int DD_DEBUGGING12  = 12+DEBUGGING_OFFSET;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

void Anl1471Processor::DeclarePlots(void) {
    VandleProcessor::DeclarePlots();
    DeclareHistogram2D(DD_DEBUGGING0, SC, S7, "General Purpose");
    DeclareHistogram2D(DD_DEBUGGING1, SA, SD, "TOF - banana gated");
    DeclareHistogram2D(DD_DEBUGGING2, SA, SD, "TOF - Lower Bars Banana Gated");
    DeclareHistogram2D(DD_DEBUGGING3, SA, SD, "TOF - Upper Bars Banana Gated");
    DeclareHistogram2D(DD_DEBUGGING4, SA, SD, "TOF - Gated 600 keV");
    DeclareHistogram2D(DD_DEBUGGING5, SA, SD, "TOF - Gated 700 keV");
    DeclareHistogram2D(DD_DEBUGGING6, SA, SC, "TOF - Bar 12 Banana Gated");
    DeclareHistogram2D(DD_DEBUGGING7, SA, SA, "Currently Unused");
    DeclareHistogram2D(DD_DEBUGGING8, SA, SA, "Currently Unused");
    DeclareHistogram2D(DD_DEBUGGING9, SC, S7, "Bar Loc vs. TOF - gated");
    DeclareHistogram2D(DD_DEBUGGING10, SB, SA, "Decay Time vs. ToF");
    DeclareHistogram2D(DD_DEBUGGING11, SC, SA, "Decay Time vs. GEnergy");
    DeclareHistogram2D(DD_DEBUGGING12, SA, SA, "Decay Time vs. ToF - banana");

    DeclareHistogram2D(DEBUGGING_OFFSET+20, SA, SA, "QDC TOF - Lower 0");
    DeclareHistogram2D(DEBUGGING_OFFSET+21, SA, SA, "QDC TOF - Lower 1");
    DeclareHistogram2D(DEBUGGING_OFFSET+22, SA, SA, "QDC TOF - Upper 0");
    DeclareHistogram2D(DEBUGGING_OFFSET+23, SA, SA, "QDC TOF - Upper 1");
    DeclareHistogram2D(DEBUGGING_OFFSET+24, SA, SD, "QDC TOF - Upper ");
    DeclareHistogram2D(DEBUGGING_OFFSET+25, SA, SD, "QDC TOF - Lower");
}

Anl1471Processor::Anl1471Processor(const std::vector<std::string> &typeList,
    const double &res, const double &offset, const double &numStarts) :
    VandleProcessor(typeList,res,offset,numStarts) {
    associatedTypes.insert("vandle");

    char hisFileName[32];
    GetArgument(1, hisFileName, 32);
    string temp = hisFileName;
    temp = temp.substr(0, temp.find_first_of(" "));
    stringstream name;
    name << temp;
    fileName_ = name.str();
    fileNames_.push_back(fileName_ + "-tof-sm.dat");
    fileNames_.push_back(fileName_ + "-tof-md.dat");
    fileNames_.push_back(fileName_ + "-tof-04Plus.dat");
}

bool Anl1471Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    if(!VandleProcessor::Process(event))
        return(false);

    for (BarMap::iterator it = bars_.begin(); it !=  bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;
        if(!bar.GetHasEvent())
            continue;

        unsigned int barLoc = barId.first;
        TimingCalibration cal = bar.GetCalibration();

        int bananaNum;
	if(bar.GetType() == "small")
	    bananaNum = 2;
	if(bar.GetType() == "medium")
	    bananaNum = 1;

        for(BarMap::iterator itStart = barStarts_.begin();
        itStart != barStarts_.end(); itStart++) {
            BarDetector start = (*itStart).second;
            
            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = numStarts_*barLoc+startLoc;
	    
            double tofOffset = cal.GetTofOffset(startLoc);
            double tof = bar.GetWalkCorTimeAve() -
                start.GetTimeAverage() + tofOffset;
            double corTof =
                VandleProcessor::CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

            bool inPeel = histo.BananaTest(bananaNum,
                                           corTof*plotMult_+plotOffset_,
                                           bar.GetQdc());
	    //All of them are gated using a banana gate
            if(inPeel) {
		ofstream data;
		if(bar.GetType() == "small") {
                    data.open(fileNames_[0].c_str(), ios::app);
                    data << corTof << " " << bar.GetQdc() << endl;
                } else {
		    data.open(fileNames_[1].c_str(), ios::app);
                    data << corTof << " " << bar.GetQdc() << endl;
		}
		data.close();
            }

            double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
            cycleTime *= (Globals::get()->clockInSeconds()*1.e9);

            double decayTime = (bar.GetTimeAverage() - cycleTime)/0.01;

            plot(DD_DEBUGGING10, corTof*plotMult_+plotOffset_, decayTime*1.e-9);
            if(inPeel)
                plot(DD_DEBUGGING12, corTof*plotMult_+plotOffset_, decayTime*1.e-9);

            //ofstream gammaGated;
            if(geSummary_->GetMult() > 0 && inPeel) {
                const vector<ChanEvent *> &geList = geSummary_->GetList();
                for(vector<ChanEvent *>::const_iterator itGe = geList.begin();
                itGe != geList.end(); itGe++) {
                    double calEnergy = (*itGe)->GetCalEnergy();
                    plot(DD_DEBUGGING11, calEnergy, decayTime * 1e-9);
                }// for(vector<ChanEvent *>::const_iterator
            }//geSummary_->GetMult() > 0
        }//loop over starts
    }//loop over bars
    EndProcess();
    return(true);
}
